#include "Objects/PerlinFoliageSpawner.h"
#include "Structs/FFoliageData.h"
#include "BPFL/CamiLibAreaBPFL.h"
#include "Kismet/KismetMathLibrary.h"

#pragma region Construction

APerlinFoliageSpawner::APerlinFoliageSpawner()
    : Super() {
    USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("SpawnerRootComponent"));
    SetRootComponent(Root);
    return;
}

void APerlinFoliageSpawner::OnConstruction(const FTransform& Transform) {
    Super::OnConstruction(Transform);
    if (m_bIsBaked) {
        UE_LOG(LogTemp, Log, TEXT("[Perlin Foliage Spawner] Did not generate on construction, bake was successful."));
        return;
    }

#if WITH_EDITOR
    if (FoliageConfig && CachedConfig != FoliageConfig) {
        FoliageConfig->OnConfigChanged.RemoveAll(this);
        FoliageConfig->OnConfigChanged.AddUObject(this, &APerlinFoliageSpawner::Regenerate);
        CachedConfig = FoliageConfig;
    }
#endif

    UE_LOG(LogTemp, Log, TEXT("[Perlin Foliage Spawner] Performing scatter on construction."));
    DoScatter();
}

#pragma endregion

#pragma region Engine Actions

void APerlinFoliageSpawner::BakeFoliage() {
    if (m_bIsBaked) return;

#if WITH_EDITOR
    DoScatter();

    for (HISMC* Component : InstancedMeshes) {
        if (Component) {
            Component->ClearFlags(RF_Transient);
            Component->CreationMethod = EComponentCreationMethod::Instance;
        }
    }

    m_bIsBaked = true;

    this->Modify();
    if (GetWorld()) {
        GetWorld()->MarkPackageDirty();
    }

    UE_LOG(LogTemp, Log, TEXT("[Perlin Foliage Spawner] Successfully baked foliage"));
#endif
}

void APerlinFoliageSpawner::UnbakeAndClear() {
    if (!m_bIsBaked ) return;

#if WITH_EDITOR
    for (HISMC* Component : InstancedMeshes) {
        if (Component) Component->DestroyComponent();
    }
    InstancedMeshes.Empty();

    m_bIsBaked = false;

    UE_LOG(LogTemp, Log, TEXT("[Perlin Foliage Spawner] Foliage unbaked."));
    this->RerunConstructionScripts();

    this->Modify();
    if (GetWorld()) {
        GetWorld()->MarkPackageDirty();
    }
#endif
}

void APerlinFoliageSpawner::Regenerate() {
    if (m_bIsBaked) return;
#if WITH_EDITOR
    this->RerunConstructionScripts();
#endif
}

#pragma endregion

#pragma region Scatter

void APerlinFoliageSpawner::RegisterExclusionAreas(const TArray<F_Area_Exclusion>& ExclusionAreas) {
    m_ExclusionAreas = ExclusionAreas;
    return;
}

void APerlinFoliageSpawner::InitializeInstancedMeshes() {
    for (HISMC* Component : InstancedMeshes) {
        if (Component) {
            Component->DestroyComponent();
        }
    }
    InstancedMeshes.Empty();

    if (!FoliageConfig) return;

    for (const FFoliageData& FoliageData : FoliageConfig->FoliageRows) {
        HISMC* NewHISMC = NewObject<HISMC>(this);

        if (FoliageData.InstancedStaticMesh) {
            NewHISMC->SetStaticMesh(FoliageData.InstancedStaticMesh);
        }
        // Instance generation Optimizations.
        // This is preferred to be false by default.
        if (!FoliageData.bDoCollision) {
            NewHISMC->SetCollisionEnabled(ECollisionEnabled::NoCollision);
            NewHISMC->SetCollisionProfileName(TEXT("NoCollision"));
        }

        NewHISMC->SetCastShadow(FoliageData.bCastShadow);
        NewHISMC->SetCanEverAffectNavigation(FoliageData.bAffectNavigation);
        NewHISMC->SetCullDistances(FoliageData.CullDistanceMinimum, FoliageData.CullDistanceMaximum);

        NewHISMC->RegisterComponent();
        NewHISMC->AttachToComponent(GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

        InstancedMeshes.Add(NewHISMC);
    }
    return;
}

void APerlinFoliageSpawner::DoScatter() {
    InitializeInstancedMeshes();

    if (!FoliageConfig) return;

    TArray<FFoliageData>& FoliagePool = FoliageConfig->FoliageRows;
    if (FoliagePool.Num() == 0) return;

    FVector SpawnerLocation = GetActorLocation();
    FRandomStream Stream(Seed);
    m_SpawnedInstances = 0;

    int32 TotalSamplePoints = FMath::RoundToInt((ScatterRadius / 1000.0f) * TotalSamplePointsCoefficient);
    TotalSamplePoints = FMath::Clamp(TotalSamplePoints, 100, 150000);

    auto TrySpawnInstance = [&](HISMC* TargetHISMC, FVector BaseLocation, const FFoliageData& Data)
    {
        if (!TargetHISMC) return;

        float JitterX = Stream.FRandRange(-Data.JitterAmount, Data.JitterAmount);
        float JitterY = Stream.FRandRange(-Data.JitterAmount, Data.JitterAmount);

        FVector SpawnWorldPos = BaseLocation + FVector(JitterX, JitterY, 0.0f);
        FVector SurfaceNormal = FVector::UpVector;

        if (Data.bSnapToLandscapeSlope && GetWorld()) {
            FVector TraceStart = FVector(SpawnWorldPos.X, SpawnWorldPos.Y, SpawnerLocation.Z + Data.TraceHeightOffset);
            FVector TraceEnd = TraceStart - FVector(0.0f, 0.0f, Data.TraceHeightOffset * 2.0f);

            FHitResult HitResult;
            FCollisionQueryParams TraceParams(FName(TEXT("FoliageSnapTrace")), true);
            TraceParams.AddIgnoredActor(this);

            bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_WorldStatic, TraceParams);

            if (bHit && HitResult.IsValidBlockingHit()) {
                FVector HitNormal = HitResult.ImpactNormal;

                float DotValue = FVector::DotProduct(HitNormal, FVector::UpVector);
                float CurrentSlopeAngle = FMath::RadiansToDegrees(FMath::Acos(DotValue));

                if (CurrentSlopeAngle < Data.AngleMinimum || CurrentSlopeAngle > Data.AngleMaximum) {
                    return;
                }

                SpawnWorldPos = HitResult.ImpactPoint;
                SurfaceNormal = HitNormal;
            } else {
                // We didn't successfully hit anything, and therefore we don't want to spawn.
                return;
            }
        } else {
            SpawnWorldPos.Z = SpawnerLocation.Z;
        }

        FRotator RandomYaw(0.0f, Stream.FRandRange(0.0f, 360.0f), 0.0f);
        FRotator SlopeRotation = UKismetMathLibrary::MakeRotFromZ(SurfaceNormal);
        FRotator FinalRotation = UKismetMathLibrary::ComposeRotators(RandomYaw, SlopeRotation);
        double RandomScaleMultiplier = Stream.FRandRange(Data.ScaleMin, Data.ScaleMax);
        FVector RandomScale(RandomScaleMultiplier, RandomScaleMultiplier, RandomScaleMultiplier);

        FTransform WorldInstanceTransform(FinalRotation, SpawnWorldPos, RandomScale);

        for (F_Area_Exclusion& Exclusion : m_ExclusionAreas) {
            if (UCamiLibAreaBPLibrary::IsPointWithinRadius(SpawnWorldPos, Exclusion.Position, Exclusion.Radius)) return;
        }

        TargetHISMC->AddInstance(WorldInstanceTransform, USE_WORLD_SPACE);
        m_SpawnedInstances++;
    };

    for (int32 i = 0; i < TotalSamplePoints; i++) {
        // Pick a random angle pointing outwards from the center of the radius
        float RandomAngle = Stream.FRandRange(0.0f, TWO_PI);

        // This adds some extra randomness to our calculations, spreading out the foliage more.
        // It's not entirely uniform, but it's much better as it evens out the spread from the center to the edges.
        float UniformAlpha = FMath::Sqrt(Stream.FRand());
        float RandomRadius = UniformAlpha * ScatterRadius;

        // Translations from Polar Coordinates into Cartesian Coordinates.
        // Horizontal (X-Axis)
        float LocalX = FMath::Cos(RandomAngle) * RandomRadius;
        // Vertical (Y-Axis)
        float LocalY = FMath::Sin(RandomAngle) * RandomRadius;

        FVector SampleWorldPos = SpawnerLocation + FVector(LocalX, LocalY, 0.0f);

        // Get the coordinates of our world position on the random perlin noise we've generated.
        FVector2D NoiseCoords = FVector2D(SampleWorldPos.X, SampleWorldPos.Y) * NoiseScale;

        // Alter the coordinates based on the seed. This means that changing the seed changes the "position" of the object on the noise map.
        // The larger the number, the greater the difference.
        NoiseCoords.X += (Seed * 1.357f);
        NoiseCoords.Y += (Seed * 2.468f);

        // We extract the noise value from the position (This is a number from 0.0 - 1.0)
        float NoiseVal = FMath::PerlinNoise2D(NoiseCoords);
        float NormalizedNoise = (NoiseVal + 1.0f) * 0.5f;

        // Then we loop over each and every piece of foliage within the foliage map to validate whether it should spawn in that position.
        for (int32 LayerIdx = 0; LayerIdx < FoliagePool.Num(); LayerIdx++) {
            if (!InstancedMeshes.IsValidIndex(LayerIdx) || !InstancedMeshes[LayerIdx]) continue;

            const FFoliageData& DataRow = FoliagePool[LayerIdx];
            HISMC* TargetHISMC = InstancedMeshes[LayerIdx];

            if (!DataRow.InstancedStaticMesh) continue;

            if (NormalizedNoise > DataRow.SpawnThreshold) {
                int32 RandomRoll = Stream.RandRange(1, 100);
                if (RandomRoll <= DataRow.Density) {
                    TrySpawnInstance(TargetHISMC, SampleWorldPos, DataRow);
                }

                if (DataRow.bDoCluster && NormalizedNoise > (DataRow.SpawnThreshold + 0.05f)) {
                    int32 ClusterCount = Stream.RandRange(DataRow.ClusterMin, DataRow.ClusterMax);
                    for (int32 c = 0; c < ClusterCount; c++) {

                        RandomRoll = Stream.RandRange(1, 100);
                        if (RandomRoll > DataRow.ClusterDensity) continue;

                        float ClusterRadiusOffset = Stream.FRandRange(10.0f, DataRow.JitterAmount * 1.2f);
                        float ClusterAngle = Stream.FRandRange(0.0f, TWO_PI);
                        FVector2D RadialOffset(FMath::Cos(ClusterAngle) * ClusterRadiusOffset, FMath::Sin(ClusterAngle) * ClusterRadiusOffset);
                        FVector ClusterPos = SampleWorldPos + FVector(RadialOffset.X, RadialOffset.Y, 0.0f);

                        TrySpawnInstance(TargetHISMC, ClusterPos, DataRow);
                    }
                }
            }
        }
    }

    UE_LOG(LogTemp, Warning, TEXT("[DoScatter] Random samples done: %d\n[DoScatter] Total spawned: %d"), TotalSamplePoints, m_SpawnedInstances);
    return;
}

#pragma endregion

#pragma region Editor Utils
#if WITH_EDITOR
void APerlinFoliageSpawner::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
    Super::PostEditChangeProperty(PropertyChangedEvent);
    const FName PropertyName = (PropertyChangedEvent.Property != nullptr) ? PropertyChangedEvent.Property->GetFName() : NAME_None;

    if (PropertyName == GET_MEMBER_NAME_CHECKED(APerlinFoliageSpawner, FoliageConfig)) {
        if (CachedConfig) {
            CachedConfig->OnConfigChanged.RemoveAll(this);
        }

        if (FoliageConfig && !m_bIsBaked) {
            FoliageConfig->OnConfigChanged.AddUObject(this, &APerlinFoliageSpawner::Regenerate);
            CachedConfig = FoliageConfig;
        }
    }
}

#endif
#pragma endregion
