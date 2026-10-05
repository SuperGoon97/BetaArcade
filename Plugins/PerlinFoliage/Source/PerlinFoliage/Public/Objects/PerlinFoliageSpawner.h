#pragma once

#include "CoreMinimal.h"
#include "Structs/FFoliageDataTable.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "BPFL/Utility/CamiArea_Exclusion.h"
#include "PerlinFoliageSpawner.generated.h"

using HISMC = UHierarchicalInstancedStaticMeshComponent;

UCLASS()
class APerlinFoliageSpawner : public AActor {
    GENERATED_BODY()

public:
    const bool USE_WORLD_SPACE = true;

#pragma region User-facing Properties

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter")
    UFoliageConfigData* FoliageConfig;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Noise", meta = (ClampMin = "0.00025", ClampMax = "1.0"))
    float NoiseScale = 0.0005f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Noise")
    int Seed = 0;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Noise", meta = (ClampMin = "100"))
    float ScatterRadius = 2000.0f;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scatter|Noise")
    int TotalSamplePointsCoefficient = 800;

#pragma endregion

private:

#pragma region Transient Properties

    int m_SpawnedInstances = 0;
    TArray<F_Area_Exclusion> m_ExclusionAreas;

#pragma endregion

#pragma region Persistent Properties

    UPROPERTY()
    TArray<UHierarchicalInstancedStaticMeshComponent*> InstancedMeshes;
    UPROPERTY()
    bool m_bIsBaked = false;

#pragma endregion

public:

#pragma region Scatter

    UFUNCTION(BlueprintCallable, Category = "Foliage|Scatter")
    void RegisterExclusionAreas(const TArray<F_Area_Exclusion>& ExclusionAreas);
    UFUNCTION(BlueprintCallable, Category = "Foliage|Scatter")
    void DoScatter();

#pragma endregion

#pragma region Engine Actions

    UFUNCTION(CallInEditor, Category = "Foliage")
    void Regenerate();
    UFUNCTION(CallInEditor, Category = "Foliage")
    void BakeFoliage();
    UFUNCTION(CallInEditor, Category = "Foliage")
    void UnbakeAndClear();

#pragma endregion

    UFUNCTION(BlueprintPure, Category = "Foliage|Getters")
    bool GetIsBaked() { return m_bIsBaked; }

private:

#pragma region Construction

    APerlinFoliageSpawner();
    void InitializeInstancedMeshes();
    void OnConstruction(const FTransform& Transform);

#pragma endregion

protected:

#pragma region Editor Utils

#if WITH_EDITOR
    TObjectPtr<UFoliageConfigData> CachedConfig = nullptr;

    virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

#pragma endregion
};
