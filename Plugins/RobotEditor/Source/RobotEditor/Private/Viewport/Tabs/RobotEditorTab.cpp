#include "Viewport/Tabs/RobotEditorTab.h"
#include "Viewport/RobotEditorViewport.h"

TSharedRef<SDockTab> SRobotEditorTab::MakeTab(const FSpawnTabArgs& SpawnTabArgs) {
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SRobotEditorViewport)
        ];
}
