#pragma once

#include "CoreMinimal.h"
#include "Viewport/EditorViewportClient.h"
#include "Data/PartDat.h"
#include "Structs/RobotData.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FRobotEditorViewportClient : public FEditorUtilsViewportClient {
public:
    FRobotEditorViewportClient(FPreviewScene& InPreviewScene);

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

    virtual void SetSelectedPart(UPDAPart* PartData);
    virtual void UpdatePreviewActor(bool ShouldShow);
    virtual void MovePreviewActorToLocation(FVector Location);
    virtual void CreatePartAtTransform(UPDAPart* Part, FTransform Transform);
    virtual void SetSelectedActorsData(const FPartData& PartData);

    virtual FHitResult GetCastHitResult();

    TTuple<const FPartData& /*HitData*/, int /*HitID*/, int /*PreviewID*/> SnapToNearestAnchor(const FHitResult& HitResult, AActor* SnapActor);
    void CreatePartFromPreview();
    FPartData GetPartDataFromActor(AActor* Actor);

    TArray<FTransform> GetAnchorsRelativeToPart(UPDAPart* Part);
    AActor* GetActor();
    FVector GetActorLocation(AActor* Actor);

    virtual void OnThumbnailTriggered(UPDAPart* Part) override;
    virtual void ConnectTileSingleClick(SThumbnailTile* Tile) override;
    virtual void ConnectTileDoubleClick(SThumbnailTile* Tile) override;
    virtual void OnAddButtonPressed() override;

    virtual void SaveActiveAsset();
    void SaveNewAsset(URobotDataContainer* RobotData, const FString& ChosenPackagePath, const FString& CurrentName);

    void CreateNewRobot();
    void LoadRobot(URobotDataContainer* RobotData);
    void ClearScene();

    void UpdateOrbitTargetPoint();
    void ApplyPreviewMaterial();
    FVector GetCenterOfMass();

private:

    const FLinearColor c_PreviewColour = FLinearColor(1.0f, 0.2f, 0.2f, 0.3f);
    const FLinearColor c_AnchorColour = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const FLinearColor c_HighlightedAnchorColour = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f);

    TObjectPtr<UPDAPart> m_SelectedPart;
    TObjectPtr<AActor> m_PreviewActor;
    TMap<TObjectPtr<AActor>, FPartData> m_Actors;
    TObjectPtr<AActor> m_SelectedActor;
    TObjectPtr<URobotDataContainer> m_CurrentRobotData;

    FVector m_TargetOrbit = FVector::ZeroVector;

    int m_CurrentRotationNum = 0;

};
