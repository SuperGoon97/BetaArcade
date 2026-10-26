#include "Data/PartDat.h"
#include "Components/PrimitiveComponent.h"

UPDAPart::UPDAPart()
    : Super() {

    int Sides = 6;

    TArray<FVector> InitialPoints = {
        FVector(100, 0, 0),
        FVector(-100, 0, 0),
        FVector(0, 100, 0),
        FVector(0, -100, 0),
        FVector(0, 0, 100),
        FVector(0, 0, -100)
    };

    if (CollisionPoints.IsEmpty()) {
        CollisionPoints = InitialPoints;
    }
}

FBox UPDAPart::GetBoundingBox() {
    FVector MinBounds = CollisionPoints[0];
    FVector MaxBounds = CollisionPoints[0];

    for (int32 i = 0; i < CollisionPoints.Num(); i++) {
        MinBounds.X = FMath::Min(MinBounds.X, CollisionPoints[i].X);
        MinBounds.Y = FMath::Min(MinBounds.Y, CollisionPoints[i].Y);
        MinBounds.Z = FMath::Min(MinBounds.Z, CollisionPoints[i].Z);

        MaxBounds.X = FMath::Max(MaxBounds.X, CollisionPoints[i].X);
        MaxBounds.Y = FMath::Max(MaxBounds.Y, CollisionPoints[i].Y);
        MaxBounds.Z = FMath::Max(MaxBounds.Z, CollisionPoints[i].Z);
    }

    /*
    We can construct an FBox. FBox is also usable in Blueprints, though it is instead called "Box" in blueprints, as they remove the prefix.
    This can be later saved in the UPDAPart* when editing, or created as a function call like GetBoundingBox()

    TODO : Make above function ref or save FBox in the part itself.
    */
    return FBox(MinBounds, MaxBounds);
}

float UPDAPart::GetHealth() {
    return Health;
}

float UPDAPart::SetHealth(float NewHealth) {
    Health = NewHealth;
    return Health;
}

UNiagaraSystem* UPDAPart::GetDeathParticle() {
    return NS_Death;
}

USoundBase* UPDAPart::GetDeathSound() {
    return SQ_Death;
}

bool UPDAPart::IsObjectMesh(UObject* InObject) {
    UStaticMesh* Mesh = Cast<UStaticMesh>(InObject);
    return IsValid(Mesh);
}

FTooltip& UPDAPart::GetTooltipData() {
    return Tooltip;
}

FWeapon& UPDAPart::GetWeaponData() {
    return WeaponTooltip;
}

TSubclassOf<AActor> UPDAPart::GetObjectClass() {
    if (Object) {
        return Object->GetClass();
    }
    return nullptr;
}
