#include "Viewport/Tabs/RobotEditorTab.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Viewport/RobotEditorViewport.h"
#include "Modules/ModuleManager.h"
#include "PropertyEditorModule.h"
#include "IStructureDetailsView.h"
#include "UObject/StructOnScope.h"

TSharedRef<SDockTab> SRobotEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {
    TSharedRef<SRobotEditorViewport> ViewportWidget = SNew(SRobotEditorViewport);


    /*
     * Instantiating a structure details view for some reason takes infinitely more steps than a default IDetailsView class. This doesn't handle classes and thus doesn't have any callbacks for
     * scoped transactions should you not supply an object outer when supplying it with a new struct to alter.
     *
     * For now, we just need it to run with a nullptr, as no object is going to be selected. I'd just like the details view to exist and and create it as a property inside the viewport client.
     */
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");

    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bAllowSearch = true;
    DetailsViewArgs.bUpdatesFromSelection = false;
    DetailsViewArgs.bShowOptions = false;
    DetailsViewArgs.bAllowFavoriteSystem = false;
    DetailsViewArgs.bUpdatesFromSelection = false;

    FStructureDetailsViewArgs StructureDetailsViewArgs;
    StructureDetailsViewArgs.bShowObjects = false;
    StructureDetailsViewArgs.bShowClasses = false;
    StructureDetailsViewArgs.bShowAssets = false;
    StructureDetailsViewArgs.bShowInterfaces = false;

    TSharedRef<IStructureDetailsView> DetailsView = PropertyEditorModule.CreateStructureDetailView(
        DetailsViewArgs,
        StructureDetailsViewArgs,
        nullptr
    );

    TSharedPtr<FRobotEditorViewportClient> ClientPtr = StaticCastSharedPtr<FRobotEditorViewportClient>(ViewportWidget->GetViewportClient());
    if (ClientPtr.IsValid()) {
        ClientPtr->SetDetailsView(DetailsView);
    }

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SSplitter)
            .Orientation(Orient_Vertical)

            + SSplitter::Slot()
            .Value(0.8f)
            [
                SNew(SSplitter)
                .Orientation(Orient_Horizontal)

                + SSplitter::Slot()
                .Value(0.75f)
                [
                    ViewportWidget
                ]

                + SSplitter::Slot()
                .Value(0.25f)
                [
                    DetailsView->GetWidget().ToSharedRef()
                ]
            ]
            + SSplitter::Slot()
            .Value(0.20f)
            [
                SNew(SContentBrowser, ViewportWidget)
            ]
        ];
}
