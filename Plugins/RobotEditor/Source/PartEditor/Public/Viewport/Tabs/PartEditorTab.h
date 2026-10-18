#pragma once

#include "CoreMinimal.h"

const static float AddPartButtonHeight = 60.0f;
const static float AddAnchorButtonHeight = 60.0f;

class SPartEditorTab : public SDockTab {
public:

    static TSharedRef<class SDockTab> MakeTab(const FSpawnTabArgs& SpawnTabArgs);

};
