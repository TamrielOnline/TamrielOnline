#include "IsolatedFunctions.h"
#include "skse/skse_version.h"	// What version of SKSE is running?
#include <shlobj.h>				// CSIDL_MYCODUMENTS
#include "skse/PluginAPI.h"		// super
#include "GameEvents.h"
#include "skse/SafeWrite.h"

int __cdecl _purecall(void)
{
	return 1;
}

static SKSEPapyrusInterface* g_papyrus = nullptr;
static SKSEMessagingInterface* g_messaging;
static SKSEScaleformInterface* g_scaleform;
static PluginHandle g_pluginHandle = kPluginHandle_Invalid;

DWORD WINAPI InitializeConsole(LPVOID lpParam)
{
	// Maximum time to wait in milliseconds before giving up.
	// Usually console should be initialized within 20 seconds even on the slowest of computers.
	const int MaxWaitTime = 60000;

	int isGuiInitialized = 0;
	int waited = 0;
	while (waited < MaxWaitTime)
	{
		// All these are needed to exist.
		int guiManager = *((int*)0x12E35E4);
		int guiStringHolder = *((int*)0x12E35E0);
		int plrChar = *((int*)0x1B2E8E4);

		if (guiManager != 0 && guiStringHolder != 0 && plrChar != 0)
			isGuiInitialized++;

		// Wait a little bit because having a pointer does not mean it's fully initialized yet.
		if (isGuiInitialized >= 5)
		{
			// Toggle console twice to initialize it.
			const int toggleConsole = 0x847210;

			// Second one must be close because it happens on another thread and toggle twice fast would open twice
			// since console didn't have time to open yet.
			const int closeConsole = 0x847130;
			_asm
			{
				pushad
				pushfd
				call toggleConsole
				call closeConsole
				popfd
				popad
			}

			// Don't need this thread anymore.
			return 0;
		}

		// Wait until trying again.
		Sleep(50);
		waited += 50;
	}

	// Write error.
	_MESSAGE("Console initialize timed out.");
	return 0;
}

GameEvents::LocalActionEventHandler t_actionEventHandler;
GameEvents::TESEquipEventHandler t_equipEventHandler;
GameEvents::LocalPackageEventHandler t_packageEventHandler;
GameEvents::LocalObjectLoadedEventHandler t_loadedEventHandler;
GameEvents::LocalDeathEventHandler t_deathEventHandler;
GameEvents::LocalInputEventHandler t_inputEventHandler;
GameEvents::LockChangedEventHandler t_lockChangedHandler;
GameEvents::MoveAttachDetachEventHandler t_actorLocationChangedHandler;
GameEvents::LocalMenuEventHandler t_menuEventHandler;
GameEvents::MagicEffectApplyEventHandler t_magicEffectApplyEventHandler;
GameEvents::TESContainerChangedEventHandler t_containerChangedEventHandler;
GameEvents::ActivateEventHandler t_activateEventHandler;
GameEvents::TESGenericEventHandler t_genericEventHandler;
GameEvents::TESQuestStageEventHandler t_questStageEventHandler;
GameEvents::TESOpenCloseEventHandler t_openCloseEventHandler;

TODispatcher<TESGenericEvent>* g_genericEvent = (TODispatcher<TESGenericEvent>*)0x012E5290;
TODispatcher<TESOpenCloseEvent>* g_openCloseEventDispatcher = (TODispatcher<TESOpenCloseEvent>*)0x012E5110;
TODispatcher<MenuOpenCloseEvent>* g_menuOpenCloseEventDispatcher = (TODispatcher<MenuOpenCloseEvent>*)0x01B397D0;
TODispatcher<TESEquipEvent>* g_equipEventDispatcher = (TODispatcher<TESEquipEvent>*)0x012E4EA0;
TODispatcher<TESMoveAttachDetachEvent>* g_moveAttachDetachEvent = (TODispatcher<TESMoveAttachDetachEvent>*)0x012E5080;
TODispatcher<TESActivateEvent>* g_activateEventDispatcher = (TODispatcher<TESActivateEvent>*)0x012E3E60;
TODispatcher<TESLockChangedEvent>* g_lockChangedEventDispatcher = (TODispatcher<TESLockChangedEvent>*)0x012E4FF0;
TODispatcher<PackageEvent>* g_packageEventDispatcher = (TODispatcher<PackageEvent>*)0x012E5140;
TODispatcher<MagicEffectApplyEvent>* g_magicEffectApplyEventDispatcher = (TODispatcher<MagicEffectApplyEvent>*)0x012E5020;
TODispatcher<TESQuestStageEvent>* tg_questStageEventDispatcher = (TODispatcher<TESQuestStageEvent>*)0x012E51D0;
TODispatcher<TESContainerChangedEvent>* tg_containerChangedEventDispatcher = (TODispatcher<TESContainerChangedEvent>*)0x012E4DE0;
TODispatcher<TESDeathEvent>* tg_deathEventDispatcher = (TODispatcher<TESDeathEvent>*)0x012E4E10;
TODispatcher<TESObjectLoadedEvent>* tg_objectLoadedEventDispatcher = (TODispatcher<TESObjectLoadedEvent>*)0x012E50B0;
TOInputEventDispatcher** g1_inputEventDispatcher = (TOInputEventDispatcher **)0x01B2E724;
TODispatcher<MenuOpenCloseEvent>* tMenuOpenCloseEventDispatcher = nullptr;
TODispatcher<SKSEActionEvent>* actionEventDispatcher = nullptr;

bool consoleSuccess = false;
map<string, bool> enabledDispatcher = map<string, bool>();

void StartHooks()
{
	if (!GameState::skyrimVMRegistry)
		GameState::skyrimVMRegistry = (*g_skyrimVM)->GetClassRegistry();

	if (!actionEventDispatcher)
		actionEventDispatcher = (TODispatcher<SKSEActionEvent>*)g_messaging->GetEventDispatcher(SKSEMessagingInterface::kDispatcher_ActionEvent);

	if (actionEventDispatcher && !enabledDispatcher["Action"])
		enabledDispatcher["Action"] = actionEventDispatcher->Try_AddEventSink(&t_actionEventHandler);

	if (!tMenuOpenCloseEventDispatcher)
		tMenuOpenCloseEventDispatcher = (TODispatcher<MenuOpenCloseEvent>*)MenuManager::GetSingleton()->MenuOpenCloseEventDispatcher();

	if (tMenuOpenCloseEventDispatcher && !enabledDispatcher["Menu"])
		enabledDispatcher["Menu"] = tMenuOpenCloseEventDispatcher->Try_AddEventSink(&t_menuEventHandler);

	if (!enabledDispatcher["Equip"])
		enabledDispatcher["Equip"] = g_equipEventDispatcher->Try_AddEventSink(&t_equipEventHandler);

	if (!enabledDispatcher["Package"])
		enabledDispatcher["Package"] = g_packageEventDispatcher->Try_AddEventSink(&t_packageEventHandler);

	if (!enabledDispatcher["Loaded"])
		enabledDispatcher["Loaded"] = tg_objectLoadedEventDispatcher->Try_AddEventSink(&t_loadedEventHandler);

	if (!enabledDispatcher["Death"])
		enabledDispatcher["Death"] = tg_deathEventDispatcher->Try_AddEventSink(&t_deathEventHandler);

	if (!enabledDispatcher["Lock"])
		enabledDispatcher["Lock"] = g_lockChangedEventDispatcher->Try_AddEventSink(&t_lockChangedHandler);

	if (!enabledDispatcher["Location"])
		enabledDispatcher["Location"] = g_moveAttachDetachEvent->Try_AddEventSink(&t_actorLocationChangedHandler);

	if (!enabledDispatcher["MagicEffect"])
		enabledDispatcher["MagicEffect"] = g_magicEffectApplyEventDispatcher->Try_AddEventSink(&t_magicEffectApplyEventHandler);

	if (!enabledDispatcher["Container"])
		enabledDispatcher["Container"] = tg_containerChangedEventDispatcher->Try_AddEventSink(&t_containerChangedEventHandler);

	if (!enabledDispatcher["Activate"])
		enabledDispatcher["Activate"] = g_activateEventDispatcher->Try_AddEventSink(&t_activateEventHandler);

	if (!enabledDispatcher["Quest"])
		enabledDispatcher["Quest"] = tg_questStageEventDispatcher->Try_AddEventSink(&t_questStageEventHandler);

	if (!enabledDispatcher["OpenClose"])
		enabledDispatcher["OpenClose"] = g_openCloseEventDispatcher->Try_AddEventSink(&t_openCloseEventHandler);

	//if (!enabledDispatcher["Generic"])
	//	enabledDispatcher["Generic"] = g_genericEvent->Try_AddEventSink(&t_genericEventHandler);

	if (!enabledDispatcher["Input"])
		enabledDispatcher["Input"] = (*g1_inputEventDispatcher)->Try_AddEventSink(&t_inputEventHandler);

	if (!consoleSuccess)
	{
		// Console must be initialized before we can use it. Open it and close immediately, however
		// UI must be initialized first so run another thread that waits until UI is initialized.
		DWORD threadId = 0, runId = 0;
		if (CreateThread(nullptr, 0, InitializeConsole, nullptr, 0, &threadId) == nullptr)
		{
			consoleSuccess = false;
			_MESSAGE("Failed to start console initialize thread!");
		}
		else
			consoleSuccess = true;
	}
}

void SKSEMessageReceptor(SKSEMessagingInterface::Message* msg)
{
	StartHooks();
	if (msg->type == SKSEMessagingInterface::kMessage_PostLoadGame)
	{
		//((EventDispatcher<TESGenericEvent>*)&(*g_thePlayer)->positionPlayerEventSource)->AddEventSink(&t_genericEventHandler);
	}
}

extern "C"
{
	bool SKSEPlugin_Query(const SKSEInterface* skse, PluginInfo* info)
	{ // Called by SKSE to learn about this plugin and check that it's safe to load it
		gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Skyrim\\SKSE\\SkyUtilities.log");
		gLog.SetPrintLevel(IDebugLog::kLevel_Error);
		gLog.SetLogLevel(IDebugLog::kLevel_DebugMessage);

		_MESSAGE("SkyUtilities");

		// populate info structure
		info->infoVersion = PluginInfo::kInfoVersion;
		info->name = "SkyUtilitiesScript";
		info->version = 1;

		// store plugin handle so we can identify ourselves later
		g_pluginHandle = skse->GetPluginHandle();

		if (skse->isEditor)
		{
			_MESSAGE("loaded in editor, marking as incompatible");

			return false;
		}
		if (skse->runtimeVersion != RUNTIME_VERSION_1_9_32_0)
		{
			_MESSAGE("unsupported runtime version %08X", skse->runtimeVersion);

			return false;
		}

		// ### do not do anything else in this callback
		// ### only fill out PluginInfo and return true/false

		// supported runtime version
		return true;
	}

	bool SKSEPlugin_Load(const SKSEInterface* skse)
	{ // Called by SKSE to load this plugin
		_MESSAGE("SkyUtilitiesPlugin loaded");

		g_papyrus = (SKSEPapyrusInterface *)skse->QueryInterface(kInterface_Papyrus);
		g_messaging = (SKSEMessagingInterface *)skse->QueryInterface(kInterface_Messaging);
		g_scaleform = (SKSEScaleformInterface *)skse->QueryInterface(kInterface_Scaleform);

		SkyUtility::instance = std::make_shared<SkyUtility>();
		Networking::appId = JString(Utilities::GetIniVar(CONFIG_FILE, "LanSettings", "appid").c_str());
		Networking::instance = make_shared<Networking>();
		bool btest = g_papyrus->Register(SkyUtility::RegisterFuncs); //Check if the function registration was a success...
		bool ctest = g_papyrus->Register(IsolatedFunctions::RegisterFuncs);

		if (btest && ctest)
			_MESSAGE("Register Succeeded");

		//Register callback for SKSE messaging interface
		g_messaging->RegisterListener(g_pluginHandle, "SKSE", SKSEMessageReceptor);
		return true;
	}
};
