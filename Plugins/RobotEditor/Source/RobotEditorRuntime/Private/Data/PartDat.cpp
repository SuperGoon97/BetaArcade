#include "Data/PartDat.h"

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
