#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FRobotEditorViewportClient : public FEditorViewportClient {
public:
    FRobotEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FRobotEditorViewportClient() override {}

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

private:
    FVector m_OrbitTargetPoint;
    float m_CameraOrbitRadius;
    bool m_bIsOrbiting;

    float MinZoom = 50.0f;
    float MaxZoom = 5000.0f;

    void UpdateCameraPositionFromOrbit();
    void ZoomCamera(float ZoomAmount);
};
