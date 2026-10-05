#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/PartEditorViewportClient.h"

class SPartEditorViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SPartEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SPartEditorViewport() override;

    TSharedPtr<FPartEditorViewportClient> GetPartClient() const { return m_ViewportClient; }

    void SetEditingObject(UObject* NewObject);

    virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;

private:
    TUniquePtr<FPreviewScene> m_PreviewScene;
    TSharedPtr<FPartEditorViewportClient> m_ViewportClient;
};
