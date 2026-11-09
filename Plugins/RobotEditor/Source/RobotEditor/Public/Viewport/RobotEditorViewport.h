#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/EditorViewport.h"
#include "Viewport/RobotEditorViewportClient.h"

class SRobotEditorViewport : public SEditorUtilsViewport {
public:
    SLATE_BEGIN_ARGS(SRobotEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SRobotEditorViewport() override {}

    virtual TSharedPtr<FEditorUtilsViewportClient> GetViewportClient() override { return m_ViewportClient; }
    virtual void SetEditingObject(UObject* NewObject) override {}

    virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override;
    virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override;

protected:

    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient();
    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;

private:

    TSharedPtr<FEditorUtilsViewportClient> m_ViewportClient;
};
