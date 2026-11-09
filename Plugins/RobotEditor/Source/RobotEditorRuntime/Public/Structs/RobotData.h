#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/PartDat.h"
#include "RobotData.generated.h"

USTRUCT(BlueprintType)
struct FPartData {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
    TSoftObjectPtr<UPDAPart> Part;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Save Data")
    FTransform RelativeTransform;
    UPROPERTY(BlueprintReadWrite,  EditAnywhere, Category = "Part Data")
    TMap<FName, float> DataFloatMap;

    FPartData()
        : Part(nullptr)
        , RelativeTransform(FTransform::Identity)
    {}

    FPartData(TSoftObjectPtr<UPDAPart> AssetPtr, const FTransform& InTransform, const TMap<FName, float>& InModifiers)
        : Part(AssetPtr)
        , RelativeTransform(InTransform)
        , DataFloatMap(InModifiers)
    {}
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
    UPROPERTY(EditAnywhere, Category = "Internal Data")
    FName RobotName = "UNNAMED";
};
