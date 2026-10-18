#include "RobotEditor.h"
#include "RobotEditorStyle.h"
#include "RobotEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Viewport/Tabs/RobotEditorTab.h"
#include "Viewport/RobotEditorViewport.h"
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
            ->SetForegroundTab(FTabId(RobotEditorTabName))
        )
    );
}

void FRobotEditorModule::ShutdownModule() {
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FRobotEditorStyle::Shutdown();
	FRobotEditorCommands::Unregister();
    FCoreDelegates::OnPostEngineInit.RemoveAll(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(RobotEditorTabName);
}

void FRobotEditorModule::PluginButtonClicked() {
	FGlobalTabmanager::Get()->TryInvokeTab(RobotEditorTabName);
}

void FRobotEditorModule::RegisterMenus() {
    UToolMenus::UnregisterOwner(this);
	FToolMenuOwnerScoped OwnerScoped(this);

    {
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"); {
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FRobotEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"); {
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools"); {
                FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
                        FName("RobotEditor_ToolbarButton"),
                        FRobotEditorCommands::Get().OpenPluginWindow,
                        INVTEXT("Robot Editor"),
                        INVTEXT("Opens the Robot Editor window"),
                        FSlateIcon(FName("RobotEditorStyle"), FName("RobotEditor.OpenPluginWindow"))
                    );

                Entry.SetCommandList(PluginCommands);
                Section.AddEntry(Entry);
			}
		}
	}
    UToolMenus::Get()->RefreshAllWidgets();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FRobotEditorModule, RobotEditor)
