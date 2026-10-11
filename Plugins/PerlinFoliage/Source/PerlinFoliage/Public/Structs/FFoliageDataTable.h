#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "FFoliageData.h"
#include "FFoliageDataTable.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnFoliageConfigChangedSignature);

UCLASS()
class PERLINFOLIAGE_API UFoliageConfigData : public UPrimaryDataAsset {
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, Category = "Config")
    TArray<FFoliageData> FoliageRows;

    FOnFoliageConfigChangedSignature OnConfigChanged;

private:
#if WITH_EDITOR
    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
