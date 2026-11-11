#pragma once

#include "CoreMinimal.h"
#include "EditorViewportClient.h"
#include "Templates/Tuple.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

class SThumbnailTile;
class UPDAPart;

class EDITORUTILS_API FEditorUtilsViewportClient : public FEditorViewportClient, public TSharedFromThis<FEditorUtilsViewportClient> {
public:
    FEditorUtilsViewportClient(FPreviewScene& InPreviewScene);
    virtual ~FEditorUtilsViewportClient() override;

    virtual void SaveActiveAsset() {};
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual bool InputAxis(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView);
    void SetInspectedObject(UObject* NewPartObject);

    TSharedPtr<IDetailsView> GetDetailsView() { return m_AssociatedDetailsView; }

    virtual FSceneView* CalcSceneView(FSceneViewFamily* ViewFamily, const int32 StereoViewIndex = INDEX_NONE) override;
    virtual void OnThumbnailTriggered(UPDAPart* Part) = 0;
    virtual void ConnectTileSingleClick(SThumbnailTile* Tile) = 0;
    virtual void ConnectTileDoubleClick(SThumbnailTile* Tile) = 0;
    virtual void OnAddButtonPressed() = 0;

    virtual void ReloadAssetRegistryModule(const FString& AssetGamePath);

private:

    const float c_MinZoom = 50.0f;
    const float c_MaxZoom = 5000.0f;
    const float c_ZoomAmount = 50.0f;

    bool m_bIsOrbiting = false;
    float m_CameraOrbitRadius = 50.0;
    bool m_bChangeBackgroundColour = false;
    FLinearColor m_BackgroundColour = FLinearColor(0.12f, 0.14f, 0.18f, 1.0f);

protected:

    TSharedPtr<IDetailsView> m_AssociatedDetailsView;
    FVector m_OrbitTargetPoint = FVector::ZeroVector;

private:

    void ZoomCamera(float ZoomAmount);
    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);

protected:

    void UpdateCameraPositionFromOrbit();
    AActor* CreateActorComponent(UBlueprint* InObject, FActorSpawnParameters& SpawnParams);
    TTuple<FVector, FVector> CastRayFromCameraToMouse();

private:

    FVector GetLocationFromPoint(const FVector& Point) const;
};
