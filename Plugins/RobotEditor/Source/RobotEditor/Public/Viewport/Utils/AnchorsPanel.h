#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Views/SListView.h"

typedef TSharedPtr<FString> FAnchorListEntryPtr;

class SAnchorsPanel : public SCompoundWidget {
public:
    SLATE_BEGIN_ARGS(SAnchorsPanel) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs, TSharedPtr<class SPartEditorViewport> InViewportWidget);
    void RefreshAnchorList();

private:

    TSharedRef<ITableRow> OnGenerateAnchorRow(FAnchorListEntryPtr Item, const TSharedRef<STableViewBase>& OwnerTable);
    void OnInspectorPropertyChanged(const FPropertyChangedEvent& PropertyChangedEvent);
    void OnAnchorSelectionChanged(FAnchorListEntryPtr Selection, ESelectInfo::Type SelectInfo);

    TWeakPtr<class SPartEditorViewport> m_AssociatedViewport;
    TSharedPtr<SListView<FAnchorListEntryPtr>> m_ListViewWidget;
    TArray<FAnchorListEntryPtr> m_ListDataSource;
};
