#pragma once

#include "CoreMinimal.h"
#include "Tooltip.generated.h"

USTRUCT(BlueprintType)
struct FTooltip {
GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip Data")
    FName TooltipCategory;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tooltip Data")
    FString TooltipDescription;

    FTooltip()
        : TooltipCategory(TEXT("N/A")),
          TooltipDescription((TEXT("N/A")))
    {}
};
