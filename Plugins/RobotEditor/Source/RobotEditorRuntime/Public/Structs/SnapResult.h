#pragma once

#include "CoreMinimal.h"
#include "SnapResult.generated.h"

USTRUCT(BlueprintType)
struct FSnapResult {
GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadOnly)
    bool bCanSnap;
    UPROPERTY(BlueprintReadOnly)
    FTransform SnapTransform;
    UPROPERTY(BlueprintReadOnly)
    TObjectPtr<AActor> HitActor;
    UPROPERTY(BlueprintReadOnly)
    int HitActorAnchorIndex;
    UPROPERTY(BlueprintReadOnly)
    int SnapActorAnchorIndex;

    FSnapResult(bool _bCanSnap = false, FTransform _SnapTransform = FTransform::Identity, TObjectPtr<AActor> _HitActor = nullptr, int _HitActorAnchorIndex = INDEX_NONE, int _SnapActorAnchorIndex = INDEX_NONE)
        : bCanSnap(_bCanSnap)
        , SnapTransform(_SnapTransform)
        , HitActor(_HitActor)
        , HitActorAnchorIndex(_HitActorAnchorIndex)
        , SnapActorAnchorIndex(_SnapActorAnchorIndex)
    {}
};
