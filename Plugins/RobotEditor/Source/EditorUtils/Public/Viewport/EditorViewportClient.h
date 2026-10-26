#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

class EDITORUTILS_API FEditorUtilsViewportClient : public FEditorViewportClient {
public:
    FEditorUtilsViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FEditorUtilsViewportClient() override;

    void SaveActiveAsset();
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView);
    void SetInspectedObject(UObject* NewPartObject);

    TSharedPtr<IDetailsView> GetDetailsView() { return m_AssociatedDetailsView; }

    virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex = INDEX_NONE) override;

private:

    const float c_MinZoom = 50.0f;
    const float c_MaxZoom = 5000.0f;
    const float c_ZoomAmount = 50.0f;

    bool m_bIsOrbiting = false;
    float m_CameraOrbitRadius = 50.0;
    bool m_bChangeBackgroundColour = false;
    FLinearColor m_BackgroundColour = FLinearColor(0.12f, 0.14f, 0.18f, 1.0f);
    FVector m_OrbitTargetPoint = FVector::ZeroVector;

protected:
    TSharedPtr<IDetailsView> m_AssociatedDetailsView;

private:

    void UpdateCameraPositionFromOrbit();
    void ZoomCamera(float ZoomAmount);
    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

};
