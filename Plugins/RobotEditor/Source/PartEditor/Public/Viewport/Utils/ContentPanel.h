#pragma once

#include "CoreMinimal.h"
#include "Viewport/Utils/PartContentBrowser.h"

typedef TSharedPtr<FAssetData> FPartAssetEntryPtr;

class SPartThumbnailTile : public SThumbnailTile {
public:
    const int THUMBNAIL_SIZE = 128;

    SLATE_BEGIN_ARGS(SPartThumbnailTile) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

};

class SPartContentBrowser : public SContentBrowser {
public:
    SLATE_BEGIN_ARGS(SPartContentBrowser)
        : _AddPlusButton(false)
    {}
        SLATE_ARGUMENT(bool, AddPlusButton)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    void ScanPluginDirectory();

};
