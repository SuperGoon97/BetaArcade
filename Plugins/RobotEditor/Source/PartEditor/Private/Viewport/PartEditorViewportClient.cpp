#include "Viewport/PartEditorViewportClient.h"
#include "Viewport/PartEditorViewport.h"
#include "Engine/CollisionProfile.h"
#include "Viewport/Utils/ContentPanel.h"
#include "DrawDebugHelpers.h"
#include "Data/PartDat.h"
#include "SnappingUtils.h"
#include "ScopedTransaction.h"
#include "MaterialDomain.h"
#include "Materials/MaterialRenderProxy.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "FileHelpers.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "UObject/Package.h"
#include "Components/SkyLightComponent.h"
#include "Engine/SkyLight.h"
#include "UnrealWidget.h"

#define LOCTEXT_NAMESPACE "PartEditor"

FPartEditorViewportClient::FPartEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {
        return;
}

void FPartEditorViewportClient::SetDetailsView(TSharedPtr<IDetailsView> InDetailsView) {
    FEditorUtilsViewportClient::SetDetailsView(InDetailsView);
    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->OnFinishedChangingProperties().AddRaw(this, &FPartEditorViewportClient::OnInspectorPropertyChanged);
    }
}

void FPartEditorViewportClient::OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) {
    if (!m_CurrentInspectedObject.IsValid()) return;
    UObject* ActiveObject = m_CurrentInspectedObject.Get();
    SetInspectedObject(ActiveObject);
}

bool FPartEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    /*
    Toggle drawing collision. Also ensure that we deselect our current selected collision point and set our selection type to none. Just below we'll also toggle drawing Anchors in the exact same way. You could consider it some copied code. Deselecting everything could easily be turned into it's own function.

    TODO : Turn Deselection into its own function.
    */
    if (EventArgs.Key == EKeys::C && EventArgs.Event == IE_Pressed && EventArgs.Viewport->KeyState(EKeys::LeftAlt)) {
        m_bDrawCollision = !m_bDrawCollision;
        SetSelectedIndex(INDEX_NONE);
        SetPartSelectionType(EPartSelectionType::None);
        Invalidate();
        return true;
    }
    if (EventArgs.Key == EKeys::A && EventArgs.Event == IE_Pressed && EventArgs.Viewport->KeyState(EKeys::LeftAlt)) {
        m_bDrawAnchors = !m_bDrawAnchors;
        SetSelectedIndex(INDEX_NONE);
        SetPartSelectionType(EPartSelectionType::None);
        Invalidate();
        return true;
    }

    if (EventArgs.Key == EKeys::LeftMouseButton) {
        if (EventArgs.Event == IE_Pressed) {
            if (Viewport && m_CurrentInspectedObject.IsValid()) {
                int32 MouseX = Viewport->GetMouseX();
                int32 MouseY = Viewport->GetMouseY();

                /*
                Firstly, we create a hit result. This hit result just checks for whether we've intersected with a UI Gizmo. In the case that we have, we ensure that this gizmo is selected.
                */

                HHitProxy* HitResult = Viewport->GetHitProxy(MouseX, MouseY);
                if (HitResult && HitResult->IsA(HWidgetAxis::StaticGetType())) {
                    HWidgetAxis* WidgetAxis = (HWidgetAxis*)HitResult;
                    SetCurrentWidgetAxis(WidgetAxis->Axis);
                    return FEditorUtilsViewportClient::InputKey(EventArgs);
                }

                /*
                If there was no selection, we can simply tell the viewport that there is no gizmo to edit.
                */

                SetCurrentWidgetAxis(EAxisList::None);

                /*
                We simulate our clicks by using a raycast with a large depth multiplier. This allows us to select objects far away.
                We don't specifically need any of our objects to be on this ray, in our later calculations we try to actually get the closest point *to* the ray
                rather than an exact click. This gives the user some lee-way and makes our selection logic slightly easier too.
                */

                // FViewportCursorLocation CursorLocation = GetCursorWorldLocationFromMousePos();
                // FVector RayOrigin = CursorLocation.GetOrigin();
                // FVector RayDirection = CursorLocation.GetDirection();
                // FVector RayEnd = RayOrigin + (RayDirection * 10000.0f);

                TTuple<FVector, FVector> RayData = CastRayFromCameraToMouse();

                if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                    int32 SelectedIndex = INDEX_NONE;
                    EPartSelectionType SelectedType = EPartSelectionType::None;
                    float BestHitDistance = MAX_flt;

                    if (m_bDrawAnchors) {
                        TArray<FVector> TempPoints;
                        TempPoints.Reserve(ActivePart->Anchors.Num());
                        for (const FAnchor& Anchor : ActivePart->Anchors) {
                            TempPoints.Add(Anchor.Position);
                        }
                        const auto& [_SelectedIndex, bWasSelected, ClosestDistance] = GetIndexOfClosestPoint(TempPoints, RayData, BestHitDistance);
                        if (bWasSelected) {
                            SelectedIndex = _SelectedIndex;
                            SelectedType = EPartSelectionType::Anchor;
                            BestHitDistance = ClosestDistance;
                        }
                    }

                    if (m_bDrawCollision) {
                        const auto& [_SelectedIndex, bWasSelected, ClosestDistance] = GetIndexOfClosestPoint(ActivePart->CollisionPoints, RayData, BestHitDistance);
                        if (bWasSelected) {
                            SelectedIndex = _SelectedIndex;
                            SelectedType = EPartSelectionType::CollisionPoint;
                            BestHitDistance = ClosestDistance;
                        }
                    }

                    m_SelectedIndex = SelectedIndex;
                    m_SelectedType = SelectedType;
                    Invalidate();
                }
            }
            return true;
        }
        /*
        On release, if we have a widget - we need to deselect our widget.
        Otherwise, we can just simply return false and return true to force handle any inputs.
        */
        else if (EventArgs.Event == IE_Released) {
            if (GetCurrentWidgetAxis() != EAxisList::None) {
                SetCurrentWidgetAxis(EAxisList::None);
                Invalidate();
                return FEditorUtilsViewportClient::InputKey(EventArgs);
            }

            SetCurrentWidgetAxis(EAxisList::None);
            Invalidate();
            return true;
        }
    }

    return FEditorUtilsViewportClient::InputKey(EventArgs);
}

TTuple<int32, bool, float> FPartEditorViewportClient::GetIndexOfClosestPoint(const TArray<FVector>& Points, const TTuple<FVector, FVector>& RayInfo, float CurrentClosestPoint) {
    const auto& [RayStart, RayEnd] = RayInfo;

    int32 ClosestIndex = INDEX_NONE;
    double ClosestDistance = CurrentClosestPoint;
    bool bWasSelected = false;

    for (int i = 0; i < Points.Num(); i++) {
        const FVector& CurrentPos = Points[i];

        FVector ClosestPointOnRay = FMath::ClosestPointOnSegment(CurrentPos, RayStart, RayEnd);
        float DistanceToRay = FVector::Dist(CurrentPos, ClosestPointOnRay);

        if (DistanceToRay < ClosestDistance && DistanceToRay < 10.0f) {
            ClosestDistance = DistanceToRay;
            ClosestIndex = i;
            bWasSelected = true;
        }
    }

    return MakeTuple(ClosestIndex, bWasSelected, ClosestDistance);
}

void FPartEditorViewportClient::SetInspectedObject(UObject* NewPartObject) {
    if (!NewPartObject) return;

    m_CurrentInspectedObject = NewPartObject;

    if (m_AssociatedDetailsView.IsValid()) {
        m_AssociatedDetailsView->SetObject(NewPartObject);
    }

    UWorld* PreviewWorld = PreviewScene->GetWorld();
    if (!PreviewWorld) return;

    if (m_SpawnedPreviewActor.IsValid()) {
        m_SpawnedPreviewActor->Destroy();
        m_SpawnedPreviewActor.Reset();
    }

    AActor* NewActor = nullptr;
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
    UPDAPart* DataAsset = Cast<UPDAPart>(NewPartObject);

    if (DataAsset) {
        if (DataAsset->Object) {
            NewActor = FEditorUtilsViewportClient::CreateActorComponent(DataAsset->Object, SpawnParams);
            if (NewActor) { m_SpawnedPreviewActor = NewActor; }
         }
    }

    OnSelectedObjectChanged.Broadcast();
    Invalidate();
}

// DRAWING AND WIDGETS


/*
This is a custom drawing override function so that we can draw our own little things ontop. This allows us to draw Anchors, Collision Points and even custom boxes around that for preview.
We can extend this to draw more in the future, or we can re-use this later on in a different plugin or feature to draw some other things with similar logic.
For now, this function exists solely to give the user a way to see them edit things within our viewport client.
*/
void FPartEditorViewportClient::Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) {
    FEditorUtilsViewportClient::Draw(View, PDI);

    if (!PDI) return;

    /*
    If we have an active part, we want to be able to draw our specific data. Anchors and Collision Points are not themselves objects, and therefore are not drawn by the Engine.
    We also want the end-user to be able to toggle whether they should draw Anchors and/or Collision Points. Those toggles are handled within InputKey.

    While FPrimitiveDrawInterface has many useful functions, it is also a function parameter itself. In the case of drawing Boxes:
    (DrawBox)
    (DrawWireBox)
    PDI is an input rather than the caller, despite the fact that PDI itself has DrawPoint.

    This is an odd quirk of UnrealEngine.
    */

    if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {

        if (m_bDrawAnchors) {
            for (int32 i = 0; i < ActivePart->Anchors.Num(); i++) {
                FVector AnchorLocation = ActivePart->Anchors[i].Position;

                /*
                TODO : Change FColor to FLinearColor on these properties.
                */
                FColor RenderColor = c_UnselectedAnchorColor;
                float CorePointSize = c_AnchorSize;

                if (m_SelectedType == EPartSelectionType::Anchor) {
                    if (i == m_SelectedIndex) {
                        RenderColor = c_SelectedAnchorColor;
                        CorePointSize = c_SelectedAnchorSize;
                    }
                }

                PDI->DrawPoint(AnchorLocation, FLinearColor(RenderColor), CorePointSize, SDPG_World);
            }
        }

        if (m_bDrawCollision) {

            for (int32 i = 0; i < ActivePart->CollisionPoints.Num(); i++) {
                FVector CollisionLocation = ActivePart->CollisionPoints[i];

                FLinearColor RenderColor = c_CollisionPoint;
                float CorePointSize = c_AnchorSize;

                if (m_SelectedType == EPartSelectionType::CollisionPoint) {
                    if (i == m_SelectedIndex) {
                        RenderColor = c_SelectedCollisionPoint;
                        CorePointSize = c_AnchorSize;
                    }
                }

                PDI->DrawPoint(CollisionLocation, RenderColor, CorePointSize, SDPG_World);
            }

            /*
            We want to draw our box, which means we need to find the minimum and maximum extent of our points. We have 6 points in total since we're working with a square.
            We just need to find the minimum of all of the X, Y and Z axis through a loop. This has O(n) complexity.
            */

            FBox BoundingBox = ActivePart->GetBoundingBox();

            /*
            We want a transparent texture for our box, while it would be great if we could simply use our c_BoundingBoxInnards colour, we have to use a material proxy, in this case a:
            FColoredMaterialRenderProxy. But, all we have to do is feed in our colour and that sets the material's albedo to our requested transparent c_BoundingBoxInnards.
            */

            FColoredMaterialRenderProxy* SafeProxy = new FColoredMaterialRenderProxy(
                GEngine->GeomMaterial->GetRenderProxy(),
                c_BoundingBoxInnards
            );

            DrawBox(PDI, FTransform(BoundingBox.GetCenter()).ToMatrixWithScale(), BoundingBox.GetExtent(), SafeProxy, SDPG_World);
            DrawWireBox(PDI, BoundingBox, FLinearColor::Red, SDPG_World, c_BoundingBoxWireThickness, 0.0f, false);
        }
    }
}

/*
We can actually change which widget draws depending on a specific input / UI element toggled or object selected.
In our case, we only care about whether anchor or collision poinst are selected. If so, we just simply return WM_Translate.
WM_Translate is the typical Move gizmo.

Otherwise, we want our gizmo to disappear, and therefore return WM_None.
*/
UE::Widget::EWidgetMode FPartEditorViewportClient::GetWidgetMode() const {
    if (m_SelectedType == EPartSelectionType::Anchor || m_SelectedType == EPartSelectionType::CollisionPoint) {
        if (m_SelectedIndex != INDEX_NONE && m_CurrentInspectedObject.IsValid()) {
            return UE::Widget::WM_Translate;
        }
    }
    return UE::Widget::WM_None;
}

/*
This updates the position of the gizmo based on where our selected part is, if there is no selected part, the gizmo will just draw at ZERO in the world, although this would mean something is broken.
In that case, revise this code.
*/
FVector FPartEditorViewportClient::GetWidgetLocation() const {
    switch(m_SelectedType) {
        case EPartSelectionType::Anchor: {
            if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                if (ActivePart->Anchors.IsValidIndex(m_SelectedIndex)) {
                    return ActivePart->Anchors[m_SelectedIndex].Position;
                }
            }
            break;
        }
        case EPartSelectionType::CollisionPoint: {
            if (UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get())) {
                if (ActivePart->CollisionPoints.IsValidIndex(m_SelectedIndex)) {
                    return ActivePart->CollisionPoints[m_SelectedIndex];
                }
            }
            break;
        }
        default:
            break;
    }

    return FVector::ZeroVector;
}

/*
Here, we move our gizmos based on the delta-input of our widget. This is handled separate to InputAxis and InputKey, although the latter is what allows this function to work.
We don't need to snap positions for our anchors, these can be freely placed and fine-tuned through the details panel on the right. But for our collision points,
we wish to snap to a specific value.
*/
bool FPartEditorViewportClient::InputWidgetDelta(FViewport* InViewport, EAxisList::Type InAxis, FVector& Drag, FRotator& Rot, FVector& Scale) {
    if (InAxis == EAxisList::None || Drag.IsZero()) {
        return false;
    }

    UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
    if (!ActivePart) return false;

    bool bHandled = false;

    switch(m_SelectedType) {
        case EPartSelectionType::Anchor: {
            if (ActivePart->Anchors.IsValidIndex(m_SelectedIndex)) {
                ActivePart->Modify();

                FVector CurrentPos = ActivePart->Anchors[m_SelectedIndex].Position;
                FVector RawNewPos = CurrentPos + Drag;
                FVector SnappedNewPos = RawNewPos;

                FSnappingUtils::SnapPointToGrid(SnappedNewPos, FVector(c_GridSnap));

                SnappedNewPos.X = FMath::RoundToDouble(RawNewPos.X / c_GridSnap) * c_GridSnap;
                SnappedNewPos.Y = FMath::RoundToDouble(RawNewPos.Y / c_GridSnap) * c_GridSnap;
                SnappedNewPos.Z = FMath::RoundToDouble(RawNewPos.Z / c_GridSnap) * c_GridSnap;
                Drag = RawNewPos - CurrentPos;

                ActivePart->Anchors[m_SelectedIndex].Position = SnappedNewPos;
                bHandled = true;
            }
            break;
        }
        case EPartSelectionType::CollisionPoint: {
            if (ActivePart->CollisionPoints.IsValidIndex(m_SelectedIndex)) {
                ActivePart->Modify();
                FVector CurrentPos = ActivePart->Anchors[m_SelectedIndex].Position;
                FVector RawNewPos = CurrentPos + Drag;
                FVector SnappedNewPos = RawNewPos;

                FSnappingUtils::SnapPointToGrid(SnappedNewPos, FVector(c_GridSnap));

                SnappedNewPos.X = FMath::RoundToDouble(RawNewPos.X / c_GridSnap) * c_GridSnap;
                SnappedNewPos.Y = FMath::RoundToDouble(RawNewPos.Y / c_GridSnap) * c_GridSnap;
                SnappedNewPos.Z = FMath::RoundToDouble(RawNewPos.Z / c_GridSnap) * c_GridSnap;
                Drag = RawNewPos - CurrentPos;

                ActivePart->CollisionPoints[m_SelectedIndex] = SnappedNewPos;
                bHandled = true;
            }
            break;
        }
        default:
            break;
    }
    if (bHandled) {
        ActivePart->MarkPackageDirty();

        if (m_AssociatedDetailsView.IsValid()) {
            m_AssociatedDetailsView->SetObject(ActivePart, true);
        }
        Invalidate();
        return bHandled;
    }

    return false;
}

/*
Our transactions need to be tracked within this overriden function in order to track our dragging inputs.
If we were to call this inside of the widget move function, each transaction would be a snapshot of many positions within the movement of the point.
In doing it when the tracking begins and ends, we create a larger scoped transaction that ends the transaction only at the end of the entire movement.
(Or when the user releases the button that began dragging.)
*/
void FPartEditorViewportClient::TrackingStarted(const FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge) {
    FEditorUtilsViewportClient::TrackingStarted(InInputState, bIsDraggingWidget, bNudge);

    if (bIsDraggingWidget && !m_ScopedDragTransaction.IsValid()) {
        UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
        if (ActivePart) {
            if (!ActivePart->HasAnyFlags(RF_Transactional)) {
                ActivePart->SetFlags(RF_Transactional);
            }

            /*
            While we could return a single transaction message of "Edited Properties," I believe swapping it here depending on which property was specifically edited: Anchor vs Collision Point, gives greater feedback to the end-user and so this extra little bit of code is absoultely fine to add.
            */
            switch(m_SelectedType) {
                case EPartSelectionType::Anchor: {
                    m_ScopedDragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("EditPartProperties", "Edit Anchor Position."));
                    break;
                }
                case EPartSelectionType::CollisionPoint: {
                    m_ScopedDragTransaction = MakeUnique<FScopedTransaction>(LOCTEXT("EditPartProperties", "Edit Collision Point."));
                    break;
                }
                default:
                    break;
            }

            ActivePart->Modify();
        }
    }
}

void FPartEditorViewportClient::TrackingStopped() {
    FEditorUtilsViewportClient::TrackingStopped();
    if (m_ScopedDragTransaction.IsValid()) {
        m_ScopedDragTransaction.Reset();
    }
}


// SAVING ASSETS


void FPartEditorViewportClient::SaveActiveAsset() {
    if (!m_CurrentInspectedObject.IsValid()) return;

    UPDAPart* ActivePart = Cast<UPDAPart>(m_CurrentInspectedObject.Get());
    if (!ActivePart) return;

    /*
    Firstly, we get the package of the object that we're editing.
    We then perform a little magic on our object's name, this removes all spaces and replaces them with _'s.
    */
    UPackage* CurrentPackage = ActivePart->GetPackage();
    FString CurrentName = ActivePart->PartName;
    CurrentName = CurrentName.Replace(TEXT(" "), TEXT("_"));

    /*
    If there's a cheeky user who thinks that they can just remove the name and get around our unsaved logic check, I reset it back to UNNAMED and force them to give it a name through an if later.
    */
    if (CurrentName.IsEmpty() || CurrentName.Len() == 0) {
        CurrentName = TEXT("UNNAMED");
    }

    /*
    We update the part name to the space-replaced current_name.
    */
    ActivePart->PartName = CurrentName;

    FString AssetGamePath = TEXT("/RobotEditor/Data/Parts");
    FEditorUtilsViewportClient::ReloadAssetRegistryModule(AssetGamePath);

    /*
    If our package already exists, was not created transiently by our editor, we can save it.
    This includes packages that have the inner name of UNNAMED, as they could've been created through the content-browser instead.
    We do not automatically handle renaming files through our save system, that's for the end-user to manage.
    */
    if (CurrentPackage != GetTransientPackage()) {
        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(CurrentPackage);

        CurrentPackage->MarkPackageDirty();
        ActivePart->MarkPackageDirty();

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
    }
    /*
    We force the user to give the file a name through a save dialog, IF the file is transient and the name field was left as UNNAMED.
    This includes UNNAMED in any sort of case.
    */
    else if (CurrentName.Equals(TEXT("UNNAMED"), ESearchCase::IgnoreCase)) {
        FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

        /*
        We can create this neat little save dialog through Unreal Engine's built in SaveAssetDialog set-up. Which is actually really neat!
        I don't know why I didn't expect this to already be set-up for us, but they use it themselves in an innumerable amount of places. Having it be a useful re-usable dialog only makes sense.
        I should take notes with some of my utility functions in here! I have a few I could add to some slate drawing utils...
        */
        FSaveAssetDialogConfig SaveDialogConfig;
        SaveDialogConfig.DialogTitleOverride = INVTEXT("Save new Asset");
        SaveDialogConfig.DefaultPath = AssetGamePath;
        SaveDialogConfig.DefaultAssetName = TEXT("Unnamed");
        SaveDialogConfig.AssetClassNames.Add(UPDAPart::StaticClass()->GetClassPathName());

        /*
        There are some event delegates that are constructed that we can attach to, so we may as-well create some lambda functions in our body here.
        The first one just saves it where requested, ChosenPackagePath, and does so as a new asset.
        */
        FOnObjectPathChosenForSave OnSavePressed = FOnObjectPathChosenForSave::CreateLambda([this, ActivePart](const FString& ChosenPackagePath) {
            FString CleanAssetName = FPaths::GetBaseFilename(ChosenPackagePath);
            FString TargetFolderPath = FPaths::GetPath(ChosenPackagePath);

            ActivePart->Rename(*CleanAssetName, ActivePart->GetOuter());

            SaveNewAsset(ActivePart, TargetFolderPath, *CleanAssetName);
        });

        /*
        There isn't really anything that we need to do when the user cancels, so we'll just close the dialog and move on with our day.
        By constructing the asset dialog!
        */
        FOnAssetDialogCancelled OnCancelPressed = FOnAssetDialogCancelled::CreateLambda([]() {
        });

        ContentBrowserModule.Get().CreateSaveAssetDialog(SaveDialogConfig, OnSavePressed, OnCancelPressed);
    }
    /*
    In our final case, the package the user is saving is transient but the user has given it a name. We can just save this directly within our plugin's content directory.
    In this case it's: `/RobotEditor/Data/`
    A lot of the logic done here is similar to the logic done in the lambda above. We give the location, we give create the asset and we save it in the same way.
    */
    else if (CurrentPackage == GetTransientPackage()) {
        FString TargetFolderPackagePath = AssetGamePath;
        SaveNewAsset(ActivePart, TargetFolderPackagePath, CurrentName);
    }

    if (SContentBrowser::m_Instance.IsValid()) {
        SContentBrowser::m_Instance.Pin()->ScanPluginDirectory();
    }
}

void FPartEditorViewportClient::SaveNewAsset(UPDAPart* ActivePart, const FString& ChosenPackagePath, const FString& CurrentName) {
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    FString UniquePackagePath;
    FString UniqueAssetName;

    AssetToolsModule.Get().CreateUniqueAssetName(ChosenPackagePath + TEXT("/") + CurrentName, TEXT(""), UniquePackagePath, UniqueAssetName);
    /*
    Using the asset tools module, we need to create a new asset of our static class type of UPDAPart.
    We need to give it a name, a path and the class.
    We could give it a factory, but we don't have one, Unreal Engine already has a DataAsset factory built in.
    In-fact, it has one built in for anything that extends UObject, really.
    */
    UPDAPart* NewSavedAsset = Cast<UPDAPart>(AssetToolsModule.Get().CreateAsset(
        UniqueAssetName,
        ChosenPackagePath,
        UPDAPart::StaticClass(),
        nullptr
    ));

    /*
    If our save is successful, then we do a little bit of logic here to load everything and make sure it's ready to edit.
    */
    if (NewSavedAsset) {
        NewSavedAsset->Object = ActivePart->Object;
        NewSavedAsset->Anchors = ActivePart->Anchors;
        NewSavedAsset->PartName = UniqueAssetName;

        NewSavedAsset->MarkPackageDirty();
        NewSavedAsset->GetPackage()->MarkPackageDirty();

        TArray<UPackage*> PackagesToSave;
        PackagesToSave.Add(NewSavedAsset->GetPackage());

        UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);

        SetInspectedObject(NewSavedAsset);
    }
}

void FPartEditorViewportClient::OnThumbnailTriggered(UPDAPart* Part) {
    SetInspectedObject(Part);
    SetSelectedIndex(INDEX_NONE);
    return;
}

void FPartEditorViewportClient::ConnectTileSingleClick(SThumbnailTile* Tile) {
    return;
}

void FPartEditorViewportClient::ConnectTileDoubleClick(SThumbnailTile* Tile) {
    Tile->OnThumbnailDoubleClicked.AddSP(this, &FPartEditorViewportClient::OnThumbnailTriggered);
}

void FPartEditorViewportClient::OnAddButtonPressed() {
    if (m_CurrentInspectedObject.Pin() && m_CurrentInspectedObject.Pin()->GetPackage() && m_CurrentInspectedObject.Pin()->GetPackage()->IsDirty()) {
        FText DialogText = LOCTEXT("UnsavedChangesBody", "You may have unsaved data, do you wish to create a new part?");
        FText DialogTitle = LOCTEXT("UnsavedChangesTitle", "UnsavedChanges");

        EAppReturnType::Type UserChoice = FMessageDialog::Open(
            EAppMsgType::YesNo,
            DialogText,
            DialogTitle
        );

        if (UserChoice != EAppReturnType::Yes){
                return;
        }
    }

    UPDAPart* NewBlankPart = NewObject<UPDAPart>(GetTransientPackage(), UPDAPart::StaticClass());
    if (NewBlankPart) {
        OnThumbnailTriggered(NewBlankPart);
    }
}

#undef LOCTEXT_NAMESPACE
