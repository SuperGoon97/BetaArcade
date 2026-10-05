#include "Viewport/PartEditorViewport.h"
#include "Widgets/SViewport.h"
#include "Components/SkyLightComponent.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "SViewportToolBar.h"
#include "Framework/Commands/UICommandInfo.h"

void SPartEditorViewport::Construct(const FArguments& InArgs) {
    m_PreviewScene = MakeUnique<FPreviewScene>();
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

SPartEditorViewport::~SPartEditorViewport()
{
    if (m_ViewportClient.IsValid()) {
        m_ViewportClient->Viewport = nullptr;
    }
    if (m_PreviewScene.IsValid()) {
        m_PreviewScene.Reset();
    }
}

TSharedRef<FEditorViewportClient> SPartEditorViewport::MakeEditorViewportClient() {
    if (m_PreviewScene.IsValid()) {
        m_ViewportClient = MakeShareable(new FPartEditorViewportClient(*m_PreviewScene.Get()));
    } else {
        m_PreviewScene = MakeUnique<FPreviewScene>();
        m_ViewportClient = MakeShareable(new FPartEditorViewportClient(*m_PreviewScene.Get()));
    }

    m_ViewportClient->ViewportType = LVT_Perspective;
    m_ViewportClient->bDrawAxes = true;

    return m_ViewportClient.ToSharedRef();
}

TSharedPtr<SWidget> SPartEditorViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}

void SPartEditorViewport::SetEditingObject(UObject* NewObject) {
    if (m_ViewportClient.IsValid()) {
        m_ViewportClient->SetInspectedObject(NewObject);
        m_ViewportClient->SetSelectedAnchorIndex(-1);
    }
    return;
}

FReply SPartEditorViewport::OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    TSharedPtr<FAssetDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
    if (DragDropOp.IsValid()) {
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SPartEditorViewport::OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) {
    TSharedPtr<FAssetDragDropOp> DragDropOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>();
    if (DragDropOp.IsValid() && DragDropOp->GetAssets().Num() > 0) {
        const FAssetData& DroppedAssetData = DragDropOp->GetAssets()[0];
        UObject* LoadedAsset = DroppedAssetData.GetAsset();
        if (LoadedAsset) {
            SetEditingObject(LoadedAsset);
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

FReply SPartEditorViewport::OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) {
    if (InKeyEvent.IsControlDown() && InKeyEvent.GetKey() == EKeys::S) {
        TSharedPtr<FPartEditorViewportClient> PartClient = GetPartClient();
        if (PartClient.IsValid()) {
            PartClient->SaveActiveAsset();
            return FReply::Handled();
        }
    }
    return SWidget::OnKeyDown(MyGeometry, InKeyEvent);
}
