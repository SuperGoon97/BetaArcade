#include "ThumbnailRendering/ThumbnailManager.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "AssetToolsModule.h"
#include "AssetThumbnail.h"
#include "Viewport/Utils/ContentPanel.h"
#include "Viewport/PartEditorViewport.h"
#include "Viewport/PartEditorViewportClient.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SWrapBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Viewport/EditorViewportClient.h"
#include "Data/PartDat.h"

void SPartThumbnailTile::Construct(const FArguments& InArgs, const FAssetData& InAssetData, TSharedPtr<class SEditorUtilsViewport> InViewportWidget) {
    SThumbnailTile::FArguments ParentArgs;
    SThumbnailTile::Construct(ParentArgs, InAssetData, InViewportWidget);
}

void SPartContentBrowser::Construct(const FArguments& InArgs, TSharedPtr<SEditorUtilsViewport> InViewportWidget) {
    SContentBrowser::FArguments Args;
    SContentBrowser::Construct(Args, InViewportWidget);
}


void SPartContentBrowser::ScanPluginDirectory() {
    SContentBrowser::ScanPluginDirectory();
}
