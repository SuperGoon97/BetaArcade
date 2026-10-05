#pragma once

#include "CoreMinimal.h"
#include "CamiArea_Exclusion.Generated.h"

USTRUCT(BlueprintType)
struct CAMILIB_API F_Area_Exclusion {
    GENERATED_BODY()
public:

    UPROPERTY(BlueprintReadWrite)
    FVector Position;
    UPROPERTY(BlueprintReadWrite)
    float Radius;
};
