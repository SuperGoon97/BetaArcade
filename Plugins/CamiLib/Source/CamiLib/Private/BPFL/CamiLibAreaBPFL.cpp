#include "BPFL/CamiLibAreaBPFL.h"

// Area

FVector UCamiLibAreaBPLibrary::GetRandomPointInBoundingBox(FVector BoundingBoxHalfExtent) {
    FVector GeneratedPoint = FVector::ZeroVector;

    GeneratedPoint.X = FMath::FRandRange(-BoundingBoxHalfExtent.X, BoundingBoxHalfExtent.X);
    GeneratedPoint.Y = FMath::FRandRange(-BoundingBoxHalfExtent.Y, BoundingBoxHalfExtent.Y);
    GeneratedPoint.Z = FMath::FRandRange(-BoundingBoxHalfExtent.Z, BoundingBoxHalfExtent.Z);

    return GeneratedPoint;
}

FVector UCamiLibAreaBPLibrary::GetRandomPointInBoundingBoxWithExclusion(FVector BoundingBoxHalfExtent, const F_Area_Exclusion& ExclusionArea, bool& bValidPointFound, int32 MaxAttempts = 100) {
    FVector GeneratedPoint = FVector::ZeroVector;
    bValidPointFound = false;

    int32 Attempts = 0;

    while (!bValidPointFound && Attempts < MaxAttempts) {
        Attempts++;

        GeneratedPoint = GetRandomPointInBoundingBox(BoundingBoxHalfExtent);
        bool bIsWithinPoint = IsPointWithinRadius(GeneratedPoint, ExclusionArea.Position, ExclusionArea.Radius);

        if (!bIsWithinPoint) {
            bValidPointFound = true;
        }
    }
    return GeneratedPoint;
}

FVector UCamiLibAreaBPLibrary::GetRandomPointInBoundingBoxWithExclusions(FVector BoundingBoxHalfExtent, const TArray<F_Area_Exclusion>& ExclusionAreas, bool& bValidPointFound, int32 MaxAttempts = 100) {
    FVector GeneratedPoint = FVector::ZeroVector;
    bValidPointFound = false;

    int32 Attempts = 0;

    while (!bValidPointFound && Attempts < MaxAttempts) {
        Attempts++;

        GeneratedPoint = GetRandomPointInBoundingBox(BoundingBoxHalfExtent);

        bool bIsInsideAnyExclusion = false;

        for (const F_Area_Exclusion& Area : ExclusionAreas) {
            if (IsPointWithinRadius(GeneratedPoint, Area.Position, Area.Radius)) {
                bIsInsideAnyExclusion = true;
                break;
            }
        }

        if (!bIsInsideAnyExclusion) {
            bValidPointFound = true;
        }
    }

    return GeneratedPoint;
}

// Perimeter

FVector UCamiLibAreaBPLibrary::GetRandomPointOnBoxPerimeter(FVector BoundingBoxHalfExtent) {
    const float HalfX = BoundingBoxHalfExtent.X;
    const float HalfY = BoundingBoxHalfExtent.Y;

    const float Width = BoundingBoxHalfExtent.X * 2.0f;
    const float Height = BoundingBoxHalfExtent.Y * 2.0f;

    const float TopSeg = Width;
    const float RightSeg = Height;
    const float BottomSeg = Width;
    const float LeftSeg = Height;

    const float TotalPerimeter = TopSeg + RightSeg + BottomSeg + LeftSeg;

    // Pick a random distance along the entire perimeter line. For example
    // You may have a Half X of 10, a Half Y of 5. Double this to get Width and Height along one axis. Double the sum to make a rectangle and you end up with 60 ((20 + 10) * 2)
    float Distance = FMath::FRandRange(0.0f, TotalPerimeter);
    FVector2D EdgePoint2D = FVector2D::ZeroVector;

    // Now, we see if this distance matches our "top segment", this being the first "20" in our sum. Lets say it generated a number of 27 out of our 60.
    // 27 > 20, so therefore we pass onto the next one.
    if (Distance <= TopSeg) {
        EdgePoint2D.X = -HalfX + Distance;
        EdgePoint2D.Y = HalfY;
    }
    // Here we then subtract our top segment in our check, and we do this progressively until we get the edge that we are on. In this case, our segment is 10 long, so we check for whether that's less than or = to 10
    else if ((Distance -= TopSeg) <= RightSeg) {
        EdgePoint2D.X = HalfX;
        EdgePoint2D.Y = HalfY - Distance;
    }
    // Right segment length (10) subtracted too... So now we do the bottom segment which is equal to the top segment's 20
    else if ((Distance -= RightSeg) <= BottomSeg) {
        EdgePoint2D.X = HalfX - Distance;
        EdgePoint2D.Y = -HalfY;
    }
    else {
    // Until eventually, we *have* to be on the bottom segment, and we can then determine where we are on that line within our 2D box bounds.
        Distance -= BottomSeg;
        EdgePoint2D.X = -HalfX;
        EdgePoint2D.Y = -HalfY + Distance;
    }

    // So now our generated point is where this has landed, along with a height offset to determine the Z-placement of whatever object is in 2D space.
    return FVector(EdgePoint2D.X, EdgePoint2D.Y, 0.0f);
}

FVector UCamiLibAreaBPLibrary::GetRandomPointOnBoxPerimeterWithExclusion(FVector BoundingBoxHalfExtent, const F_Area_Exclusion& ExclusionArea, bool& bValidPointFound, int32 MaxAttempts = 100, float HeightOffset = 0.0f) {
    FVector GeneratedPoint = FVector::ZeroVector;
    bValidPointFound = false;

    int32 Attempts = 0;

    while (!bValidPointFound && Attempts < MaxAttempts) {
        Attempts++;

        GeneratedPoint = GetRandomPointOnBoxPerimeter(BoundingBoxHalfExtent);

        bool bIsWithinPoint = IsPointWithinRadius(GeneratedPoint, ExclusionArea.Position, ExclusionArea.Radius);

        if (!bIsWithinPoint) {
            bValidPointFound = true;
        }
    }
    GeneratedPoint.Z = HeightOffset;
    return GeneratedPoint;
}

FVector UCamiLibAreaBPLibrary::GetRandomPointOnBoxPerimeterWithExclusions(FVector BoundingBoxHalfExtent, const TArray<F_Area_Exclusion>& ExclusionAreas, bool& bValidPointFound, int32 MaxAttempts = 100, float HeightOffset = 0.0f) {
    FVector GeneratedPoint = FVector::ZeroVector;
    bValidPointFound = false;

    int32 Attempts = 0;

    while (!bValidPointFound && Attempts < MaxAttempts) {
        Attempts++;

        GeneratedPoint = GetRandomPointOnBoxPerimeter(BoundingBoxHalfExtent);

        // And we perform all our exclusion checks as we have before.
        bool bIsInsideAnyExclusion = false;
        for (const F_Area_Exclusion& Area : ExclusionAreas) {
            if (IsPointWithinRadius(GeneratedPoint, Area.Position, Area.Radius)) {
                bIsInsideAnyExclusion = true;
                break;
            }
        }

        if (!bIsInsideAnyExclusion) {
            bValidPointFound = true;
        }
    }
    GeneratedPoint.Z = HeightOffset;
    return GeneratedPoint;
}

// Radius

bool UCamiLibAreaBPLibrary::IsPointWithinRadius(FVector Point, FVector Position, float Radius) {
    float RadiusSq = Radius * Radius;
    return FVector::DistSquared(Point, Position) <= RadiusSq;
}
