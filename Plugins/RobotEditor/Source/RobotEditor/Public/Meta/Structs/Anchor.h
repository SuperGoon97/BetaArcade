#pragma once

#include "CoreMinimal.h"
#include "Anchor.generated.h"

USTRUCT(BlueprintType)
struct FAnchor {
GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Data")
    FString AnchorName = TEXT("New Anchor");
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Data")
    FVector Position;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anchor Data")
    bool bIsConnected;

    FAnchor()
        : AnchorName("New Anchor"),
          Position(FVector::ZeroVector),
          bIsConnected(false)
    {}
};
