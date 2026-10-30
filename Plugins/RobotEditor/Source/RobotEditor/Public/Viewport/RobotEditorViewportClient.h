#pragma once

#include "CoreMinimal.h"
#include "Viewport/EditorViewportClient.h"
#include "EditorViewportClient.h"
#include "PreviewScene.h"

class FRobotEditorViewportClient : public FEditorUtilsViewportClient {
public:
    FRobotEditorViewportClient(FPreviewScene& InPreviewScene);

    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;
    virtual void Tick(float DeltaSeconds) override;

private:

    TObjectPtr<AActor> m_PreviewActor;

};
