#include "Viewport/RobotEditorViewport.h"
#include "Widgets/SViewport.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "Structs/RobotData.h"
#include "Components/SkyLightComponent.h"
#include "SViewportToolBar.h"

void SRobotEditorViewport::Construct(const FArguments& InArgs) {
    SEditorUtilsViewport::Construct(SEditorUtilsViewport::FArguments());
}

TSharedRef<FEditorViewportClient> SRobotEditorViewport::MakeEditorViewportClient() {
    m_ViewportClient = MakeShareable(new FRobotEditorViewportClient(*m_PreviewScene.Get()));
    return m_ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SRobotEditorViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}

FReply SRobotEditorViewport::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    TSharedPtr<FAssetDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
    if (DragDropOp.IsValid()) {
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SRobotEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    TSharedPtr<FAssetDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
    if (DragDropOp.IsValid() && DragDropOp->GetAssets().Num() > 0) {
        const FAssetData& DroppedAssetData = DragDropOp->GetAssets()[0];
        UObject* LoadedAsset = DroppedAssetData.GetAsset();
        URobotDataContainer* CastData = Cast<URobotDataContainer>(LoadedAsset);
        if (CastData) {
            TSharedPtr<FRobotEditorViewportClient> CastViewport = StaticCastSharedPtr<FRobotEditorViewportClient>(m_ViewportClient);
            if (CastViewport.IsValid()) {
                CastViewport->LoadRobot(CastData);
            }
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

FReply SRobotEditorViewport::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
    if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::S) {
        TSharedPtr<FEditorUtilsViewportClient> _Client = GetViewportClient();
        TSharedPtr<FRobotEditorViewportClient> RobotEditorClient = StaticCastSharedPtr<FRobotEditorViewportClient>(_Client);
        if (RobotEditorClient.IsValid()) {
            RobotEditorClient->SaveActiveAsset();
            return FReply::Handled();
        }
    }
    return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
}
