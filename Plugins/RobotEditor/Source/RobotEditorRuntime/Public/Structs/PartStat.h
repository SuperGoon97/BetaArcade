#pragma once

#include "CoreMinimal.h"
#include "Enums/StatDataType.h"
#include "PartStat.generated.h"

USTRUCT(BlueprintType)
struct FPartStat {
GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Category")
    EDataType StatDataType;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    float Default;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    float Min;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
    float Max;
};
