#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Structs/RobotData.h"
#include "RobotSaveGame.generated.h"

UCLASS()
class ROBOTEDITORRUNTIME_API URobotSaveGame : public USaveGame {
GENERATED_BODY()
public:

    UPROPERTY(Instanced, EditAnywhere, BlueprintReadWrite, Category = "Save")
    URobotDataContainer* SaveData;

};
