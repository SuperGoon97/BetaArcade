#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Layout/SWrapBox.h"
#include "AssetRegistry/AssetData.h"

typedef TSharedPtr<FAssetData> FPartAssetEntryPtr;

class SPartThumbnailTitle : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPartThumbnailTitle) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SPartEditorViewport> InViewportWidget);
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;

private:
    FAssetData m_AssetDataRecord;
    TWeakPtr<class SPartEditorViewport> m_AssociatedViewport;

    TSharedPtr<FAssetThumbnail> m_AssetThumbnailInstance;
};

class SPartContentBrowser : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SPartContentBrowser) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SPartEditorViewport> InViewportWidget);
    void ScanPluginDirectory();

    static TWeakPtr<SPartContentBrowser> m_Instance;
private:
    TSharedRef<SWidget> OnGenerateTileWidget(FPartAssetEntryPtr Item);

    TWeakPtr<class SPartEditorViewport> m_AssociatedViewport;
    TSharedPtr<SWrapBox> m_TileGridContainer;
};
