#include "Viewport/PartEditorViewportClient.h"
#include "Engine/StaticMesh.h"
#include "Engine/Blueprint.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PrimitiveComponent.h"
#include "SceneManagement.h"
#include "Viewport/Utils/ContentPanel.h"
#include "DrawDebugHelpers.h"
#include "RobotData/PartDat.h"
#include "MouseDeltaTracker.h"
#include "Framework/Application/SlateApplication.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UObject/Package.h"
#include "Interfaces/IMainFrameModule.h"
#include "UnrealWidget.h"

FPartEditorViewportClient::FPartEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorViewportClient(nullptr, &InPreviewScene) {
        bDrawAxes = true;
        SetViewMode(VMI_Unlit);
        SetViewportType(LVT_Perspective);
        DrawHelper.bDrawGrid = true;

        m_OrbitTargetPoint = FVector::ZeroVector;
        m_CameraOrbitRadius = 50.0f;
        m_bIsOrbiting = false;

        SetViewRotation(FRotator(-30.0f, -45.0f, 0.0f));
        UpdateCameraPositionFromOrbit();
        EngineShowFlags.SetEyeAdaptation(false);
        Invalidate();
}

void FPartEditorViewportClient::SetDetailsView(TSharedPtr<IDetailsView> InDetailsView) {
    m_AssociatedDetailsView = InDetailsView;
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().AddRaw(this, &FPartEditorViewportClient::OnInspectorPropertyChanged);
    }
}

FPartEditorViewportClient::~FPartEditorViewportClient() {
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().RemoveAll(this);
    }
}

void FPartEditorViewportClient::OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) {
    UE_LOG(LogTemp, Log, TEXT("Property on edited object has changed, updating and re-rendering."))
    if (!m_CurrentInspectedObject.IsValid()) return;

    UObject* ActiveObject = m_CurrentInspectedObject.Get();
    SetInspectedObject(ActiveObject);
}

void FPartEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);

    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
}

bool FPartEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    if (EventArgs.Key == EKeys::MouseScrollUp && EventArgs.Event == IE_Pressed) {
        ZoomCamera(-c_ZoomAmount);
        return true;
    }
    if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed) {
        ZoomCamera(c_ZoomAmount);
        return true;
    }

    if (EventArgs.Key == EKeys::RightMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            m_bIsOrbiting = true;
            Viewport->LockMouseToViewport(true);
        }
        else if (EventArgs.Event == IE_Released) {
            m_bIsOrbiting = false;
            Viewport->LockMouseToViewport(false);
        }
        return true;
    }

    if (EventArgs.Key == EKeys::LeftMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            if (Viewport && m_CurrentInspectedObject.IsValid()) {
                int32 MouseX = Viewport->GetMouseX();
                int32 MouseY = Viewport->GetMouseY();

                HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
                if (HitResult && HitResult->IsA(HWidgetAxis::StaticGetType())) {
                    HWidgetAxis* WidgetAxis = (HWidgetAxis*)HitResult;
                    SetCurrentWidgetAxis(WidgetAxis->Axis);
                    return FEditorViewportClient::InputKey(EventArgs);
                }

                SetCurrentWidgetAxis(EAxisList::None);

                FViewportCursorLocation CursorLocation = GetCursorWorldLocationFromMousePos();
                FVector RayOrigin = CursorLocation.GetOrigin();
                FVector RayDirection = CursorLocation.GetDirection();
                FVector RayEnd = RayOrigin + (RayDirection * 10000.0f);

                if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                    float BestHitDistance = FLT_MAX;
                    int32 SelectedAnchorIndex = INDEX_NONE;

                    for (int32 i = 0; i < ActivePart->Anchors.Num(); i++) {
                        FVector PointPos = ActivePart->Anchors[i].Position;

                        FVector ClosestPointOnRay = FMath::ClosestPointOnSegment(PointPos, RayOrigin, RayEnd);
                        float DistanceToRay = FVector::Dist(PointPos, ClosestPointOnRay);

                        if (DistanceToRay < 25.0f &&  DistanceToRay < BestHitDistance) {
                            BestHitDistance = DistanceToRay;
                            SelectedAnchorIndex = i;
                        }
                    }

                    m_SelectedAnchorIndex = SelectedAnchorIndex;
                    Invalidate();
                }
            }
            return true;
        }
        else if (EventArgs.Event == IE_Released) {
            if (GetCurrentWidgetAxis() != EAxisList::None) {
                SetCurrentWidgetAxis(EAxisList::None);
                Invalidate();
                return FEditorViewportClient::InputKey(EventArgs);
            }

            SetCurrentWidgetAxis(EAxisList::None);
            Invalidate();
            return true;
        }
    }

    return FEditorViewportClient::InputKey(EventArgs);
}

bool FPartEditorViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs) {

    EAxisList::Type ActiveAxis = GetCurrentWidgetAxis();
    float Delta = EventArgs.AmountDepressed;

    if (ActiveAxis != EAxisList::None) {
        return FEditorViewportClient::InputAxis(EventArgs);
    }


    if (m_bIsOrbiting) {
        FRotator CurrentRotation = GetViewRotation();

        if (EventArgs.Key == EKeys::MouseX) {
            CurrentRotation.Yaw += Delta * 0.5f;
            SetViewRotation(CurrentRotation);
            UpdateCameraPositionFromOrbit();
            Invalidate();
            return true;
        }
        else if (EventArgs.Key == EKeys::MouseY) {
            CurrentRotation.Pitch = FMath::Clamp(CurrentRotation.Pitch + (Delta * 0.5f), -85.0f, 85.0f);
            SetViewRotation(CurrentRotation);
            UpdateCameraPositionFromOrbit();
            Invalidate();
            return true;
        }
    }
    return FEditorViewportClient::InputAxis(EventArgs);
}

void FPartEditorViewportClient::UpdateCameraPositionFromOrbit() {
    FVector LookDirection = GetViewRotation().Vector();
    FVector NewLocation = m_OrbitTargetPoint - (LookDirection * m_CameraOrbitRadius);
    SetViewLocation(NewLocation);
}

void FPartEditorViewportClient::SetInspectedObject(UObject* NewPartObject) {
    if (!NewPartObject) return;

    m_CurrentInspectedObject = NewPartObject;

    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->SetObject(NewPartObject);
    }

    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    if (m_SpawnedPreviewActor.IsValid()) {
        m_SpawnedPreviewActor->Destroy();
        m_SpawnedPreviewActor.Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    UPDAPart* DataAsset = Cast<UPDAPart>(NewPartObject);

    if (!DataAsset) {
        NewActor = SpawnActorComponent(NewPartObject, SpawnParams);
    } else {
        if (DataAsset->Object) {
            NewActor = SpawnActorComponent(DataAsset->Object, SpawnParams);
         }
    }

    if (NewActor) {
        m_SpawnedPreviewActor = NewActor;

        TInlineComponentArray<UActorComponent*> ActorComponents;
        NewActor->GetComponents(ActorComponents);
        for (UActorComponent* Component : ActorComponents) {
            if (UPrimitiveComponent* PrimitiveComponent = Cast<UPrimitiveComponent>(Component)) {
                PreviewScene->AddComponent(PrimitiveComponent, PrimitiveComponent->GetRelativeTransform());
            }
        }
    }

    OnSelectedObjectChanged.Broadcast();
    Invalidate();
}

AStaticMeshActor* FPartEditorViewportClient::CreateStaticMeshComponent(UStaticMesh* InObject, FActorSpawnParameters& SpawnParams) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    AStaticMeshActor* MeshActor = PreviewWorld->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    if (MeshActor && MeshActor->GetStaticMeshComponent()) {
        MeshActor->GetStaticMeshComponent()->SetStaticMesh(InObject);
        return MeshActor;
    }
    return nullptr;
}

AActor* FPartEditorViewportClient::CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (UClass* ActorClass = InObject->GeneratedClass) {
        return PreviewWorld->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
    }
    return nullptr;
}

AActor* FPartEditorViewportClient::SpawnActorComponent(UObject* InObject, FActorSpawnParameters& SpawnParams) {
    if (UStaticMesh* InnerMesh = Cast<UStaticMesh>(InObject)) {
        return CreateStaticMeshComponent(InnerMesh, SpawnParams);
    }
    else if (UBlueprint* InnerBP = Cast<UBlueprint>(InObject)) {
        return CreateActorComponent(InnerBP, SpawnParams);
    }
    return nullptr;
}

void FPartEditorViewportClient::ZoomCamera(float ZoomAmount) {
    m_CameraOrbitRadius = FMath::Clamp(m_CameraOrbitRadius + ZoomAmount, c_MinZoom, c_MaxZoom);
    UpdateCameraPositionFromOrbit();
    Invalidate();
}

void FPartEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorViewportClient::Draw(View, PDI);

    if (!PDI) return;

    if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
        for (int32 i = 0; i < ActivePart->Anchors.Num(); i++) {
            FVector AnchorLocation = ActivePart->Anchors[i].Position;

            FColor RenderColor = c_UnselectedAnchorColor;
            float CorePointSize = c_AnchorSize;

            if (i == m_SelectedAnchorIndex) {
                RenderColor = c_SelectedAnchorColor;
                CorePointSize = c_SelectedAnchorSize;
            }

            PDI->DrawPoint(AnchorLocation, FLinearColor(RenderColor), CorePointSize, SDPG_World);

            if (i == m_SelectedAnchorIndex) {
                FVector ZeroProjection = FVector::ZeroVector;
                PDI->DrawLine(AnchorLocation, ZeroProjection, FLinearColor(RenderColor), SDPG_World, 0.2f);
            }
        }
    }
}

UE::Widget::EWidgetMode FPartEditorViewportClient::GetWidgetMode() const {
    if (m_SelectedAnchorIndex != INDEX_NONE && m_CurrentInspectedObject.IsValid()) {
        return UE::Widget::WM_Translate;
    }

    return UE::Widget::WM_None;
}

FVector FPartEditorViewportClient::GetWidgetLocation() const {
    if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
        if (ActivePart->Anchors.IsValidIndex(m_SelectedAnchorIndex)) {
            return ActivePart->Anchors[m_SelectedAnchorIndex].Position;
        }
    }
    return FVector::ZeroVector;
}

bool FPartEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InAxis, FVector& Drag, FRotator& Rot, FVector& Scale) {
    if (InAxis == EAxisList::None || Drag.IsZero()) {
        return false;
    }

    if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
        if (ActivePart->Anchors.IsValidIndex(m_SelectedAnchorIndex)) {
            ActivePart->Anchors[m_SelectedAnchorIndex].Position += Drag;
            ActivePart->MarkPackageDirty();

            if (m_AssociatedDetailsView.IsValid()) {
                m_AssociatedDetailsView->SetObject(ActivePart, true);
            }

            Invalidate();
            return true;
        }
    }
    return false;
}

void FPartEditorViewportClient::SaveActiveAsset() {
    if (!m_CurrentInspectedObject.IsValid()) return;

    UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
    if (!ActivePart) return;

    UPackage* CurrentPackage = ActivePart->GetPackage();
    FString CurrentName = ActivePart->PartName;
    CurrentName = CurrentName.Replace(TEXT(" "), TEXT("_"));

    if (CurrentName.IsEmpty() || CurrentName.Len() == 0) {
        CurrentName = TEXT("UNNAMED");
    }

    ActivePart->PartName = CurrentName;

    if (CurrentPackage != GetTransientPackage()) {
        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(CurrentPackage);

        CurrentPackage->MarkPackageDirty();
        ActivePart->MarkPackageDirty();

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
    } else if (CurrentName.Equals(TEXT("UNNAMED"), ESearchCase::IgnoreCase)) {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

        FSaveAssetDialogConfig SaveDialogConfig;
        SaveDialogConfig.DialogTitleOverride = INVTEXT("Save new Asset");
        SaveDialogConfig.DefaultPath = TEXT("/RobotEditor/Data");
        SaveDialogConfig.DefaultAssetName = TEXT("Unnamed");
        SaveDialogConfig.AssetClassNames.Add(UPDAPart::StaticClass()->GetClassPathName());

        FOnObjectPathChosenForSave OnSavePressed = FOnObjectPathChosenForSave::CreateLambda([this, ActivePart](const FString& ChosenPackagePath) {
            FString TargetFolderPackagePath = FPaths::GetPath(ChosenPackagePath);
            FString CleanAssetName = FPaths::GetBaseFilename(ChosenPackagePath);

            ActivePart->Rename(*CleanAssetName, ActivePart->GetOuter());

            FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
            UPDAPart* NewSavedAsset = Cast<UPDAPart>(AssetToolsModule.Get().CreateAsset(
                CleanAssetName,
                TargetFolderPackagePath,
                UPDAPart::StaticClass(),
                nullptr
            ));

            if (NewSavedAsset) {
                NewSavedAsset->Object = ActivePart->Object;
                NewSavedAsset->Anchors = ActivePart->Anchors;
                NewSavedAsset->PartName = CleanAssetName;

                NewSavedAsset->MarkPackageDirty();
                NewSavedAsset->GetPackage()->MarkPackageDirty();

                TArray<UPackage*> PackagesToSave;
                PackagesToSave.Add(NewSavedAsset->GetPackage());

                UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

                SetInspectedObject(NewSavedAsset);
            }
        });

        FOnAssetDialogCancelled OnCancelPressed = FOnAssetDialogCancelled::CreateLambda([]() {
            UE_LOG(LogTemp, Warning, TEXT("Save action cancelled."));
        });

        ContentBrowserModule.Get().CreateSaveAssetDialog(SaveDialogConfig, OnSavePressed, OnCancelPressed);
    } else if (CurrentPackage == GetTransientPackage()) {
        FString TargetFolderPackagePath = TEXT("/RobotEditor/Data");

        FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
        FString UniquePackagePath;
        FString UniqueAssetName;

        AssetToolsModule.Get().CreateUniqueAssetName(TargetFolderPackagePath + TEXT("/") + CurrentName, TEXT(""), UniquePackagePath, UniqueAssetName);

        UPDAPart* NewSavedAsset = Cast<UPDAPart>(AssetToolsModule.Get().CreateAsset(
            UniqueAssetName,
            TargetFolderPackagePath,
            UPDAPart::StaticClass(),
            nullptr
        ));

        if (NewSavedAsset) {
            NewSavedAsset->Object = ActivePart->Object;
            NewSavedAsset->Anchors = ActivePart->Anchors;
            NewSavedAsset->PartName = ActivePart->PartName;

            NewSavedAsset->MarkPackageDirty();
            NewSavedAsset->GetPackage()->MarkPackageDirty();

            TArray<UPackage*> PackagesToSave;
            PackagesToSave.Add(NewSavedAsset->GetPackage());

            UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

            SetInspectedObject(NewSavedAsset);
        }
    }

    if (SPartContentBrowser::m_Instance.IsValid()) {
        SPartContentBrowser::m_Instance.Pin()->ScanPluginDirectory();
    }
}
