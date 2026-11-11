#include "Viewport/RobotEditorViewportClient.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/Material.h"
#include "MaterialDomain.h"
#include "Components/StaticMeshComponent.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "Components/DirectionalLightComponent.h"
#include "Components/SkyLightComponent.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "Structs/RobotData.h"
#include "Data/PartDat.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {
        SetViewMode(VMI_Lit);
        UDirectionalLightComponent* KeyLight = NewObject<UDirectionalLightComponent>();
        KeyLight->Intensity = 3.0f;
        KeyLight->LightColor = FColor::White;
        KeyLight->SetForwardShadingPriority(1);

        FRotator KeyRotation = FRotator(-45.0f, 45.0f, 0.0f);


        UDirectionalLightComponent* FillLight = NewObject<UDirectionalLightComponent>();
        FillLight->Intensity = 1.0f;
        FillLight->LightColor = FColor(200, 220, 255);
        FillLight->SetForwardShadingPriority(0);

        FRotator FillRotation = FRotator(45.0f, -135.0f, 0.0f);


        USkyLightComponent* SkyLight = NewObject<USkyLightComponent>();
        SkyLight->Intensity = 0.5f;

        PreviewScene->AddComponent(KeyLight, FTransform(KeyRotation));
        PreviewScene->AddComponent(FillLight, FTransform(FillRotation));
        PreviewScene->AddComponent(SkyLight, FTransform::Identity);

        m_TargetOrbit = m_OrbitTargetPoint;

        if (!m_CurrentRobotData){
            CreateNewRobot();
        }
        return;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorUtilsViewportClient::Tick(DeltaSeconds);

    FHitResult bHit = GetCastHitResult();
    if (bHit.bBlockingHit) {
        if (bHit.GetActor()) {
            UpdatePreviewActor(true);

            if (m_PreviewActor) {
                m_PreviewActor->SetActorLocationAndRotation(bHit.ImpactPoint, FRotator::ZeroRotator);
            }

            SnapToNearestAnchor(bHit, m_PreviewActor);
        }
    } else {
        UpdatePreviewActor(false);
        MovePreviewActorToLocation(bHit.TraceEnd);
    }

    if (!FVector::PointsAreNear(m_TargetOrbit, m_OrbitTargetPoint, 0.01f)) {
        float Alpha = 5.0f * DeltaSeconds;
        m_OrbitTargetPoint = FMath::Lerp(m_OrbitTargetPoint, m_TargetOrbit, FMath::Clamp(Alpha, 0.0f, 1.0f));
        FEditorUtilsViewportClient::UpdateCameraPositionFromOrbit();
    }
    FEditorUtilsViewportClient::Invalidate();
}

TTuple<const FPartData& /*HitData*/, int /*HitID*/, int /*PreviewID*/> FRobotEditorViewportClient::SnapToNearestAnchor(const FHitResult& HitResult, AActor* SnapActor) {
    if (!HitResult.bBlockingHit || !SnapActor) {
        return MakeTuple(FPartData(), INDEX_NONE, INDEX_NONE);
    }

    AActor* HitActor = HitResult.GetActor();
    const FPartData& HitData = GetPartDataFromActor(HitActor);

    if (!HitData.Part || !m_SelectedPart || HitData.Part->Anchors.Num() == 0 || m_SelectedPart->Anchors.Num() == 0) {
        return MakeTuple(FPartData(), INDEX_NONE, INDEX_NONE);
    }

    /*
     * Our first iteration. In this loop, we find the closest anchor to the impact position of the ray.
     * We then store a copy of the anchor to perform some transforms on.
     */

    FTransform HitTransform = HitActor->GetActorTransform();

    FAnchor TargetHitAnchor;
    FVector ClosestAnchorWorldPos = FVector::ZeroVector;
    float ClosestDistanceSq = MAX_FLT;
    int TargetAnchorIndex = INDEX_NONE;
    int CurrentIndex = 0;

    for (const FAnchor& Anchor : HitData.Part->Anchors) {
        FVector AnchorWorldPos = HitTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, HitResult.ImpactPoint);

        if (DistanceSq < ClosestDistanceSq) {
            ClosestAnchorWorldPos = AnchorWorldPos;
            TargetAnchorIndex = CurrentIndex;
            ClosestDistanceSq = DistanceSq;
            TargetHitAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /*
     * If there isn't an anchor within a relatively normal set distance, (in this case we'll use the imaginary numer Infinity as our test point), then there's nothing to snap to.
     */

    if (ClosestDistanceSq > MAX_FLT) {
        return MakeTuple(FPartData(), INDEX_NONE, INDEX_NONE);
    }

    /*
     * We need to get the normal of the anchor based on the transform of the actor that we're reading from. We just simply call GetSafeNormal(); A fun little function that already exists inside Unreal.
     * But that's not all, we also need to find the target snap. This acts as the "opposite" of the hit anchor normal, so it ensures that the anchor that you get always has a normal facing INTO it,
     * rather than away from.
     */

    FVector HitAnchorNormalWorld = HitTransform.TransformVector(TargetHitAnchor.Position.GetSafeNormal()).GetSafeNormal();
    FVector TargetSnapNormalWorld = -HitAnchorNormalWorld;

    /*
     * Now we score differently here, because the object can be drastically different depending on where you're colliding. We need to factor in a few things here.
     * 1) Orientation of the object:
     *
     *  An object might be a cylinder, as opposed to a cube. This cylinder may be harder to calculate distances from if they're equidistant from the center.
     *
     * 2) The dot product of the normals
     *
     *  A normal might have a greater or lesser dot-product depending on where it is on the object.
     *
     * 3) The position of the anchors relative to the object.
     *
     * Combining these three rules gets us a total score that allows us to closer choose an anchor.
     */

    FTransform SnapTransform = SnapActor->GetActorTransform();
    FAnchor SnapAnchor;
    int TargetSnapAnchorIndex = INDEX_NONE;
    CurrentIndex = 0;

    float BestAnchorScore = -MAX_FLT;

    for (const FAnchor& Anchor : m_SelectedPart->Anchors) {
        FVector LocalNormal = Anchor.Position.IsNearlyZero() ? -FVector::UpVector : Anchor.Position.GetSafeNormal();

        FVector AnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();
        float DirectionScore = FVector::DotProduct(AnchorNormalWorld, TargetSnapNormalWorld);

        FVector AnchorWorldPos = SnapTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, ClosestAnchorWorldPos);
        float ProximityScore = -(DistanceSq / 10000.0f);

        float TotalScore = DirectionScore + ProximityScore;

        if (TotalScore > BestAnchorScore) {
            TargetSnapAnchorIndex = CurrentIndex;
            BestAnchorScore = TotalScore;
            SnapAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /*
     * Long, Boring and indescribable maths up ahead. This required Quarternions, Dot Products and Vector Transforms. I never want to touch maths ever again.
     * Nevertheless, time to explain how this works.
     *
     * Below, we declare LocalNormal and WorldNormals. LocalNormal determines that if an Anchor's position is nearly 0, 0, 0, we prevent a divide by 0 issue and declare that the anchor is pointing
     * upwards through an arbitrary rule. This should only affect anchors that are not correctly put in place.
     *
     * We then calculate the actual normal based on the transform in the world, and turn it into a value between -1.0 and 1.0 for our necessary transformations.
     */
    FVector LocalNormal = SnapAnchor.Position.IsNearlyZero() ? -FVector::UpVector : SnapAnchor.Position.GetSafeNormal();
    FVector SnapAnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();

    /*
     * We want our objects to handle rotations. For example, should you be hovering over the side of a six-sided cube and have an object with a single anchor point, this will align the points together.
     * Thankfully, Unreal has a bunch of useful Quarternion functions inside their actual Quarternion data, so we just need to find the shortest safe angular rotation to get us to our desired point.
     * That's what this lovely FindBetweenNormals function does, it finds the shortest angular path to a point.
     */

    FQuat AlignmentRotation = FQuat::FindBetweenNormals(SnapAnchorNormalWorld, TargetSnapNormalWorld);
    FQuat FinalRotation = AlignmentRotation * SnapTransform.GetRotation();

    // /*
    //  * Here we handle user-input-based rotation. When the user presses either Q or E, the m_CurrentRotationInputSteps will increment or decrement with a % of 4, ensuring that it will always wrap around
    //  * between 0 -> 3.
    //  *
    //  * We then multiply our current rotation input steps by the amount we actually want an object to rotate, in this case we just want snapping every 90 degrees or so, so we can do so here.
    //  * (I also did this during Advanced Development Techniques, through `(TAU * 4)) * rotation_num`. So this is a fun useful little tidbit for rotation snapping!)
    //  *
    //  * We then apply this rotation after the previous one. (Quarternion multiplication is different to actual multiplication. Consider it adding angles sequentially rather than multiplying the
    //  * values directly.)
    //  */

    float AngleOffset = m_CurrentRotationNum * 90.0f;
    FQuat AxisStepRotation = FQuat(TargetSnapNormalWorld, FMath::DegreesToRadians(AngleOffset));
    FinalRotation = AxisStepRotation * FinalRotation;

    /*
     * We then rotate our anchor vector by this final quarternion rotation, and can set our location relative to all of the maths that we just performed to find the optimal place to put the damn thing.
     */
    FVector RotatedSnapLocalAnchor = FinalRotation.RotateVector(SnapAnchor.Position);
    FVector FinalWorldLocation = ClosestAnchorWorldPos - RotatedSnapLocalAnchor;

    /*
     * Then all we need to do is set the location and the rotation, and the snapping is complete.
     */

    SnapActor->SetActorLocationAndRotation(FinalWorldLocation, FinalRotation);

    FEditorUtilsViewportClient::Invalidate();
    return MakeTuple(HitData, TargetAnchorIndex, TargetSnapAnchorIndex);
}

void FRobotEditorViewportClient::CreatePartFromPreview() {
    if (!m_PreviewActor || !m_SelectedPart) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    FTransform SpawnTransform = m_PreviewActor->GetActorTransform();
    AActor* NewActor = FEditorUtilsViewportClient::CreateActorComponent(m_SelectedPart->Object, SpawnParams);

    if (NewActor) {
        NewActor->SetActorTransform(SpawnTransform);
        m_Actors.Emplace(NewActor, FPartData(m_SelectedPart, SpawnTransform, {}));
    }

    UpdateOrbitTargetPoint();
    FEditorUtilsViewportClient::Invalidate();
}

FHitResult FRobotEditorViewportClient::GetCastHitResult() {
    UWorld* World = PreviewScene->GetWorld();

    TTuple<FVector, FVector> RayData = FEditorUtilsViewportClient::CastRayFromCameraToMouse();
    const auto& [RayStart, RayEnd] = RayData;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(m_PreviewActor);

    bool bHit = World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, Params);
    return HitResult;
}

bool FRobotEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {

    if (EventArgs.Key == EKeys::E && EventArgs.Event == IE_Pressed) {
        m_CurrentRotationNum = ((m_CurrentRotationNum + 1) % 4);
    }
    if (EventArgs.Key == EKeys::Q && EventArgs.Event == IE_Pressed) {
        m_CurrentRotationNum = ((m_CurrentRotationNum - 1) % 4);
    }

    if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed) {
        /*
         * Right click orbits the view, but we need a button to remove objects that have been placed. So, we'll just override left control here and handle removing the hit actor.
         * We don't have to do much logic here, we're not checking for connected anchors so we can just remove it from the map and destroy the object.
         */
        if (EventArgs.Viewport->KeyState(EKeys::LeftControl)) {
            FHitResult HitResult = GetCastHitResult();

            if (HitResult.bBlockingHit) {
                AActor* HitActor = HitResult.GetActor();
                if (HitActor && m_Actors.Find(HitActor)) {
                    m_Actors.Remove(HitActor);
                    HitActor->Destroy();
                }
            }
            UpdateOrbitTargetPoint();
            return true;
        }

        if (!m_SelectedPart) { return true; }
        /*
         *   If there isn't a part displayed on the screen, place immediately at 0, 0, 0. If there is a part on the screen, perform a cast and ensure that the cast is hitting something.
         *   If it isn't, pass this down thorugh FEditorUtilsViewportClient::InputKey(EventArgs);
         */
        if (m_Actors.Num() == 0) {
            CreatePartAtTransform(m_SelectedPart, FTransform::Identity);
        }
        else {
            FHitResult HitResult = GetCastHitResult();
            if (HitResult.bBlockingHit) {
                if (m_PreviewActor) {
                    CreatePartFromPreview();
                } else {
                    CreatePartAtTransform(m_SelectedPart, FTransform(HitResult.ImpactPoint));
                }
            }
        }

         return true;
    }



    return FEditorUtilsViewportClient::InputKey(EventArgs);
}

void FRobotEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorUtilsViewportClient::Draw(View, PDI);
}

void FRobotEditorViewportClient::SetSelectedPart(UPDAPart* PartData) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    m_SelectedPart = PartData;

    if (m_PreviewActor) {
        m_PreviewActor->Destroy();
        m_PreviewActor->Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (PartData) {
        if (PartData->Object) {
            NewActor = FEditorUtilsViewportClient::CreateActorComponent(PartData->Object, SpawnParams);
            if (NewActor) {
                m_PreviewActor = NewActor;
                ApplyPreviewMaterial();
            }
        }
    }
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::UpdatePreviewActor(bool ShouldShow) {
    const bool bPropagate = true;
    if (m_PreviewActor) {
        if (USceneComponent* Root = m_PreviewActor->GetRootComponent()) {
            Root->SetVisibility(ShouldShow, bPropagate);
        }
    }
    return;
}

void FRobotEditorViewportClient::MovePreviewActorToLocation(FVector Location) {
    if (m_PreviewActor) {
        m_PreviewActor->SetActorLocation(Location);
    }
    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::CreatePartAtTransform(UPDAPart* Part, FTransform Transform) {
    if (!Part) { return; }

    if (Part->Object) {
        FActorSpawnParameters SpawnParams;
        SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        AActor* NewActor = FEditorUtilsViewportClient::CreateActorComponent(Part->Object, SpawnParams);
        NewActor->SetActorTransform(Transform);

        if (NewActor) {
            m_Actors.Emplace(NewActor, FPartData(Part, Transform, {}));
        }
    }

    UpdateOrbitTargetPoint();
    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::SetSelectedActorsData(const FPartData& PartData) {
    return;
}

TArray<FTransform> FRobotEditorViewportClient::GetAnchorsRelativeToPart(UPDAPart* Part) {
    return {};
}

FPartData FRobotEditorViewportClient::GetPartDataFromActor(AActor* Actor) {
    if (m_Actors.Find(Actor)) {
        FPartData& FindData = *m_Actors.Find(Actor);
        return FindData;
    }

    return FPartData();
}

AActor* FRobotEditorViewportClient::GetActor() {
    return m_SelectedActor;
}

FVector FRobotEditorViewportClient::GetActorLocation(AActor* Actor) {
    return Actor->GetActorLocation();
}

void FRobotEditorViewportClient::OnThumbnailTriggered(UPDAPart* Part) {
    SetSelectedPart(Part);
    return;
}

void FRobotEditorViewportClient::ConnectTileSingleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailClicked.AddSP(this, &FRobotEditorViewportClient::OnThumbnailTriggered);
}

void FRobotEditorViewportClient::ConnectTileDoubleClick(SThumbnailTile* Tile) {
    return;
}

void FRobotEditorViewportClient::OnAddButtonPressed() {
    return;
}

void FRobotEditorViewportClient::SaveActiveAsset() {
    if (!m_CurrentRobotData || m_Actors.IsEmpty()) {
        return;
    }

    URobotDataContainer* Data = m_CurrentRobotData;
    TArray<FPartData> PartData;

    FTransform CenterOfMass;
    CenterOfMass.SetTranslation(GetCenterOfMass());

    for (auto& KVPair : m_Actors) {
        FPartData& InnerData = KVPair.Value;
        if (AActor* Actor = KVPair.Key) {
            InnerData.RelativeTransform.SetLocation(CenterOfMass.InverseTransformPosition(Actor->GetActorLocation()));
        }
    }

    m_Actors.GenerateValueArray(PartData);

    Data->RobotParts.Parts = PartData;

    UPackage* CurrentPackage = Data->GetPackage();
    FString CurrentName = Data->RobotName.ToString();
    CurrentName = CurrentName.Replace(TEXT(" "), TEXT("_"));

    if (CurrentName.IsEmpty() || CurrentName.Len() == 0) {
        CurrentName = TEXT("UNNAMED");
    }

    Data->RobotName = FName(*CurrentName);

    FString AssetGamePath = TEXT("/RobotEditor/Data/Robots");
    FEditorUtilsViewportClient::ReloadAssetRegistryModule(AssetGamePath);

    if (CurrentPackage != GetTransientPackage()) {
        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(CurrentPackage);

        CurrentPackage->MarkPackageDirty();
        Data->MarkPackageDirty();

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
        LoadRobot(Data);
    }
    else if (CurrentName.Equals(TEXT("UNNAMED"), ESearchCase::IgnoreCase)) {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

        FSaveAssetDialogConfig SaveDialogConfig;
        SaveDialogConfig.DialogTitleOverride = INVTEXT("Save new Robot");
        SaveDialogConfig.DefaultPath = AssetGamePath;
        SaveDialogConfig.DefaultAssetName = TEXT("Unnamed");
        SaveDialogConfig.AssetClassNames.Add(URobotDataContainer::StaticClass()->GetClassPathName());

        FOnObjectPathChosenForSave OnSavePressed = FOnObjectPathChosenForSave::CreateLambda([this, Data](const FString& ChosenPackagePath) {
            FString CleanAssetName = FPaths::GetBaseFilename(ChosenPackagePath);
            FString TargetFolderPath = FPaths::GetPath(ChosenPackagePath);

            Data->Rename(*CleanAssetName, Data->GetOuter());

            SaveNewAsset(Data, TargetFolderPath, *CleanAssetName);
        });

        FOnAssetDialogCancelled OnCancelPressed = FOnAssetDialogCancelled::CreateLambda([]() {
        });

        ContentBrowserModule.Get().CreateSaveAssetDialog(SaveDialogConfig, OnSavePressed, OnCancelPressed);
    }

    else if (CurrentPackage == GetTransientPackage()) {
        FString TargetFolderPackagePath = AssetGamePath;
        SaveNewAsset(Data, TargetFolderPackagePath, CurrentName);
    }
    return;
}

void FRobotEditorViewportClient::SaveNewAsset(URobotDataContainer* RobotData, const FString& ChosenPackagePath, const FString& CurrentName) {
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    FString UniquePackagePath;
    FString UniqueAssetName;

    AssetToolsModule.Get().CreateUniqueAssetName(ChosenPackagePath + TEXT("/") + CurrentName, TEXT(""), UniquePackagePath, UniqueAssetName);
    URobotDataContainer* NewSavedAsset = Cast<URobotDataContainer>(AssetToolsModule.Get().CreateAsset(
        UniqueAssetName,
        ChosenPackagePath,
        URobotDataContainer::StaticClass(),
        nullptr
    ));

    if (NewSavedAsset) {
        NewSavedAsset->RobotParts = RobotData->RobotParts;
        NewSavedAsset->RobotName = FName(*UniqueAssetName);

        NewSavedAsset->MarkPackageDirty();
        NewSavedAsset->GetPackage()->MarkPackageDirty();

        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(NewSavedAsset->GetPackage());

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
    }
    LoadRobot(NewSavedAsset);
}

void FRobotEditorViewportClient::CreateNewRobot() {
    if (m_CurrentRobotData) {

        /*
         * Clear the viewport so that any constructed actors no longer exist and will not be a part of the next robot data.
         */

        ClearScene();
    }

    m_CurrentRobotData = NewObject<URobotDataContainer>(
        GetTransientPackage(),
        URobotDataContainer::StaticClass()
    );

    m_CurrentRobotData->RobotParts = FRobotData();
}

void FRobotEditorViewportClient::LoadRobot(URobotDataContainer* RobotData) {
    if (m_CurrentRobotData) {
        ClearScene();
    }

    m_CurrentRobotData = RobotData;

    /*
     * Now we need to populate our robot with existing data. It should be easy, since the data we're saving is just an array of structs that contains everything we need already.
     */

    for (const FPartData& Part : RobotData->RobotParts.Parts) {
        UPDAPart* VisualPart = Part.Part.Get();
        FTransform Transform = Part.RelativeTransform;

        CreatePartAtTransform(VisualPart, Transform);
    }
}

void FRobotEditorViewportClient::ClearScene() {
    for (const TPair<AActor*, FPartData>& KVPair : m_Actors) {
        KVPair.Key->Destroy();
    }
    m_Actors.Empty();
}

void FRobotEditorViewportClient::UpdateOrbitTargetPoint() {
    m_TargetOrbit = GetCenterOfMass();
    return;
}

FVector FRobotEditorViewportClient::GetCenterOfMass() {
    if (m_Actors.IsEmpty()) {
        return FVector::ZeroVector;
    }

    FVector TotalPositions;
    int TotalParts = m_Actors.Num();
    for (const TPair<AActor*, FPartData>& KVpair : m_Actors) {
        TotalPositions += KVpair.Key->GetActorLocation();
    }
    return TotalPositions / TotalParts;
}

void FRobotEditorViewportClient::ApplyPreviewMaterial() {
    if (!m_PreviewActor) {
        return;
    }

    UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(StaticLoadObject(
        UMaterialInterface::StaticClass(),
        nullptr,
        TEXT("/RobotEditor/Materials/M_RobotPreview.M_RobotPreview")
    ));

    /*
     * Double validation, try to fall-back to a different type of material before completely failing altogether.
     */

    if (!BaseMaterial) {
        BaseMaterial = GEngine->WireframeMaterial;
    } if (!BaseMaterial) {
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, m_PreviewActor);
    if (!DynamicMaterial) {
        return;
    }

    DynamicMaterial->SetVectorParameterValue(FName("Color"), FLinearColor::Green);
    DynamicMaterial->SetScalarParameterValue(FName("Opacity"), 0.3f);

    TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
    m_PreviewActor->GetComponents(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents) {
        if (!MeshComp) {
            continue;
        }

        MeshComp->SetCastShadow(false);

        int32 MaterialCount = MeshComp->GetNumMaterials();
        if (MaterialCount == 0) {
            MeshComp->SetMaterial(0, DynamicMaterial);
        }

        for (int32 i = 0; i < MaterialCount; i++) {
            MeshComp->SetMaterial(i, DynamicMaterial);
        }
    }
}
