#pragma once

#include "CoreMinimal.h"
#include "WeaponTooltip.generated.h"

USTRUCT(BlueprintType)
struct FWeapon {
GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    float FireRate;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon Data")
    float ProjectileDamage;

    FWeapon()
        : FireRate(0.0),
          ProjectileDamage(0.0)
    {}
};
