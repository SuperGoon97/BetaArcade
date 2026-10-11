#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "CamiLibEngineBPFL.generated.h"

UCLASS()
class CAMILIB_API UCamiLibEngineBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintPure, Category = "Utilities|Shader Compilation")
    static int32 GetNumPrecompilingPSOsRemaining();
    UFUNCTION(BlueprintPure, Category = "UI|Formatting")
    static FText FormatLargeCurrency(int64 Amount, int32 Decimals, bool bUseCSV);
    UFUNCTION(BlueprintPure, Category = "Utility|Big Numbers")
    static int64 GetRandomLargeInteger(int64 Min, int64 Max);
    UFUNCTION(BlueprintPure, Category = "Utility|Big Numbers")
    static double DivideInt64(int64 a, int64 b);

private:
    static FString InsertCommas(int64 AbsAmount);
};
