#include "Structs/FFoliageDataTable.h"

#if WITH_EDITOR
void UFoliageConfigData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    OnConfigChanged.Broadcast();
}
#endif
