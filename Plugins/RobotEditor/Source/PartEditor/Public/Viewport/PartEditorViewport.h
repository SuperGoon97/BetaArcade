#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/EditorViewport.h"
#include "Viewport/PartEditorViewportClient.h"

class SPartEditorViewport : public SEditorUtilsViewport {
public:
    SLATE_BEGIN_ARGS(SPartEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SPartEditorViewport() override;

    virtual TSharedPtr<FEditorUtilsViewportClient> GetViewportClient() override { return m_ViewportClient; }

    void SetEditingObject(UObject* NewObject);

    virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;

private:

    TSharedPtr<FPartEditorViewportClient> m_ViewportClient;

};
