#include "Viewport/RobotEditorViewportClient.h"
#include "Data/PartDat.h"

FRobotEditorViewportClient::FRobotEditorViewportClient(FPreviewScene& InPreviewScene)
    : FEditorUtilsViewportClient(InPreviewScene) {
        return;
}

void FRobotEditorViewportClient::Tick(float DeltaSeconds) {
    FEditorUtilsViewportClient::Tick(DeltaSeconds);

    // TODO : Perform a cast from the camera's location to the mouse and move the preview actor to that position.
}

bool FRobotEditorViewportClient::InputKey(const FInputKeyEventArgs& EventArgs) {
    if (EventArgs.Key == EKeys::LeftMouseButton && EventArgs.Event == IE_Pressed) {
        return true;
    }

    return FEditorUtilsViewportClient::InputKey(EventArgs);
}
