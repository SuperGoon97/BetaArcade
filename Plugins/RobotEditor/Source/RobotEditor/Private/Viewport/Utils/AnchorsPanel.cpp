#include "Viewport/Utils/AnchorsPanel.h"
#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"
#include "RobotData/PartDat.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Input/SButton.h"

void SAnchorsPanel::Construct(const FArguments& InArgs, TSharedPtr<SPartEditorViewport> InViewportWidget) {
    m_AssociatedViewport = InViewportWidget;

    if (!m_AssociatedViewport.IsValid()) return;

    TSharedPtr<FPartEditorViewportClient> Client = m_AssociatedViewport.Pin()->GetPartClient();
    TSharedPtr<IDetailsView> DetailsView = Client->GetDetailsView();

    Client->OnSelectedObjectChanged.AddRaw(this, &SAnchorsPanel::RefreshAnchorList);
    DetailsView->OnFinishedChangingProperties().AddRaw(this, &SAnchorsPanel::OnInspectorPropertyChanged);

    ChildSlot
    [
        SNew(SVerticalBox)

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(6.0f)
        .HAlign(HAlign_Center)
        [
            SNew(STextBlock)
            .Text(INVTEXT("Part Anchors"))
            .Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
        ]

        + SVerticalBox::Slot()
        .AutoHeight()
        .Padding(4.0f, 2.0f, 4.0f, 6.0f)
        [
            SNew(SBox)
            .HeightOverride(32.0f)
            [
                SNew(SButton)
                .HAlign(HAlign_Center)
                .VAlign(VAlign_Center)
                .OnClicked_Lambda([this]() -> FReply {
                    if (m_AssociatedViewport.IsValid()) {
                        TSharedPtr<FPartEditorViewportClient> Client = m_AssociatedViewport.Pin()->GetPartClient();
                        if (Client.IsValid()) {
                            if (UPDAPart* ActivePart = Cast<UPDAPart>(Client->GetInspectedObject())) {
                                FAnchor NewAnchor;
                                NewAnchor.AnchorName = FString::Printf(TEXT("Anchor_%d"), ActivePart->Anchors.Num());
                                NewAnchor.Position = FVector::ZeroVector;
                                ActivePart->Anchors.Add(NewAnchor);

                                RefreshAnchorList();
                                Client->Invalidate();
                            }
                        }
                    }
                    return FReply::Handled();
                })
                [
                    SNew(STextBlock).Text(INVTEXT("+ Add Anchor"))
                ]
            ]
        ]

        + SVerticalBox::Slot()
        .FillHeight(1.0f)
        [
            SAssignNew(m_ListViewWidget, SListView<FAnchorListEntryPtr>)
            .ListItemsSource(&m_ListDataSource)
            .OnGenerateRow(this, &SAnchorsPanel::OnGenerateAnchorRow)
            .OnSelectionChanged(this, &SAnchorsPanel::OnAnchorSelectionChanged)
        ]
    ];

    RefreshAnchorList();
}

void SAnchorsPanel::RefreshAnchorList() {
    m_ListDataSource.Empty();

    if (m_AssociatedViewport.IsValid()) {
        TSharedPtr<FPartEditorViewportClient> ClientPtr = m_AssociatedViewport.Pin()->GetPartClient();

        UPDAPart* ActivePart = Cast<UPDAPart>(ClientPtr->GetInspectedObject());
        if (ActivePart) {
            for (const FAnchor& Anchor : ActivePart->Anchors) {
                m_ListDataSource.Add(MakeShareable(new FString(Anchor.AnchorName)));
            }
        } 
    }

    if (m_ListViewWidget.IsValid()) {
        m_ListViewWidget->RequestListRefresh();
    }
}

TSharedRef<ITableRow> SAnchorsPanel::OnGenerateAnchorRow(FAnchorListEntryPtr Item, const TSharedRef<STableViewBase>& OwnerTable) {
    return SNew(STableRow<FAnchorListEntryPtr>, OwnerTable)
        .Padding(FMargin(12.0f, 4.0f))
        [
            SNew(STextBlock)
            .Text(FText::FromString(*Item.Get()))
        ];
}

void SAnchorsPanel::OnAnchorSelectionChanged(FAnchorListEntryPtr Selection, ESelectInfo::Type SelectInfo) {
    if (!m_AssociatedViewport.IsValid()) return;
    TSharedPtr<FPartEditorViewportClient> ClientPtr = m_AssociatedViewport.Pin()->GetPartClient();

    if (!Selection.IsValid()) {
        ClientPtr->SetSelectedAnchorIndex(INDEX_NONE);
        ClientPtr->Invalidate();
        return;
    }

    if (ClientPtr.IsValid()) {
        int32 FoundIndex = m_ListDataSource.Find(Selection);
        if (FoundIndex != INDEX_NONE) {
            ClientPtr->SetSelectedAnchorIndex(FoundIndex);
            ClientPtr->Invalidate();
        }
    }
}

void SAnchorsPanel::OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent) {
    RefreshAnchorList();
}
