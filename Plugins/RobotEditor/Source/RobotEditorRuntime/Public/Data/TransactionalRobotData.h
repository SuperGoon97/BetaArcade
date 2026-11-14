#pragma once

#include "CoreMinimal.h"
#include "Structs/RobotData.h"
#include "TransactionalRobotData.generated.h"

UCLASS()
class URobotEditorUndoState : public UObject {
GENERATED_BODY()
public:
    UPROPERTY()
    TArray<FPartData> PartSnapshot;
};
