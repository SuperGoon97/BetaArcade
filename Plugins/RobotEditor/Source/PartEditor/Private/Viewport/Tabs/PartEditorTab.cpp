#include "Viewport/PartEditorViewportClient.h"
#include "Viewport/Utils/AnchorsPanel.h"
#include "Viewport/Utils/ContentPanel.h"
#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"
#include "Widgets/Layout/SSplitter.h"
#include "Widgets/Input/SButton.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "Data/PartDat.h"
#include "IDetailsView.h"

TSharedRef<SDockTab> SPartEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = false;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bShowOptions = false;
    DetailsViewArgs.bAllowFavoriteSystem = false;

    TSharedRef<IDetailsView> PartsDetailsView = PropertyEditorModule.CreateDetailView(DetailsViewArgs);
    TSharedRef<SPartEditorViewport> ViewportWidget = SNew(SPartEditorViewport);
    TSharedPtr<FPartEditorViewportClient> ClientPtr = ViewportWidget->GetPartClient();
    if (ClientPtr.IsValid()) {
        ClientPtr->SetDetailsView(PartsDetailsView);
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SSplitter)
            .Orientation(Orient_Vertical)

            + SSplitter::Slot()
            .Value(0.75f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)

                + SSplitter::Slot()
                .Value(0.2f)
                [
                    SNew(SAnchorsPanel, ViewportWidget)
                ]
                + SSplitter::Slot()
                .Value(0.55f)
                [
                    ViewportWidget
                ]

                + SSplitter::Slot()
                .Value(0.25f)
                [
                    SNew(SVerticalBox)
                    + SVerticalBox::Slot()
                    .AutoHeight()
                    .Padding(6.0f)
                    .HAlign(HAlign_Center)
                    [
                        SNew(STextBlock)
                        .Text(INVTEXT("Parts Inspector"))
                        .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
                    ]
                    + SVerticalBox::Slot()
                    .FillHeight(1.0f)
                    [
                        PartsDetailsView
                    ]
                ]
            ]

            + SSplitter::Slot()
            .Value(0.25f)
            [
                SNew(SPartContentBrowser, ViewportWidget)
            ]
        ];
}
