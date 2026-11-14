#pragma once

#include "CoreMinimal.h"
#include "Viewport/EditorViewportClient.h"
#include "Data/PartDat.h"
#include "Data/TransactionalRobotData.h"
#include "Structs/RobotData.h"
#include "Structs/SnapResult.h"
#include "IStructureDetailsView.h"
#include "UObject/StructOnScope.h"
#include "EditorViewportClient.h"
#include "EditorUndoClient.h"
#include "PreviewScene.h"

class FRobotEditorViewportClient : public FEditorUtilsViewportClient, public FEditorUndoClient {
public:
    FRobotEditorViewportClient(FPreviewScene& InPreviewScene);

    virtual void PostUndo(bool bSuccess) override;
    virtual void PostRedo(bool bSuccess) override;

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;

    virtual void SetDetailsView(TSharedPtr<IStructureDetailsView> DetailsView);
    virtual void UpdateDetailsView();

    virtual void OnThumbnailTriggered(UPDAPart* Part) override;
    virtual void ConnectTileSingleClick(SThumbnailTile* Tile) override;
    virtual void ConnectTileDoubleClick(SThumbnailTile* Tile) override;
    virtual void OnAddButtonPressed() override;

    virtual void SaveActiveAsset();
    virtual FHitResult GetCastHitResult();

    void CommitUndoState();

    void ClearScene();
    void CreateNewRobot();
    void LoadRobot(URobotDataContainer* RobotData);

    void UpdateOrbitTargetPoint();
    void ApplyPreviewMaterial();
    FVector GetCenterOfMass();

    void SaveNewAsset(URobotDataContainer* RobotData, const FString& ChosenPackagePath, const FString& CurrentName);
    void SetSelectedPart(UPDAPart* PartData);

    void UpdatePreviewActor(bool ShouldShow);
    void MovePreviewActorToLocation(FVector Location);

    void DestroyActor(AActor* Actor);
    void SetSelectedActor(AActor* Actor);
    void SetSelectedActorsData(const FPartData& PartData);
    void CreatePartAtTransform(UPDAPart* Part, FTransform Transform, const TMap<FName, FPartStat>& Stats = {});
    void SpawnPartInternal(UPDAPart* Part, FTransform Transform, const TMap<FName, FPartStat>& Stats = {});
    FPartData GetPartDataFromActor(AActor* Actor);
    TArray<FTransform> GetAnchorsRelativeToPart(UPDAPart* Part);
    AActor* GetActor();
    FVector GetActorLocation(AActor* Actor);

    bool HandleObjectHighlight(AActor* Actor);
    bool HandleObjectSnapping(const FHitResult& bHit);

    void AddCollisionToActor(AActor* Actor);

    UMaterialInstanceDynamic* CreateTransparentMaterialInstance(const FLinearColor& Color, float Opacity);
    void ApplyOverlayMaterial(AActor* Actor, UMaterialInstanceDynamic* Material);
    void ClearOverlayMaterial(AActor* Actor);

private:

    const FLinearColor c_PreviewColour = FLinearColor(1.0f, 0.2f, 0.2f, 0.3f);
    const FLinearColor c_AnchorColour = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const FLinearColor c_HighlightedAnchorColour = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f);

    TObjectPtr<UPDAPart> m_SelectedPart;
    TObjectPtr<AActor> m_PreviewActor;
    TObjectPtr<AActor> m_SelectedActor;
    TObjectPtr<AActor> m_HoveredActor;
    TObjectPtr<URobotDataContainer> m_CurrentRobotData;
    TSharedPtr<IStructureDetailsView> m_DetailsView;
    TSharedPtr<FStructOnScope> m_ScopedStruct;

    TStrongObjectPtr<URobotEditorUndoState> m_UndoState;

    FVector m_TargetOrbit = FVector::ZeroVector;

    int m_CurrentRotationNum = 0;
    bool m_bValidPlacement = false;


public:
    UPROPERTY()
    TMap<TObjectPtr<AActor>, FPartData> m_Actors;

};
