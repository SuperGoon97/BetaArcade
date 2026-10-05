#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Meta/Structs/Tooltip.h"
#include "Meta/Structs/WeaponTooltip.h"
#include "Meta/Structs/Anchor.h"
#include "PartDat.generated.h"

class UNiagaraSystem;
class USoundBase;

UCLASS(BlueprintType)
class UPDAPart : public UPrimaryDataAsset {
GENERATED_BODY()

public:
    UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Part Dat")
    FString PartName = TEXT("UNNAMED");
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    float Health;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    TObjectPtr<UNiagaraSystem> NS_Death;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    TObjectPtr<USoundBase> SQ_Death;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    TObjectPtr<UObject> Object;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    FTooltip Tooltip;
    UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Part Dat")
    FWeapon WeaponTooltip;
    UPROPERTY(BlueprintReadOnly, Category = "Part Data")
    TArray<FAnchor> Anchors;

public:
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float GetHealth();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Health")
    float SetHealth(float NewHealth);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Visuals")
    UNiagaraSystem* GetDeathParticle();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Audio")
    USoundBase* GetDeathSound();

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Construction")
    bool IsObjectMesh(UObject* InObject);

    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
    FTooltip& GetTooltipData();
    UFUNCTION(BlueprintCallable, BlueprintPure, Category = "UI")
    FWeapon& GetWeaponData();
};
