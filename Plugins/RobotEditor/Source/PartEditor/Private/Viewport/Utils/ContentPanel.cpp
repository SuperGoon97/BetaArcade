#include "ThumbnailRendering/ThumbnailManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "AssetThumbnail.h"
#include "Viewport/Utils/ContentPanel.h"
#include "Viewport/PartEditorViewport.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Data/PartDat.h"

TWeakPtr<SPartContentBrowser> SPartContentBrowser::m_Instance = nullptr;

void SPartThumbnailTitle::Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SPartEditorViewport> InViewportWidget) {
    m_AssetDataRecord = InAssetData;
    m_AssociatedViewport = InViewportWidget;

    FString AssetNameStr = m_AssetDataRecord.AssetName.ToString();

    if (!m_AssetDataRecord.IsValid()) {
        ChildSlot [ SNew(STextBlock).Text(FText::FromString(AssetNameStr)) ];
        return;
    }

    TSharedPtr<FAssetThumbnailPool> LocalThumbnailPool = MakeShareable(new FAssetThumbnailPool(1));
    m_AssetThumbnailInstance = MakeShareable(new FAssetThumbnail(m_AssetDataRecord, 64, 64, LocalThumbnailPool));
    TSharedRef<SWidget> ThumbnailWidget = SNew(STextBlock).Text(INVTEXT("Data"));

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

FReply SPartThumbnailTitle::OnMouseButtonDoubleClick(const FGeometry& InMyGeometry, const FPointerEvent& InMouseEvent) {
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && m_AssociatedViewport.IsValid()) {
        UObject* FullyLoadedAsset = m_AssetDataRecord.GetAsset();

        if (FullyLoadedAsset) {
            m_AssociatedViewport.Pin()->SetEditingObject(FullyLoadedAsset);
            return FReply::Handled();
        }
    }
    return FReply::Unhandled();
}

void SPartContentBrowser::Construct(const FArguments& InArgs, TSharedPtr<SPartEditorViewport> InViewportWidget) {
    m_AssociatedViewport = InViewportWidget;
    m_Instance = SharedThis(this);

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
                + SWrapBox::Slot()
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
                        .OnClicked_Lambda([InViewportWidget]() -> FReply {
                            if (InViewportWidget.IsValid() && InViewportWidget->GetPartClient().IsValid()) {
                                UPDAPart* NewBlankPart = NewObject<UPDAPart>(GetTransientPackage(), UPDAPart::StaticClass());
                                if (NewBlankPart) {
                                    InViewportWidget->SetEditingObject(NewBlankPart);
                                }
                            }
                            return FReply::Handled();
                        })
                    ]
                ]
            ]
        ]
    ];
    ScanPluginDirectory();
}


void SPartContentBrowser::ScanPluginDirectory() {
    if (!m_TileGridContainer.IsValid()) return;

    m_TileGridContainer->ClearChildren();

    TWeakPtr<SPartEditorViewport> WeakViewport = m_AssociatedViewport;

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
                .OnClicked_Lambda([WeakViewport]() -> FReply {
                    if (WeakViewport.IsValid()) {
                        TSharedPtr<SPartEditorViewport> ViewportPin = WeakViewport.Pin();
                        if (ViewportPin.IsValid() && ViewportPin->GetPartClient().IsValid()) {
                            UPDAPart* NewBlankPart = NewObject<UPDAPart>(GetTransientPackage(), UPDAPart::StaticClass());
                            if (NewBlankPart) {
                                ViewportPin->SetEditingObject(NewBlankPart);
                            }
                        }
                    }
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock)
                    .Text(INVTEXT("+"))
                    .Font(FAppStyle::Get().GetFontStyle("HeadingExtraLarge"))
                ]
            ]
        ];

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");

    FARFilter DataFolderFilter;
    DataFolderFilter.PackagePaths.Add(TEXT("/RobotEditor/Data"));
    DataFolderFilter.ClassPaths.Add(UPDAPart::StaticClass()->GetClassPathName());
    DataFolderFilter.bRecursivePaths = true;

    TArray<FAssetData> ExtractedAssetRecords;
    AssetRegistryModule.Get().GetAssets(DataFolderFilter, ExtractedAssetRecords);

    for (const FAssetData& AssetRecord : ExtractedAssetRecords) {
        TSharedRef<SPartThumbnailTitle> GeneratedTileCard = SNew(SPartThumbnailTitle, AssetRecord, m_AssociatedViewport.Pin());
        m_TileGridContainer->AddSlot()
            .Padding(FMargin(4.0f))
            [
                GeneratedTileCard
            ];
    }
}
