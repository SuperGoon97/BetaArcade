#pragma once

#include "CoreMinimal.h"

class SRobotEditorTab : public SDockTab {
public:
    static TSharedRef<class SDockTab> MakeTab(const FSpawnTabArgs& SpawnTabArgs);
};
