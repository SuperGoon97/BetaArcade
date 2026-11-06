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
    virtual void UpdatePreviewActor();
    virtual void MovePreviewActorToLocation(FVector Location);
    virtual void CreatePartAtLocation(FVector Location);

    virtual void SetSelectedActorsData(const FPartData& PartData);

    TArray<FTransform> GetAnchorsRelativeToPart(UPDAPart* Part);
    FPartData GetSelectedPartData();
    AActor* GetActor();
    FVector GetActorLocation(AActor* Actor);

    virtual void OnThumbnailTriggered(UPDAPart* Part) override;
    virtual void ConnectTileSingleClick(SThumbnailTile* Tile) override;
    virtual void ConnectTileDoubleClick(SThumbnailTile* Tile) override;
    virtual void OnAddButtonPressed() override;

private:

    const FLinearColor c_PreviewColour = FLinearColor(1.0f, 0.2f, 0.2f, 0.3f);
    const FLinearColor c_AnchorColour = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const FLinearColor c_HighlightedAnchorColour = FLinearColor(1.0f, 0.5f, 0.5f, 1.0f);

    TObjectPtr<UPDAPart> m_SelectedPart;
    TObjectPtr<AActor> m_PreviewActor;
    TMap<TObjectPtr<AActor>, FPartData> m_Actors;
    TObjectPtr<AActor> m_SelectedActor;

};
