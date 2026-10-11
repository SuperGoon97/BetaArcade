#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

DECLARE_MULTICAST_DELEGATE(FOnSelectedObjectChanged);

class FPartEditorViewportClient : public FEditorViewportClient {
public:
    FPartEditorViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FPartEditorViewportClient() override;

    void SaveActiveAsset();
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView);
    TSharedPtr<IDetailsView> GetDetailsView() { return m_AssociatedDetailsView; }
    void SetInspectedObject(UObject* NewPartObject);
    TWeakObjectPtr<UObject> GetInspectedObject() { return m_CurrentInspectedObject; }

    void SetSelectedAnchorIndex(int32 NewIndex) { m_SelectedAnchorIndex = NewIndex; }
    int32 GetSelectedAnchorIndex() const { return m_SelectedAnchorIndex; }

    virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override { return true; }
    virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
    virtual FVector GetWidgetLocation() const override;
    virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;

    FOnSelectedObjectChanged OnSelectedObjectChanged;

private:
    FVector m_OrbitTargetPoint;
    float m_CameraOrbitRadius;
    bool m_bIsOrbiting;

    TSharedPtr<IDetailsView> m_AssociatedDetailsView;
    TWeakObjectPtr<UObject> m_CurrentInspectedObject;
    TWeakObjectPtr<AActor> m_SpawnedPreviewActor;

    const FColor c_UnselectedAnchorColor = FColor(255, 255, 255);
    const FColor c_SelectedAnchorColor = FColor(0, 255, 64);
    const float c_AnchorSize = 12.0f;
    const float c_SelectedAnchorSize = c_AnchorSize * 1.5;

    int32 m_SelectedAnchorIndex = -1;

    AStaticMeshActor* CreateStaticMeshComponent(UStaticMesh* InObject, FActorSpawnParameters& SpawnParams);
    AActor* CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams);
    AActor* SpawnActorComponent(UObject* InObject, FActorSpawnParameters& SpawnParams);

    void UpdateCameraPositionFromOrbit();
    void ZoomCamera(float ZoomAmount);

    const float c_MinZoom = 50.0f;
    const float c_MaxZoom = 5000.0f;
    const float c_ZoomAmount = 50.0f;

    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

};
