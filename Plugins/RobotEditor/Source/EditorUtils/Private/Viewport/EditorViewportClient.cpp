#include "Viewport/EditorViewportClient.h"
#include "SceneManagement.h"
#include "MouseDeltaTracker.h"
#include "Engine/Blueprint.h"
#include "Components/PrimitiveComponent.h"
#include "GeometryCollection/GeometryCollectionComponent.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IMainFrameModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"

#define LOCTEXT_NAMESPACE "EditorViewport"

FEditorUtilsViewportClient::FEditorUtilsViewportClient(FPreviewScene& InPreviewScene)
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

FEditorUtilsViewportClient::~FEditorUtilsViewportClient() {
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().RemoveAll(this);
    }
}

FSceneView* FEditorUtilsViewportClient::CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex) {
    FSceneView* View = FEditorViewportClient::CalcSceneView(ViewFamily, StereoViewIndex);
    if (!m_bChangeBackgroundColour) return View;

    if (View) {
        View->BackgroundColor = m_BackgroundColour;
    }

    return View;
}

void FEditorUtilsViewportClient::SetDetailsView(TSharedPtr<IDetailsView> InDetailsView) {
    m_AssociatedDetailsView = InDetailsView;
}

void FEditorUtilsViewportClient::Tick(float DeltaSeconds) {
    FEditorViewportClient::Tick(DeltaSeconds);
    if (Viewport == nullptr) {
        return;
    }

    if (PreviewScene) {
        UWorld* PreviewWorld = PreviewScene->GetWorld();
        if (PreviewWorld) {
            PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
        }
    }
}

bool FEditorUtilsViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    /*
    Handle zooming inputs, whether we should zoom - or + based on scroll inputs.
    */
    if (EventArgs.Key == EKeys::MouseScrollUp && EventArgs.Event == IE_Pressed) {
        ZoomCamera(-c_ZoomAmount);
        return true;
    }
    if (EventArgs.Key == EKeys::MouseScrollDown && EventArgs.Event == IE_Pressed) {
        ZoomCamera(c_ZoomAmount);
        return true;
    }

    /*
    Handle orbit logic by locking our mouse to the viewport and toggling m_bIsOrbiting so our InputAxis function can correctly update the camera position.
    Unlock the mouse on releasing our orbit button.
    */
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

    return FEditorViewportClient::InputKey(EventArgs);
}

bool FEditorUtilsViewportClient::InputAxis(const FInputKeyEventArgs& EventArgs) {

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

void FEditorUtilsViewportClient::UpdateCameraPositionFromOrbit() {
    FVector LookDirection = GetViewRotation().Vector();
    FVector NewLocation = m_OrbitTargetPoint - (LookDirection * m_CameraOrbitRadius);
    SetViewLocation(NewLocation);
}

void FEditorUtilsViewportClient::ZoomCamera(float ZoomAmount) {
    m_CameraOrbitRadius = FMath::Clamp(m_CameraOrbitRadius + ZoomAmount, c_MinZoom, c_MaxZoom);
    UpdateCameraPositionFromOrbit();
    Invalidate();
}

AActor* FEditorUtilsViewportClient::CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams) {
    if (!InObject || !InObject->GeneratedClass) { return nullptr; }
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (UClass* ActorClass = InObject->GeneratedClass) {
        AActor* SpawnedActor = PreviewWorld->SpawnActor<AActor>(ActorClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);

        SpawnedActor->SetActorTickEnabled(false);

        TInlineComponentArray<UActorComponent*> Components;
        SpawnedActor->GetComponents(Components);
        for (UActorComponent* Component : Components) {
            if (Component) {
                Component->SetComponentTickEnabled(false);
            }
        }
        return SpawnedActor;
    }
    return nullptr;
}

TTuple<FVector, FVector> FEditorUtilsViewportClient::CastRayFromCameraToMouse() {
    FViewportCursorLocation CursorLocation = GetCursorWorldLocationFromMousePos();
    FVector RayStart = CursorLocation.GetOrigin();
    FVector RayDirection = CursorLocation.GetDirection();
    FVector RayEnd = RayStart + (RayDirection * 10000.0f);

    return MakeTuple(RayStart, RayEnd);
}

void FEditorUtilsViewportClient::OnThumbnailTriggered(UPDAPart* Part) {
    UE_LOG(LogTemp, Log, TEXT("Tried to trigger thumbnail but called parent class OnThumbnailTriggered. Did you implement it?"));
    return;
}

void FEditorUtilsViewportClient::ConnectTileSingleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailClicked.AddSP(this, &FEditorUtilsViewportClient::OnThumbnailTriggered);
}

void FEditorUtilsViewportClient::ConnectTileDoubleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailDoubleClicked.AddSP(this, &FEditorUtilsViewportClient::OnThumbnailTriggered);
}

void FEditorUtilsViewportClient::OnAddButtonPressed() {
    return;
}

void FEditorUtilsViewportClient::ReloadAssetRegistryModule(const FString& AssetGamePath) {
    FString AbsolutePhysicalPath = FPackageName::LongPackageNameToFilename(AssetGamePath);
    IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
    if (!PlatformFile.DirectoryExists(*AbsolutePhysicalPath)) {
        PlatformFile.CreateDirectoryTree(*AbsolutePhysicalPath);
    }

    if (FAssetRegistryModule* AssetRegistryModule = FModuleManager::LoadModulePtr<FAssetRegistryModule>("AssetRegistry")) {
        IAssetRegistry& AssetRegistry = AssetRegistryModule->Get();
        AssetRegistry.AddPath(AssetGamePath);

        TArray<FString> PathsToScan;
        PathsToScan.Add(AssetGamePath);

        AssetRegistry.ScanPathsSynchronous(PathsToScan, true);
    }
}

#undef LOCTEXT_NAMESPACE
