#include "Framework/Commands/UICommandInfo.h"
#include "Components/SkyLightComponent.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "SViewportToolBar.h"
#include "Widgets/SViewport.h"

#include "Viewport/EditorViewport.h"
#include "Viewport/EditorViewportClient.h"

void SEditorUtilsViewport::Construct(const FArguments& InArgs) {
    m_PreviewScene = MakeUnique<FPreviewScene>();
    SEditorViewport::Construct(SEditorViewport::FArguments());
}

TSharedPtr<SWidget> SEditorUtilsViewport::BuildViewportToolbar() {
    return SNew(SViewportToolBar);
}
