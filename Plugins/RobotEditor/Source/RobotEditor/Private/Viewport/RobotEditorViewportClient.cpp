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
#include "Structs/PartStat.h"
#include "Structs/RobotData.h"
#include "Data/PartDat.h"
#include "Components/PrimitiveComponent.h"
#include "Components/BoxComponent.h"
#include "BPFL/RobotEditorBPFL.h"
#include "Editor.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {

        if (GEditor) {
            GEditor->RegisterForUndo(this);
        }

        m_UndoState = TStrongObjectPtr<URobotEditorUndoState>(
            NewObject<URobotEditorUndoState>(GetTransientPackage(), NAME_None, RF_Transactional)
        );

        EngineShowFlags.SetSelection(true);
        EngineShowFlags.SetSelectionOutline(true);

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

        m_CameraOrbitRadius = 200.0f;
        m_TargetOrbit = m_OrbitTargetPoint;

        FEditorUtilsViewportClient::UpdateCameraPositionFromOrbit();

        if (!m_CurrentRobotData){
            CreateNewRobot();
        }
        return;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorUtilsViewportClient::Tick(DeltaSeconds);

    FHitResult bHit = GetCastHitResult();
    bool bSnapped = false;
    if (bHit.bBlockingHit) {
        if (bHit.GetActor()) {
            if (HandleObjectHighlight(bHit.GetActor())) {
                bSnapped = false;
            }
            else if (HandleObjectSnapping(bHit)) {
                bSnapped = true;
            }
        }
    }

    const FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();
    if (!ModifierKeys.IsLeftControlDown() && m_HoveredActor) {
        HandleObjectHighlight(nullptr);
    }

    if (!bHit.bBlockingHit || !bSnapped) {
        m_bValidPlacement = false;
        UpdatePreviewActor(false);
        MovePreviewActorToLocation(bHit.TraceEnd);
    }

    /* Update the camera's position until it is close enough to the target orbit point with a total tolerance of 0.01f
     * Currently a magic number.
     *
     * TODO : Remove magic numbers for constants.
     */

    if (!FVector::PointsAreNear(m_TargetOrbit, m_OrbitTargetPoint, 0.01f)) {
        float Alpha = 5.0f * DeltaSeconds;
        m_OrbitTargetPoint = FMath::Lerp(m_OrbitTargetPoint, m_TargetOrbit, FMath::Clamp(Alpha, 0.0f, 1.0f));
        FEditorUtilsViewportClient::UpdateCameraPositionFromOrbit();
    }

    FEditorUtilsViewportClient::Invalidate();
}

bool FRobotEditorViewportClient::HandleObjectHighlight(AActor* Actor) {
    bool bAffectedObject = false;
    const FModifierKeysState ModifierKeys = FSlateApplication::Get().GetModifierKeys();

    if (ModifierKeys.IsLeftControlDown()) {
        if (m_HoveredActor) {
            ClearOverlayMaterial(m_HoveredActor);
        }

        m_HoveredActor = Actor;

        if (Actor) {
            UMaterialInstanceDynamic* RedOverlay = CreateTransparentMaterialInstance(FLinearColor::Red, 0.8f);
            ApplyOverlayMaterial(Actor, RedOverlay);
        }
        bAffectedObject = true;
    } else {
        if (m_HoveredActor) {
            ClearOverlayMaterial(m_HoveredActor);
            m_HoveredActor = nullptr;
            bAffectedObject = true;
        }
    }
    FEditorUtilsViewportClient::Invalidate();
    return bAffectedObject;
}

bool FRobotEditorViewportClient::HandleObjectSnapping(const FHitResult& bHit) {
    UpdatePreviewActor(true);
    bool bSnapped = false;

    /*
     * Try to snap the object to the nearest other object. If you cannot, simply deal with UpdatePreviewActor()
     */

    UPDAPart* HitPart = GetPartDataFromActor(bHit.GetActor()).Part.Get();
    FSnapResult SnapResult = URobotEditorBPFL::GetSnapResult(bHit.ImpactPoint, bHit.GetActor(), HitPart, m_SelectedPart, m_CurrentRotationNum);
    if (SnapResult.bCanSnap) {
        FTransform SnapTransform = SnapResult.SnapTransform;
        if (URobotEditorBPFL::IsPartColliding(PreviewScene->GetWorld(), SnapTransform, m_SelectedPart)) {
            m_PreviewActor->SetActorTransform(SnapTransform);
            m_bValidPlacement = true;
            bSnapped = true;
        }
    }
    FEditorUtilsViewportClient::Invalidate();
    return bSnapped;
}

FHitResult FRobotEditorViewportClient::GetCastHitResult() {
    UWorld* World = PreviewScene->GetWorld();

    TTuple<FVector, FVector> RayData = FEditorUtilsViewportClient::CastRayFromCameraToMouse();
    const auto& [RayStart, RayEnd] = RayData;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(m_PreviewActor);

    bool bHit = World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GameTraceChannel1, Params);
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
                    DestroyActor(HitActor);
                }
            }
            return true;
        }

        if (!m_SelectedPart) {
            /*
             * If we have no part selected, we want to just select any parts that exist in the world. This includes selecting no part so that we can properly deselect.
             * I'll include another hotkey for deselection
             *
             * TODO : Include that hotkey.
             */
            FHitResult HitResult = GetCastHitResult();
            AActor* HitActor = HitResult.GetActor();
            SetSelectedActor(HitActor);
            return true;
        }
        /*
         *   If there isn't a part displayed on the screen, place immediately at 0, 0, 0. If there is a part on the screen, perform a cast and ensure that the cast is hitting something.
         *   If it isn't, pass this down thorugh FEditorUtilsViewportClient::InputKey(EventArgs);
         */
        if (m_Actors.Num() == 0) {
            SetSelectedActor(nullptr);
            UpdateDetailsView();
            CreatePartAtTransform(m_SelectedPart, FTransform::Identity);
        }
        else {
            FHitResult HitResult = GetCastHitResult();
            if (HitResult.bBlockingHit && m_bValidPlacement) {
                SetSelectedActor(nullptr);
                UpdateDetailsView();
                if (m_PreviewActor) {
                    CreatePartAtTransform(m_SelectedPart, m_PreviewActor->GetActorTransform());
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

    if (PartData == m_SelectedPart) {
        m_SelectedPart = nullptr;
        m_PreviewActor->Destroy();
        m_PreviewActor->Reset();
        return;
    }
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
            NewActor = FEditorUtilsViewportClient::CreateActorComponent(PartData->Object, SpawnParams, true);
            if (NewActor) {
                m_PreviewActor = NewActor;
                ApplyPreviewMaterial();
            }
        }
    }
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::SetSelectedActor(AActor* Actor) {
    if (m_SelectedActor && m_SelectedActor != Actor) {
        TArray<UMeshComponent*> MeshComps;
        m_SelectedActor->GetComponents<UMeshComponent>(MeshComps);

        for (UMeshComponent* MeshComp : MeshComps) {
            if (!MeshComp) continue;
            MeshComp->SelectionOverrideDelegate.Unbind();
        }
        m_SelectedActor->PushSelectionToProxies();
    }

     m_SelectedActor = Actor;

    if (Actor) {
        TArray<UMeshComponent*> MeshComps;
        Actor->GetComponents<UMeshComponent>(MeshComps);

        for (UMeshComponent* MeshComp : MeshComps) {
            if (!MeshComp) continue;
            MeshComp->SelectionOverrideDelegate = UPrimitiveComponent::FSelectionOverride::CreateLambda(
                [this](const UPrimitiveComponent* Comp) -> bool
                {
                    return Comp && Comp->GetOwner() == m_SelectedActor;
                });
        }
        m_SelectedActor->PushSelectionToProxies();
    }
    UpdateDetailsView();
    FEditorUtilsViewportClient::Invalidate();
    return;
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

void FRobotEditorViewportClient::DestroyActor(AActor* Actor) {
    if (!Actor) {
        return;
    }

    const FScopedTransaction Transaction(NSLOCTEXT("RobotEditor", "DeletePart", "Remove Robot Part"));
    CommitUndoState();

    SetSelectedActor(nullptr);
    UpdateDetailsView();
    m_Actors.Remove(Actor);
    Actor->Destroy();

    CommitUndoState();

    UpdateOrbitTargetPoint();
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::CreatePartAtTransform(UPDAPart* Part, FTransform Transform, const TMap<FName, FPartStat>& Stats) {
    if (!Part || !Part->Object) {
        return;
    }

    const FScopedTransaction Transaction(NSLOCTEXT("RobotEditor", "CreatePart", "Add Part To Robot"));
    CommitUndoState();

    SpawnPartInternal(Part, Transform, Stats);

    CommitUndoState();

    UpdateOrbitTargetPoint();
    FEditorUtilsViewportClient::Invalidate();
    return;
}

void FRobotEditorViewportClient::SpawnPartInternal(UPDAPart* Part, FTransform Transform, const TMap<FName, FPartStat>& Stats) {
    if (!Part || !Part->Object) {
        return;
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    AActor* NewActor = FEditorUtilsViewportClient::CreateActorComponent(Part->Object, SpawnParams, true);
    if (NewActor) {
        NewActor->SetActorTransform(Transform);
        m_Actors.Emplace(NewActor, FPartData(Part, Transform, Stats));
        AddCollisionToActor(NewActor);
    }
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

    SetSelectedActor(nullptr);

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
    SetSelectedActor(nullptr);
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
    if (GEditor) {
        GEditor->ResetTransaction(NSLOCTEXT("RobotEditor", "ResetUndo", "Clear History"));
    }

    SetSelectedActor(nullptr);

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
        TMap<FName, FPartStat> Stats = Part.PartStats;

        SpawnPartInternal(VisualPart, Transform, Stats);
    }

    UpdateOrbitTargetPoint();
    FRobotEditorViewportClient::Invalidate();
}

void FRobotEditorViewportClient::ClearScene() {
    SetSelectedActor(nullptr);
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

UMaterialInstanceDynamic* FRobotEditorViewportClient::CreateTransparentMaterialInstance(const FLinearColor& Color, float Opacity) {
    UMaterialInterface* BaseMaterial = Cast<UMaterialInterface>(StaticLoadObject(
        UMaterialInterface::StaticClass(),
        nullptr,
        TEXT("/RobotEditor/Materials/M_RobotPreview.M_RobotPreview")
    ));

    if (!BaseMaterial) {
        BaseMaterial = GEngine->WireframeMaterial;
    } if (!BaseMaterial) {
        return nullptr;
    }

    UMaterialInstanceDynamic* DynamicMaterial = UMaterialInstanceDynamic::Create(BaseMaterial, m_PreviewActor);
    if (!DynamicMaterial) {
        return nullptr;
    }

    DynamicMaterial->SetVectorParameterValue(FName("Color"), Color);
    DynamicMaterial->SetScalarParameterValue(FName("Opacity"), Opacity);
    return DynamicMaterial;
}

void FRobotEditorViewportClient::ApplyPreviewMaterial() {
    if (!m_PreviewActor) {
        return;
    }

    UMaterialInstanceDynamic* DynamicMaterial = CreateTransparentMaterialInstance(FLinearColor::Green, 0.3f);

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

void FRobotEditorViewportClient::ApplyOverlayMaterial(AActor* Actor, UMaterialInstanceDynamic* Material) {
    if (!Actor || !Material) { return; }

    TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents) {
        if (!MeshComp) {
            continue;
        }
        MeshComp->SetOverlayMaterial(Material);
    }
    return;
}

void FRobotEditorViewportClient::ClearOverlayMaterial(AActor* Actor) {
    if (!Actor) { return; }

    TInlineComponentArray<UStaticMeshComponent*> MeshComponents;
    Actor->GetComponents(MeshComponents);

    for (UStaticMeshComponent* MeshComp : MeshComponents) {
        if (!MeshComp) {
            continue;
        }
        MeshComp->SetOverlayMaterial(nullptr);
    }
    return;
}

void FRobotEditorViewportClient::SetDetailsView(TSharedPtr<IStructureDetailsView> DetailsView) {
    m_DetailsView = DetailsView;

    TSharedPtr<SWidget> DetailsWidget = m_DetailsView->GetWidget();

    if (DetailsWidget.IsValid()) {
        DetailsWidget->SetVisibility(EVisibility::Collapsed);
    }
}

void FRobotEditorViewportClient::UpdateDetailsView() {
    if (!m_DetailsView) {
        return;
    }

    TSharedPtr<SWidget> DetailsWidget = m_DetailsView->GetWidget();

    if (!m_SelectedActor) {
        m_ScopedStruct.Reset();
        m_DetailsView->SetStructureData(nullptr);
        if (DetailsWidget.IsValid()) {
            DetailsWidget->SetVisibility(EVisibility::Collapsed);
        }
    }
    else {
        m_ScopedStruct = MakeShared<FStructOnScope>(
            FPartData::StaticStruct(),
            reinterpret_cast<uint8*>(&m_Actors[m_SelectedActor])
        );
        m_ScopedStruct->SetPackage(m_CurrentRobotData->GetOutermost());
        m_DetailsView->SetStructureData(m_ScopedStruct);

        if (DetailsWidget.IsValid()) {
            DetailsWidget->SetVisibility(EVisibility::Visible);
        }
    }
}

void FRobotEditorViewportClient::AddCollisionToActor(AActor* Actor) {
    if (Actor) {
        UBoxComponent* BoxComponent = NewObject<UBoxComponent>(Actor, TEXT("Box Component"));
        UPDAPart* PartData = GetPartDataFromActor(Actor).Part.Get();
        FBox BoundingBox = PartData->GetBoundingBox();

        FVector BoxExtent = BoundingBox.GetExtent();
        FVector BoxCenter = BoundingBox.GetCenter();

        if (BoxComponent) {
            BoxComponent->SetBoxExtent(BoxExtent);
            BoxComponent->SetRelativeLocation(BoxCenter);
            BoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
            BoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
            BoxComponent->SetCollisionResponseToChannel(ECC_GameTraceChannel2, ECR_Block);
            BoxComponent->SetVisibility(false);

            if (USceneComponent* RootComp = Actor->GetRootComponent()) {
                BoxComponent->SetupAttachment(RootComp);
            } else {
                Actor->SetRootComponent(BoxComponent);
            }

            BoxComponent->RegisterComponent();
            Actor->AddInstanceComponent(BoxComponent);

        }
    }
}

void FRobotEditorViewportClient::PostUndo(bool bSuccess) {
    if (!bSuccess || !m_UndoState.IsValid()) {
        return;
    }

    SetSelectedActor(nullptr);

    for (const auto& Pair : m_Actors) {
        if (Pair.Key) {
            Pair.Key->Destroy();
        }
    }
    m_Actors.Empty();

    for (const FPartData& Data : m_UndoState->PartSnapshot) {
        SpawnPartInternal(Data.Part.Get(), Data.RelativeTransform, Data.PartStats);
    }

    UpdateDetailsView();
    UpdateOrbitTargetPoint();
    FEditorUtilsViewportClient::Invalidate();
}

void FRobotEditorViewportClient::PostRedo(bool bSuccess) {
    PostUndo(bSuccess);
}

void FRobotEditorViewportClient::CommitUndoState() {
    if (!m_UndoState || !m_CurrentRobotData) {
        return;
    }

    m_UndoState->Modify();
    m_UndoState->PartSnapshot.Reset(m_Actors.Num());

    for (const TPair<AActor*, FPartData>& Pair : m_Actors) {
        FPartData PartSnapshot = Pair.Value;
        if (Pair.Key) {
            PartSnapshot.RelativeTransform = Pair.Key->GetActorTransform();
        }
        m_UndoState->PartSnapshot.Add(PartSnapshot);
    }

    m_CurrentRobotData->MarkPackageDirty();
}
