#include "Viewport/RobotEditorViewportClient.h"
#include "Viewport/Utils/PartContentBrowser.h"
#include "Components/StaticMeshComponent.h"
#include "Data/PartDat.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {
        return;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorUtilsViewportClient::Tick(DeltaSeconds);

    /*
     * TODO : Turn this into a function and take it out of tick. Preferrably this could be something in the parent class: FEditorUtilsViewportClient.
     */

    UWorld* World = PreviewScene->GetWorld();

    TTuple<FVector, FVector> RayData = FEditorUtilsViewportClient::CastRayFromCameraToMouse();
    const auto& [RayStart, RayEnd] = RayData;

    FHitResult HitResult;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(m_PreviewActor);

    bool bHit = World->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_Visibility, Params);
    if (bHit) {
        AActor* HitActor = HitResult.GetActor();

        if (HitActor) {
            /*
             * Handle what to do when the ray actually hits an actor (Move the preview actor to the closest anchor that doesn't end up overlapping with the object. Consider requiring collisions)
             */
            return;
        }
    }
}

bool FRobotEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed) {
        /*
         *   If there isn't a part displayed on the screen, place immediately at 0, 0, 0. If there is a part on the screen, perform a cast and ensure that the cast is hitting something.
         *   If it isn't, pass this down thorugh FEditorUtilsViewportClient::InputKey(EventArgs);
         */
        if (!m_SelectedPart) { return FEditorUtilsViewportClient::InputKey(EventArgs); }
        if (m_Actors.Num() == 0) {
            CreatePartAtLocation(FVector::ZeroVector);
        }



         return true;
    }

    return FEditorUtilsViewportClient::InputKey(EventArgs);
}

void FRobotEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorUtilsViewportClient::Draw(View, PDI);
}

void FRobotEditorViewportClient::SetSelectedPart(UPDAPart* PartData) {
    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    if (m_PreviewActor) {
        m_PreviewActor->Destroy();
        m_PreviewActor->Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    if (PartData) {
        if (PartData->Object) {
            NewActor = FEditorUtilsViewportClient::CreateActorComponent(PartData->Object, SpawnParams);
            if (NewActor) { m_PreviewActor = NewActor; }
         }
    }

    TSet<UActorComponent*> Components = NewActor->GetComponents();
    for (auto& Component : Components) {
        UStaticMeshComponent* StaticMeshComponent = Cast<UStaticMeshComponent>(Component);
    }
}

void FRobotEditorViewportClient::UpdatePreviewActor() {
    return;
}

void FRobotEditorViewportClient::MovePreviewActorToLocation(FVector Location) {
    return;
}

void FRobotEditorViewportClient::CreatePartAtLocation(FVector Location) {
    return;
}

void FRobotEditorViewportClient::SetSelectedActorsData(const FPartData& PartData) {
    return;
}

TArray<FTransform> FRobotEditorViewportClient::GetAnchorsRelativeToPart(UPDAPart* Part) {
    return {};
}

FPartData FRobotEditorViewportClient::GetSelectedPartData() {
    return m_Actors[m_SelectedActor];
}


AActor* FRobotEditorViewportClient::GetActor() {
    return m_SelectedActor;
}

FVector FRobotEditorViewportClient::GetActorLocation(AActor* Actor) {
    return Actor->GetActorLocation();
}

void FRobotEditorViewportClient::OnThumbnailTriggered(UPDAPart* Part) {
    SetSelectedPart(Part);
    return;
}

void FRobotEditorViewportClient::ConnectTileSingleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailClicked.AddSP(this, &FRobotEditorViewportClient::OnThumbnailTriggered);
}

void FRobotEditorViewportClient::ConnectTileDoubleClick(SThumbnailTile* Tile) {
    return;
}

void FRobotEditorViewportClient::OnAddButtonPressed() {
    return;
}
