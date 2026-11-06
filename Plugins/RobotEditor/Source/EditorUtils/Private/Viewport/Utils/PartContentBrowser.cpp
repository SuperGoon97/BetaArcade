#include "Viewport/Utils/PartContentBrowser.h"
#include "Viewport/EditorViewportClient.h"
#include "Viewport/EditorViewport.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "AssetThumbnail.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Data/PartDat.h"

TWeakPtr<SContentBrowser> SContentBrowser::m_Instance = nullptr;

void SThumbnailTile::Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget) {
    m_AssetDataRecord = InAssetData;
    m_AssociatedViewport = InViewportWidget;

    TSharedPtr<FEditorUtilsViewportClient> ViewportClient = m_AssociatedViewport.Pin()->GetViewportClient();
    if (ViewportClient.IsValid()) {
        ViewportClient->ConnectTileSingleClick(this);
        ViewportClient->ConnectTileDoubleClick(this);
    }

    FString AssetNameStr = m_AssetDataRecord.AssetName.ToString();

    if (!m_AssetDataRecord.IsValid()) {
        ChildSlot [ SNew(STextBlock).Text(FText::FromString(AssetNameStr)) ];
        return;
    }

    TSharedPtr<FAssetThumbnailPool> GlobalPool = UThumbnailManager::Get().GetSharedThumbnailPool();
    TSharedRef<SWidget> ThumbnailWidget = SNew(STextBlock).Text(INVTEXT("Data"));

    UObject* Outer = m_AssetDataRecord.GetAsset();
    UPDAPart* OuterCast = Cast<UPDAPart>(Outer);

    if (!OuterCast) return;

    UObject* Inner = OuterCast->Object;

    if (Inner) {
        m_AssetThumbnailInstance = MakeShareable(new FAssetThumbnail(Inner, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GlobalPool));
    } else {
        m_AssetThumbnailInstance = MakeShareable(new FAssetThumbnail(OuterCast, THUMBNAIL_SIZE, THUMBNAIL_SIZE, GlobalPool));
    }

    if (m_AssetThumbnailInstance.IsValid()) {
        TSharedPtr<SWidget> NativeWidget = m_AssetThumbnailInstance->MakeThumbnailWidget();
        if (NativeWidget.IsValid()) {
            ThumbnailWidget = NativeWidget.ToSharedRef();
        }
    }

    ChildSlot
    [
        SNew(SBox)
        .WidthOverride(96.0f)
        .HeightOverride(96.0f)
        [
            SNew(SBorder)
            .BorderImage(FAppStyle::Get().GetBrush("Brushes.Panel"))
            .Padding(FMargin(4.0f))
            .HAlign(HAlign_Center)
            .VAlign(VAlign_Center)
            [
                SNew(SVerticalBox)

                + SVerticalBox::Slot()
                .FillHeight(1.0f)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                [
                    SNew(SBox)
                    .WidthOverride(64.0f)
                    .HeightOverride(64.0f)
                    [
                        ThumbnailWidget
                    ]
                ]
                + SVerticalBox::Slot()
                .AutoHeight()
                .HAlign(HAlign_Center)
                .Padding(FMargin(2.0f, 0.0f, 2.0f, 4.0f))
                [
                    SNew(STextBlock)
                    .Text(FText::FromString(AssetNameStr))
                    .Font(FAppStyle::Get().GetFontStyle("NormalFontMicro"))
                ]
            ]
        ]
    ];
}



void SContentBrowser::Construct(const FArguments& InArgs, TSharedPtr<class SEditorUtilsViewport> InViewportWidget) {
    m_AssociatedViewport = InViewportWidget;
    m_Instance = SharedThis(this);
    m_AddPlusButton = InArgs._AddPlusButton;

    TSharedPtr<FEditorUtilsViewportClient> ViewportClient = m_AssociatedViewport.Pin()->GetViewportClient();
    if (ViewportClient.IsValid()) {
        OnAddButtonPressed.AddSP(ViewportClient.Get(), &FEditorUtilsViewportClient::OnAddButtonPressed);
    }

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(FMargin(6.0f, 4.0f))
        [
            SNew(STextBlock)
            .Text(INVTEXT("Parts Asset Registry"))
            .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
        ]

        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        .Padding(FMargin(4.0f))
        .HAlign(HAlign_Fill)
        [
            SNew(SScrollBox)
            .Orientation(Orient_Vertical)
            + SScrollBox::Slot()
            [
                SAssignNew(m_TileGridContainer, SWrapBox)
                .UseAllottedSize(true)
            ]
        ]
    ];
    ScanPluginDirectory();
}

FReply SThumbnailTile::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
        OnThumbnailDoubleClicked.Broadcast(GetRelatedPart());
        return FReply::Handled();
    }
    return FReply::Unhandled();
}

FReply SThumbnailTile::OnMouseButtonDown(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    m_MouseButtonHeld = true;
    return FReply::Handled();
}

FReply SThumbnailTile::OnMouseButtonUp(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    if (m_MouseButtonHeld) {
        m_MouseButtonHeld = false;

        if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton) {
            OnThumbnailClicked.Broadcast(GetRelatedPart());
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

UPDAPart* SThumbnailTile::GetRelatedPart() {
    UPDAPart* ReturnPart = nullptr;
    if (m_AssetDataRecord.IsValid()) {
        UObject* FullyLoadedAsset = m_AssetDataRecord.GetAsset();
        ReturnPart = Cast<UPDAPart>(FullyLoadedAsset);
    }
    return ReturnPart;
}


void SContentBrowser::ScanPluginDirectory() {
    if (!m_TileGridContainer.IsValid()) return;
    m_TileGridContainer->ClearChildren();

    if (m_AddPlusButton) {
        TWeakPtr<SContentBrowser> WeakSelf = SharedThis(this);
        m_TileGridContainer->AddSlot()
            .Padding(FMargin(4.0f))
            [
                SNew(SBox)
                .WidthOverride(96.0f)
                .HeightOverride(96.0f)
                [
                    SNew(SButton)
                    .ButtonStyle(FAppStyle::Get(), "SimpleButton")
                    .HAlign(HAlign_Center)
                    .VAlign(VAlign_Center)
                    .OnClicked_Lambda([WeakSelf]() -> FReply {
                        WeakSelf.Pin()->OnAddButtonPressed.Broadcast();
                        return FReply::Handled();
                    })
                    [
                        SNew(STextBlock)
                        .Text(INVTEXT("+"))
                        .Font(FAppStyle::Get().GetFontStyle("HeadingExtraLarge"))
                    ]
                ]
            ];
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    FARFilter DataFolderFilter;
    DataFolderFilter.PackagePaths.Add(TEXT("/RobotEditor/Data"));
    DataFolderFilter.ClassPaths.Add(UPDAPart::StaticClass()->GetClassPathName());
    DataFolderFilter.bRecursivePaths = true;

    TArray<FAssetData> ExtractedAssetRecords;
    AssetRegistryModule.Get().GetAssets(DataFolderFilter, ExtractedAssetRecords);

    for (const FAssetData& AssetRecord : ExtractedAssetRecords) {
        TSharedRef<SThumbnailTile> GeneratedTileCard = SNew(SThumbnailTile, AssetRecord, m_AssociatedViewport.Pin());
        m_TileGridContainer->AddSlot()
            .Padding(FMargin(4.0f))
            [
                GeneratedTileCard
            ];
    }
}
