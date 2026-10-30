#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/PartDat.h"
#include "RobotData.generated.h"

USTRUCT(BlueprintType)
struct FPartData {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    TSoftObjectPtr<UPDAPart> Part;
    UPROPERTY(BlueprintReadWrite, Category = "Save Data")
    FTransform RelativeTransform;
    UPROPERTY(BlueprintReadWrite, Category = "Part Data")
    TMap<FName, float> DataFloatMap;
};

USTRUCT(BlueprintType)
struct FRobotData {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
    TArray<FPartData> Parts;
};

UCLASS(BlueprintType, Blueprintable)
class ROBOTEDITORRUNTIME_API URobotDataContainer : public UObject {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Robot Data")
    FRobotData RobotParts;
};
