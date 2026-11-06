#pragma once

#include "CoreMinimal.h"

UENUM(BlueprintType)
 enum class EDataType : uint8 {
    Bool    UMETA(DisplayName = "Boolean"),
    Float   UMETA(DisplayName = "Floating Point"),
    Int     UMETA(DisplayName = "Integer"),
};
