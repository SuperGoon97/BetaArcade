#include "RobotEditor.h"
#include "RobotEditorStyle.h"
#include "RobotEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Viewport/Tabs/RobotEditorTab.h"
#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/RobotEditorViewport.h"
#include "Viewport/PartEditorViewport.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FRobotEditorModule"

void FRobotEditorModule::StartupModule() {
	FRobotEditorStyle::Initialize();
	FRobotEditorStyle::ReloadTextures();

	FRobotEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FRobotEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FRobotEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FRobotEditorModule::RegisterMenus));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(RobotEditorTabName, FOnSpawnTab::CreateStatic(&SRobotEditorTab::MakeTab))
        .SetDisplayName(INVTEXT("Robot Editor"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);
    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PartEditorTabName, FOnSpawnTab::CreateStatic(&SPartEditorTab::MakeTab))
        .SetDisplayName(INVTEXT("Part Editor"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);

    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FRobotEditorModule::OnEngineInitialized);
}

void FRobotEditorModule::OnEngineInitialized() {
    TSharedRef<FTabManager::FLayout> CustomEditorLayout = FTabManager::NewLayout(EditorUnifiedWindowLayoutVer)
    ->AddArea(
        FTabManager::NewPrimaryArea()
        ->SetOrientation(Orient_Horizontal)
        ->Split(
            FTabManager::NewStack()
            ->AddTab(RobotEditorTabName, ETabState::OpenedTab)
            ->AddTab(PartEditorTabName, ETabState::OpenedTab)
            ->SetForegroundTab(FTabId(PartEditorTabName))
        )
    );

    TSharedPtr<SWindow> RootWindow = FGlobalTabmanager::Get()->GetRootWindow();
    FGlobalTabmanager::Get()->RestoreFrom(CustomEditorLayout, RootWindow);
}

void FRobotEditorModule::ShutdownModule() {
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FRobotEditorStyle::Shutdown();
	FRobotEditorCommands::Unregister();
    FCoreDelegates::OnPostEngineInit.RemoveAll(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RobotEditorTabName);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PartEditorTabName);
}

void FRobotEditorModule::PluginButtonClicked() {
	FGlobalTabmanager::Get()->TryInvokeTab(RobotEditorTabName);
    FGlobalTabmanager::Get()->TryInvokeTab(PartEditorTabName);
}

void FRobotEditorModule::RegisterMenus() {
	FToolMenuOwnerScoped OwnerScoped(this);

    {
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
        {
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRobotEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FRobotEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRobotEditorModule, RobotEditor)
