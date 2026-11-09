#include "Viewport/Tabs/RobotEditorTab.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Viewport/RobotEditorViewport.h"

TSharedRef<SDockTab> SRobotEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {
    TSharedRef<SRobotEditorViewport> ViewportWidget = SNew(SRobotEditorViewport);

    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SSplitter)
            .Orientation(Orient_Vertical)

            + SSplitter::Slot()
            .Value(1.0f)
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
                    SNew(SContentBrowser, ViewportWidget)
                ]
            ]
        ];

    // return SNew(SDockTab)
    //     .TabRole(ETabRole::NomadTab)
    //     [
    //         SNew(SRobotEditorViewport)
    //     ];
}
