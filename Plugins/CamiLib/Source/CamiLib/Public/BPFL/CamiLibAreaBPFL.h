#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Utility/CamiArea_Exclusion.h"
#include "CamiLibAreaBPFL.generated.h"

UCLASS()
class CAMILIB_API UCamiLibAreaBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	// Point in Area

    UFUNCTION(BlueprintCallable, Category = "Utilities|Bounding Box")
    static FVector GetRandomPointInBoundingBox(FVector BoundingBoxHalfExtent);
	UFUNCTION(BlueprintCallable, Category = "Utilities|Bounding Box")
	static FVector GetRandomPointInBoundingBoxWithExclusion(FVector BoundingBoxHalfExtent, const F_Area_Exclusion& ExclusionArea, bool& bValidPointFound, int32 MaxAttempts);
	UFUNCTION(BlueprintCallable, Category = "Utilities|Bounding Box")
	static FVector GetRandomPointInBoundingBoxWithExclusions(FVector BoundingBoxHalfExtent, const TArray<F_Area_Exclusion>& ExclusionAreas, bool& bValidPointFound, int32 MaxAttempts);

	UFUNCTION(BlueprintCallable, Category=  "Utilities|Bounding Box")
	static FVector GetRandomPointOnBoxPerimeter(FVector BoundingBoxHalfExtent);
	UFUNCTION(BlueprintCallable, Category = "Utilities|Bounding Box")
	static FVector GetRandomPointOnBoxPerimeterWithExclusion(FVector BoundingBoxHalfExtent, const F_Area_Exclusion& ExclusionArea, bool& bValidPointFound, int32 MaxAttempts, float HeightOffset);
	UFUNCTION(BlueprintCallable, Category = "Utilities|Bounding Box")
	static FVector GetRandomPointOnBoxPerimeterWithExclusions(FVector BoundingBoxHalfExtent, const TArray<F_Area_Exclusion>& ExclusionAreas, bool& bValidPointFound, int32 MaxAttempts, float HeightOffset);

	// Radius

	UFUNCTION(BlueprintCallable, Category = "Utilities|Radius")
	static bool IsPointWithinRadius(FVector Point, FVector Position, float Radius);
};
