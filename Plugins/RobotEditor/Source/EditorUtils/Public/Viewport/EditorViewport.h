#pragma once

#include "CoreMinimal.h"
#include "SEditorViewport.h"
#include "PreviewScene.h"

class FEditorUtilsViewportClient;

class EDITORUTILS_API SEditorUtilsViewport : public SEditorViewport {
public:
    SLATE_BEGIN_ARGS(SEditorUtilsViewport) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

    virtual TSharedPtr<FEditorUtilsViewportClient> GetViewportClient() = 0;
    virtual void SetEditingObject(UObject* NewObject) = 0;

protected:

    virtual TSharedPtr<SWidget> BuildViewportToolbar() override;

protected:
    TUniquePtr<FPreviewScene> m_PreviewScene;
};
