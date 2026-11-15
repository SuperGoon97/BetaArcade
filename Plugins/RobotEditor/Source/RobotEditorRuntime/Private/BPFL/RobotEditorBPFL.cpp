#include "BPFL/RobotEditorBPFL.h"
#include "Engine/World.h"
#include "CollisionShape.h"
#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"

FSnapResult URobotEditorBPFL::GetSnapResult(const FVector& HitImpactPoint, AActor* HitActor, UPDAPart* HitPart, UPDAPart* PartToSnap, int CurrentRotationIndex) {
    if (!HitActor ||
        !HitPart || HitPart->Anchors.Num() < 1 ||
        !PartToSnap || PartToSnap->Anchors.Num() < 1) {
            return FSnapResult(false);
    }

    FTransform HitTransform = HitActor->GetActorTransform();
    FAnchor TargetHitAnchor;
    FVector ClosestAnchorWorldPos = FVector::ZeroVector;
    float ClosestDistanceSq = MAX_FLT;
    int TargetAnchorIndex = INDEX_NONE;
    int CurrentIndex = 0;

    for (const FAnchor& Anchor : HitPart->Anchors) {
        FVector AnchorWorldPos = HitTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, HitImpactPoint);

        if (DistanceSq < ClosestDistanceSq) {
            ClosestAnchorWorldPos = AnchorWorldPos;
            TargetAnchorIndex = CurrentIndex;
            ClosestDistanceSq = DistanceSq;
            TargetHitAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /*
     * If there isn't an anchor within a relatively normal set distance, (in this case we'll use the imaginary numer Infinity as our test point), then there's nothing to snap to.
     */

    if (ClosestDistanceSq > MAX_FLT) {
        return FSnapResult(false);
    }

    /*
     * We need to get the normal of the anchor based on the transform of the actor that we're reading from. We just simply call GetSafeNormal(); A fun little function that already exists inside Unreal.
     * But that's not all, we also need to find the target snap. This acts as the "opposite" of the hit anchor normal, so it ensures that the anchor that you get always has a normal facing INTO it,
     * rather than away from.
     */

    FVector HitAnchorNormalWorld = HitTransform.TransformVector(TargetHitAnchor.Position.GetSafeNormal()).GetSafeNormal();
    FVector TargetSnapNormalWorld = -HitAnchorNormalWorld;

    /*
     * Now we score differently here, because the object can be drastically different depending on where you're colliding. We need to factor in a few things here.
     * 1) Orientation of the object:
     *
     *  An object might be a cylinder, as opposed to a cube. This cylinder may be harder to calculate distances from if they're equidistant from the center.
     *
     * 2) The dot product of the normals
     *
     *  A normal might have a greater or lesser dot-product depending on where it is on the object.
     *
     * 3) The position of the anchors relative to the object.
     *
     * Combining these three rules gets us a total score that allows us to closer choose an anchor.
     */

    FTransform SnapTransform(HitImpactPoint);
    FAnchor SnapAnchor;
    int TargetSnapAnchorIndex = INDEX_NONE;
    float BestAnchorScore = -MAX_FLT;
    CurrentIndex = 0;

    for (const FAnchor& Anchor : PartToSnap->Anchors) {
        FVector LocalNormal = Anchor.Position.IsNearlyZero() ? -FVector::UpVector : Anchor.Position.GetSafeNormal();

        FVector AnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();
        float DirectionScore = FVector::DotProduct(AnchorNormalWorld, TargetSnapNormalWorld);

        FVector AnchorWorldPos = SnapTransform.TransformPosition(Anchor.Position);
        float DistanceSq = FVector::DistSquared(AnchorWorldPos, ClosestAnchorWorldPos);
        float ProximityScore = -(DistanceSq / 10000.0f);

        float TotalScore = DirectionScore + ProximityScore;

        if (TotalScore > BestAnchorScore) {
            TargetSnapAnchorIndex = CurrentIndex;
            BestAnchorScore = TotalScore;
            SnapAnchor = Anchor;
        }
        CurrentIndex++;
    }

    /*
     * Long, Boring and indescribable maths up ahead. This required Quarternions, Dot Products and Vector Transforms. I never want to touch maths ever again.
     * Nevertheless, time to explain how this works.
     *
     * Below, we declare LocalNormal and WorldNormals. LocalNormal determines that if an Anchor's position is nearly 0, 0, 0, we prevent a divide by 0 issue and declare that the anchor is pointing
     * upwards through an arbitrary rule. This should only affect anchors that are not correctly put in place.
     *
     * We then calculate the actual normal based on the transform in the world, and turn it into a value between -1.0 and 1.0 for our necessary transformations.
     */
    FVector LocalNormal = SnapAnchor.Position.IsNearlyZero() ? -FVector::UpVector : SnapAnchor.Position.GetSafeNormal();
    FVector SnapAnchorNormalWorld = SnapTransform.TransformVector(LocalNormal).GetSafeNormal();

    /*
     * We want our objects to handle rotations. For example, should you be hovering over the side of a six-sided cube and have an object with a single anchor point, this will align the points together.
     * Thankfully, Unreal has a bunch of useful Quarternion functions inside their actual Quarternion data, so we just need to find the shortest safe angular rotation to get us to our desired point.
     * That's what this lovely FindBetweenNormals function does, it finds the shortest angular path to a point.
     */

    FQuat AlignmentRotation = FQuat::FindBetweenNormals(SnapAnchorNormalWorld, TargetSnapNormalWorld);
    FQuat FinalRotation = AlignmentRotation * SnapTransform.GetRotation();

    // /*
    //  * Here we handle user-input-based rotation. When the user presses either Q or E, the m_CurrentRotationInputSteps will increment or decrement with a % of 4, ensuring that it will always wrap around
    //  * between 0 -> 3.
    //  *
    //  * We then multiply our current rotation input steps by the amount we actually want an object to rotate, in this case we just want snapping every 90 degrees or so, so we can do so here.
    //  * (I also did this during Advanced Development Techniques, through `(TAU * 4)) * rotation_num`. So this is a fun useful little tidbit for rotation snapping!)
    //  *
    //  * We then apply this rotation after the previous one. (Quarternion multiplication is different to actual multiplication. Consider it adding angles sequentially rather than multiplying the
    //  * values directly.)
    //  */

    float AngleOffset = CurrentRotationIndex * 90.0f;
    FQuat AxisStepRotation = FQuat(TargetSnapNormalWorld, FMath::DegreesToRadians(AngleOffset));
    FinalRotation = AxisStepRotation * FinalRotation;

    /*
     * We then rotate our anchor vector by this final quarternion rotation, and can set our location relative to all of the maths that we just performed to find the optimal place to put the damn thing.
     */
    FVector RotatedSnapLocalAnchor = FinalRotation.RotateVector(SnapAnchor.Position);
    FVector FinalWorldLocation = ClosestAnchorWorldPos - RotatedSnapLocalAnchor;

    /*
     * Then all we need to do is set the location and the rotation, and the snapping is complete.
     */

    FTransform FinalTransform(FinalRotation, FinalWorldLocation);
    return FSnapResult(true, FinalTransform, HitActor, TargetAnchorIndex, TargetSnapAnchorIndex);
}

bool URobotEditorBPFL::IsPartColliding(UWorld* World, const FTransform& Transform, UPDAPart* Part) {
    if (!World || !Part) {
        return false;
    }

    const FBox& LocalBox = Part->GetBoundingBox();
    if (!LocalBox.IsValid) {
        return false;
    }

    const FVector ScaledExtent = (LocalBox.GetExtent()) * m_ShapeCastMarginMultiplier;
    const FVector WorldCenter = Transform.TransformPosition(LocalBox.GetCenter());
    const FCollisionShape BoxShape = FCollisionShape::MakeBox(ScaledExtent);

    FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ShapeCastPartCollision), false);

    const bool bHasOverlap = World->OverlapBlockingTestByChannel(
        WorldCenter,
        Transform.GetRotation(),
        ECC_GameTraceChannel2,
        BoxShape,
        QueryParams
    );

    const bool bDrawDebug = false;

    if (bDrawDebug) {
        const FColor DebugColor = bHasOverlap ? FColor::Green : FColor::Red;
        const bool bPersistentLines = false;
        const float LifeTime = 2.0f;
        const uint8 DepthPriority = 0;
        const float Thickness = 1.5f;

        DrawDebugBox(
            World,
            WorldCenter,
            ScaledExtent,
            Transform.GetRotation(),
            DebugColor,
            bPersistentLines,
            LifeTime,
            DepthPriority,
            Thickness
        );
    }

    return !bHasOverlap;
}
