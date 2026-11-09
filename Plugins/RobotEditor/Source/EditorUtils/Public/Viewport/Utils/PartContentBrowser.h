#pragma once

#if WITH_EDITOR

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Layout/SWrapBox.h"
#include "AssetRegistry/AssetData.h"

class UPDAPart;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnThumbnailClicked, UPDAPart*);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnThumbnailDoubleClicked, UPDAPart*);

typedef TSharedPtr<FAssetData> FPartAssetEntryPtr;

class EDITORUTILS_API SThumbnailTile : public SCompoundWidget {
public:
    const int THUMBNAIL_SIZE = 128;

    SLATE_BEGIN_ARGS(SThumbnailTile) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    virtual FReply OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;
    virtual FReply OnMouseButtonUp(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent) override;

    virtual UPDAPart* GetRelatedPart();
    virtual void OnThumbnailSelected(SThumbnailTile* Thumbnail);

    FOnThumbnailClicked OnThumbnailClicked;
    FOnThumbnailDoubleClicked OnThumbnailDoubleClicked;

protected:

    FAssetData m_AssetDataRecord;
    TWeakPtr<class SEditorUtilsViewport> m_AssociatedViewport;
    TSharedPtr<FAssetThumbnail> m_AssetThumbnailInstance;
    TSharedPtr<SBorder> m_BackgroundBorder;

private:

    bool m_MouseButtonHeld = false;

};

DECLARE_MULTICAST_DELEGATE(FOnAddButtonPressed);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnThumbnailSelected, SThumbnailTile*);

class EDITORUTILS_API SContentBrowser : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SContentBrowser)
        : _AddPlusButton(false)
    {}
        SLATE_ARGUMENT(bool, AddPlusButton)
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SEditorUtilsViewport> InViewportWidget);
    void ScanPluginDirectory();

    static TWeakPtr<SContentBrowser> m_Instance;

    FOnAddButtonPressed OnAddButtonPressed;
    FOnThumbnailSelected OnThumbnailSelected;

protected:

    TSharedRef<SWidget> OnGenerateTileWidget(FPartAssetEntryPtr Item);
    TWeakPtr<class SEditorUtilsViewport> m_AssociatedViewport;
    TSharedPtr<SWrapBox> m_TileGridContainer;

private:

    bool m_AddPlusButton = false;
};

#endif
