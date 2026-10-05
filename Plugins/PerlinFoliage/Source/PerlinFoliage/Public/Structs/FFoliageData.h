#pragma once

#include "CoreMinimal.h"
#include "FFoliageData.generated.h"

USTRUCT(BlueprintType)
struct FFoliageData : public FTableRowBase {
    GENERATED_BODY()

public:
    FFoliageData()
        : InstancedStaticMesh(nullptr)
    {}

    UPROPERTY(EditAnywhere, Category = "Foliage|Static Mesh")
    UStaticMesh* InstancedStaticMesh = nullptr;

    UPROPERTY(EditAnywhere, Category = "Foliage|Density")
    float SpawnThreshold = 0.5f;
    UPROPERTY(EditAnywhere, Category = "Foliage|Density", meta = (ClampMin = "0", ClampMax = "100"))
    int Density = 50;
    UPROPERTY(EditAnywhere, Category = "Foliage|Density", meta = (ClampMin = "0.1"))
    float JitterAmount = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Foliage|Scale", meta = (ClampMin = "0.0"))
    float ScaleMin = 0.8f;
    UPROPERTY(EditAnywhere, Category = "Foliage|Scale")
    float ScaleMax = 1.2f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Snapping")
    bool bSnapToLandscapeSlope = false;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Snapping", meta = (ClampMin = "0"))
    float TraceHeightOffset = 1000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Snapping", meta = (ClampMin = "0", ClampMax = "90"))
    float AngleMinimum = 0.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Foliage|Snapping", meta = (ClampMin = "0", ClampMax = "90"))
    float AngleMaximum = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Foliage|Cluster")
    bool bDoCluster = false;
    UPROPERTY(EditAnywhere, Category = "Foliage|Cluster", meta = (ClampMin = "1", ClampMax = "100"))
    int ClusterDensity = 50;
    UPROPERTY(EditAnywhere, Category = "Foliage|Cluster", meta = (ClampMin = "1"))
    int ClusterMin = 1;
    UPROPERTY(EditAnywhere, Category = "Foliage|Cluster", meta = (ClampMin = "1"))
    int ClusterMax = 2;

    UPROPERTY(EditAnywhere, Category = "Optimization|Culling", meta = (ClampMin = "0"))
    float CullDistanceMinimum = 1000.0f;
    UPROPERTY(EditAnywhere, Category = "Optimization|Culling", meta = (ClampMin = "0"))
    float CullDistanceMaximum = 50000.0f;
    UPROPERTY(EditAnywhere, Category = "Optimization")
    bool bAffectNavigation = false;
    UPROPERTY(EditAnywhere, Category = "Optimization")
    bool bDoCollision = false;
    UPROPERTY(EditAnywhere, Category = "Optimization")
    bool bCastShadow = true;

};
