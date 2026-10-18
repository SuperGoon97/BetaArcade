#include "PartEditor.h"
#include "PartEditorStyle.h"
#include "PartEditorCommands.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Viewport/Tabs/PartEditorTab.h"
#include "Viewport/PartEditorViewport.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FPartEditorModule"

void FPartEditorModule::StartupModule() {
	FPartEditorStyle::Initialize();
	FPartEditorStyle::ReloadTextures();

	FPartEditorCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FPartEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FPartEditorModule::PluginButtonClicked),
		FCanExecuteAction());

    UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FPartEditorModule::RegisterMenus));

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(PartEditorTabName, FOnSpawnTab::CreateStatic(&SPartEditorTab::MakeTab))
        .SetDisplayName(INVTEXT("Part Editor"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);

    FCoreDelegates::OnPostEngineInit.AddRaw(this, &FPartEditorModule::OnEngineInitialized);
}

void FPartEditorModule::OnEngineInitialized() {
    TSharedRef<FTabManager::FLayout> CustomEditorLayout = FTabManager::NewLayout(EditorUnifiedWindowLayoutVer)
    ->AddArea(
        FTabManager::NewPrimaryArea()
        ->SetOrientation(Orient_Horizontal)
        ->Split(
            FTabManager::NewStack()
            ->AddTab(PartEditorTabName, ETabState::OpenedTab)
            ->SetForegroundTab(FTabId(PartEditorTabName))
        )
    );
}

void FPartEditorModule::ShutdownModule() {
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);
	FPartEditorStyle::Shutdown();
	FPartEditorCommands::Unregister();
    FCoreDelegates::OnPostEngineInit.RemoveAll(this);
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(PartEditorTabName);
}

void FPartEditorModule::PluginButtonClicked() {
	FGlobalTabmanager::Get()->TryInvokeTab(PartEditorTabName);
}

void FPartEditorModule::RegisterMenus() {
    UToolMenus::UnregisterOwner(this);
	FToolMenuOwnerScoped OwnerScoped(this);

    {
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window"); {
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FPartEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar"); {
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools"); {
                FToolMenuEntry Entry = FToolMenuEntry::InitMenuEntry(
                        FName("PartEditor_ToolbarButton"),
                        FPartEditorCommands::Get().OpenPluginWindow,
                        INVTEXT("Part Editor"),
                        INVTEXT("Opens the Part Editor window"),
                        FSlateIcon(FName("PartEditorStyle"), FName("PartEditor.OpenPluginWindow"))
                    );

                Entry.SetCommandList(PluginCommands);
                Section.AddEntry(Entry);
			}
		}
	}
    UToolMenus::Get()->RefreshAllWidgets();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPartEditorModule, PartEditor)
