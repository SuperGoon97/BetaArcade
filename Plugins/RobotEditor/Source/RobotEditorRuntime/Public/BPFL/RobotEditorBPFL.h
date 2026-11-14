#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Data/PartDat.h"
#include "Structs/SnapResult.h"
#include "RobotEditorBPFL.generated.h"

UCLASS()
class ROBOTEDITORRUNTIME_API URobotEditorBPFL : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category = "Robot Building")
    static FSnapResult GetSnapResult(const FVector& HitImpactPoint, AActor* HitActor, UPDAPart* HitPart, UPDAPart* PartToSnap, int CurrentRotationIndex = 0);
    UFUNCTION()
    static bool IsPartColliding(UWorld* World, const FTransform& Transform, UPDAPart* Part);

public:

    static inline float m_ShapeCastMarginMultiplier = 0.95f;
};
