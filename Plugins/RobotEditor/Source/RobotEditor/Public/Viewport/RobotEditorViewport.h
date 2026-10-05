#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"
#include "Viewport/RobotEditorViewportClient.h"

class SRobotEditorViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SRobotEditorViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);
    virtual ~SRobotEditorViewport() override {}

protected:
    virtual TSharedRef<FEditorViewportClient> MakeEditorViewportClient() override;
    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;

private:
    FPreviewScene m_PreviewScene;
    TSharedPtr<FRobotEditorViewportClient> m_ViewportClient;
};
