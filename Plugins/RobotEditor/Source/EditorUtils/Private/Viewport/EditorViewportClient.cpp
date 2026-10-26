#include "Viewport/EditorViewportClient.h"
#include "SceneManagement.h"
#include "MouseDeltaTracker.h"
#include "Framework/Application/SlateApplication.h"
#include "Interfaces/IMainFrameModule.h"

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
    PreviewScene->GetWorld()->Tick(LEVELTICK_All, DeltaSeconds);
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


#undef LOCTEXT_NAMESPACE
