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
};

UCLASS(BlueprintType, Blueprintable)
class ROBOTEDITORRUNTIME_API UPartDataContainer : public UObject {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
    FPartData PartData;
};

UCLASS(BlueprintType, Blueprintable)
class ROBOTEDITORRUNTIME_API URobotDataContainer : public UObject {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, Instanced, EditAnywhere, Category = "Robot Data")
    TArray<UPartDataContainer*> RobotParts;
};
