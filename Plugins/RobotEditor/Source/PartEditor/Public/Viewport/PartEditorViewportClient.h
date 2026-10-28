#pragma once

#include "CoreMinimal.h"
#include "Viewport/EditorViewportClient.h"
#include "IDetailsView.h"
#include "PreviewScene.h"

class UPDAPart;

enum class EPartSelectionType : uint8 {
    None,
    Anchor,
    CollisionPoint
};

/*
These delegates are going to be used within our class to offload function calls to the delegate system. This should allow other set-ups to listen for what we're doing in the client as-well,
such as the content browser, the anchors panel and the details view.
*/

DECLARE_MULTICAST_DELEGATE(FOnDeselection);
DECLARE_MULTICAST_DELEGATE(FOnSelectedObjectChanged);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnSelectedObjectSaved, UPDAPart*);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnSelectedPointChanged, int, EPartSelectionType);

class FPartEditorViewportClient : public FEditorUtilsViewportClient {
public:
    FPartEditorViewportClient(FPreviewScene& InPreviewScene);

// Override base class functionality

    void SaveActiveAsset();
    virtual void Draw(const FSceneView* View, FPrimitiveDrawInterface* PDI) override;
    virtual bool InputKey(const FInputKeyEventArgs& EventArgs) override;

// External Setters

    void SetDetailsView(TSharedPtr<IDetailsView> InDetailsView);
    void SetInspectedObject(UObject* NewPartObject);
    void SetPartSelectionType(EPartSelectionType Type) { m_SelectedType = Type; }
    void SetSelectedIndex(int32 NewIndex) { m_SelectedIndex = NewIndex; }

// External Getters

    TWeakObjectPtr<UObject> GetInspectedObject() { return m_CurrentInspectedObject; }
    int32 GetSelectedIndex() const { return m_SelectedIndex; }

// Widgets

    virtual UE::Widget::EWidgetMode GetWidgetMode() const override;
    virtual FVector GetWidgetLocation() const override;
    virtual bool CanSetWidgetMode(UE::Widget::EWidgetMode NewMode) const override { return true; }
    virtual bool InputWidgetDelta(FViewport* InViewport, EAxisList::Type CurrentAxis, FVector& Drag, FRotator& Rot, FVector& Scale) override;

// Delegates

    FOnDeselection OnDeslection;
    FOnSelectedObjectChanged OnSelectedObjectChanged;
    FOnSelectedObjectSaved OnSelectedObjectSaved;
    FOnSelectedPointChanged OnSelectedPointChanged;

private:

    bool m_bIsClickingGizmo = false;
    FLinearColor m_BackgroundColour = FLinearColor(0.12f, 0.14f, 0.18f, 1.0f);
    bool m_bDrawAnchors = true;
    bool m_bDrawCollision = true;
    int32 m_SelectedIndex = -1;
    EPartSelectionType m_SelectedType = EPartSelectionType::None;

    TWeakObjectPtr<UObject> m_CurrentInspectedObject;
    TWeakObjectPtr<AActor> m_SpawnedPreviewActor;
    TUniquePtr<FScopedTransaction> m_ScopedDragTransaction;

    const FColor c_UnselectedAnchorColor = FColor(255, 255, 255);
    const FColor c_SelectedAnchorColor = FColor(0, 255, 64);
    const FLinearColor c_CollisionPoint = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    const FLinearColor c_SelectedCollisionPoint = FLinearColor(1.0f, 1.0f, 1.0f, 1.0f);
    const FLinearColor c_BoundingBoxWire = FLinearColor(1.0f, 0.0f, 0.0f, 1.0f);
    const FLinearColor c_BoundingBoxInnards = FLinearColor(1.0f, 0.0f, 0.0f, 0.2f);
    const int c_BoundingBoxWireThickness = 1;
    const float c_AnchorSize = 12.0f;
    const float c_SelectedAnchorSize = c_AnchorSize * 1.5;
    const float c_GridSnap = 0.5f;

private:

    void TrackingStarted(const FInputEventState& InInputState, bool bIsDraggingWidget, bool bNudge) override;
    void TrackingStopped() override;
    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
    void SaveNewAsset(UPDAPart* ActivePart, const FString& ChosenPackagePath, const FString& CurrentName);

};
