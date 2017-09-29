#pragma once
#include "SkyUtilities.h"
#include "skse/PapyrusUI.h"
#include "skse/CustomMenu.h"
#include "skse/GlobalLocks.h"
#include "skse/GameBSExtraData.h"
#include "skse/GameExtraData.h"

#include "NativeFunctions.h"

struct TESMoveAttachDetachEvent
{
	Actor* reference;
	UInt32 unk1;
	UInt32 unk2;
	UInt32 unk3;
	UInt32 unk4;
	Actor* referenceA;
	TESObjectCELL* fromCell;
	TESWorldSpace* fromWorldSpace;
	Actor* referenceB;
	UInt32 unk5;
	Actor* referenceC;
	TESObjectCELL* destinationCell;
};

struct TESActivateEvent
{
	struct Event
	{

	};
};

struct TESOpenCloseEvent
{
	UInt32 val;

	struct Event
	{

	};
};

struct TESGenericEvent
{
	TESObjectREFR* target;

	struct Event
	{

	};
};

struct TESLockChangedEvent
{
	TESObjectREFR* target;
	UInt32 unk04;
	UInt32 unk08;
	UInt32 unk0C;
	UInt32 unk10;
	TESObjectREFR* unk14;
	UInt32 unk18;
	UInt32 unk1C;
	UInt32 unk20;
	UInt32 unk24;
	bool isLocked;
};

struct PackageEvent
{
	Actor* source;
};

struct MagicEffectApplyEvent
{
	Actor* source;
	Actor* target;
};

template <>
class BSTEventSink <TESEquipEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESEquipEvent* evn, EventDispatcher<TESEquipEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <TESMoveAttachDetachEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESMoveAttachDetachEvent* evn, EventDispatcher<TESMoveAttachDetachEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <TESActivateEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESActivateEvent* evn, EventDispatcher<TESActivateEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <TESLockChangedEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESLockChangedEvent* evn, EventDispatcher<TESLockChangedEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <PackageEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(PackageEvent* evn, EventDispatcher<PackageEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <MagicEffectApplyEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(MagicEffectApplyEvent* evn, EventDispatcher<MagicEffectApplyEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <TESOpenCloseEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESOpenCloseEvent* evn, EventDispatcher<TESOpenCloseEvent> * dispatcher) = 0;
};

template <>
class BSTEventSink <TESGenericEvent>
{
public:
	virtual ~BSTEventSink() {}
	virtual EventResult ReceiveEvent(TESGenericEvent* evn, EventDispatcher<TESGenericEvent> * dispatcher) = 0;
};

class GameEvents
{
public:
	static int lastNpcListSize;
	static bool hasLocationsLoaded, loadingMenuOpen, mistMenuOpen, firstNpcRequest;
	static BGSLocation *npcLocation;
	static Actor* loadedActor;
	static Timer npcListTimer, regularTimer, animationTimer; //npcListTimer - Determines how long we'll initially be grabbing npcs.

	//For testing startup unknown events.
	class TESGenericEventHandler : public BSTEventSink <TESGenericEvent>
	{
	public:
		int count = 0;
		EventResult	ReceiveEvent(TESGenericEvent * evn, EventDispatcher<TESGenericEvent> * dispatcher)
		{
			count += 1;
			_MESSAGE(to_string(count).c_str());
			return EventResult::kEvent_Continue;
		}
	};

	class TESOpenCloseEventHandler : public BSTEventSink <TESOpenCloseEvent>
	{
	public:
		EventResult	ReceiveEvent(TESOpenCloseEvent * evn, EventDispatcher<TESOpenCloseEvent> * dispatcher)
		{
			SkyUtility::instance->Run();
			return EventResult::kEvent_Continue;
		}
	};

	class TESQuestStageEventHandler : public BSTEventSink <TESQuestStageEvent>
	{
	public:
		EventResult	ReceiveEvent(TESQuestStageEvent * evn, EventDispatcher<TESQuestStageEvent> * dispatcher)
		{
			SkyUtility::instance->Run();
			return EventResult::kEvent_Continue;
		}
	};

	class TESEquipEventHandler : public BSTEventSink <TESEquipEvent>
	{
	public:
		EventResult	ReceiveEvent(TESEquipEvent * evn, EventDispatcher<TESEquipEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected)
				SkyUtility::instance->CheckEquipEvent(*evn);

			return EventResult::kEvent_Continue;
		}
	};

	class LocalActionEventHandler : public BSTEventSink <SKSEActionEvent>
	{
	public:
		EventResult ReceiveEvent(SKSEActionEvent * evn, EventDispatcher<SKSEActionEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected && !GameState::IsMenuOpen)
				SkyUtility::instance->CheckCombatAction(*evn);

			return EventResult::kEvent_Continue;
		}
	};

	class MoveAttachDetachEventHandler : public BSTEventSink <TESMoveAttachDetachEvent>
	{
	public:
		bool tDisabled = false;

		EventResult ReceiveEvent(TESMoveAttachDetachEvent * evn, EventDispatcher<TESMoveAttachDetachEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::PreventingUnauthorizedSpawn(evn->reference))
				return kEvent_Continue;

			NetworkHandler::UpdateLocalNpcList(evn->reference);

			return EventResult::kEvent_Continue;
		}
	};

	class ActivateEventHandler : public BSTEventSink <TESActivateEvent>
	{
	public:
		EventResult ReceiveEvent(TESActivateEvent * evn, EventDispatcher<TESActivateEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected && !GameState::IsMenuOpen)
			{
				TESObjectREFR* refHolder;
				UInt32 handleId = CrosshairRefHandleHolder::GetSingleton()->CrosshairRefHandle();
				LookupREFRByHandle(&handleId, &refHolder);

				if (refHolder && refHolder->GetFormType() != Actor::kTypeID)
				{
					string netData = "button," + Utilities::GetFormIDString(refHolder->formID);
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}
			return EventResult::kEvent_Continue;
		}
	};

	class LockChangedEventHandler : public BSTEventSink <TESLockChangedEvent>
	{
	public:
		EventResult ReceiveEvent(TESLockChangedEvent * evn, EventDispatcher<TESLockChangedEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected)
			{
				if (!evn->target)
					return EventResult::kEvent_Continue;

				if (NetworkHandler::ignoreLock[evn->target->formID] > 0)
				{
					NetworkHandler::ignoreLock[evn->target->formID] -= 1;
					return EventResult::kEvent_Continue;
				}

				LockData tLock = { evn->target->formID, NativeFunctions::IsLocked(evn->target) };

				//If this is already in our list, return. If the lock is in a different state, update it.
				for (vector<LockData>::iterator it = NetworkHandler::lockList.begin(); it != NetworkHandler::lockList.end(); ++it)
				{
					if (it->id == tLock.id)
					{
						if (it->isLocked != tLock.isLocked)
							it->isLocked = tLock.isLocked;

						return EventResult::kEvent_Continue;
					}
				}

				NetworkHandler::lockList.push_back(tLock);
			}
			return EventResult::kEvent_Continue;
		}
	};

	class LocalInputEventHandler : public BSTEventSink <InputEvent>
	{
	public:
		TESObjectREFR *magicTarget;
		UInt32 crosshairHandleId;
		Timer jumpingTimer;

		EventResult	ReceiveEvent(InputEvent ** evn, InputEventDispatcher * dispatcher)
		{
			SkyUtility::instance->Run();

			if (GameState::IsMenuOpen && !IsInMenuMode(GameState::skyrimVMRegistry, 0, nullptr))
				GameState::IsMenuOpen = false;
			else if (!GameState::IsMenuOpen && IsInMenuMode(GameState::skyrimVMRegistry, 0, nullptr))
				GameState::IsMenuOpen = true;

			if (!GameState::IsMenuOpen)
			{
				if (!GameState::IsLoading && (*evn) && (*evn)->eventType == InputEvent::kEventType_Button)
					SkyUtility::OnKeyEvent(((ButtonEvent*)*evn)->keyMask);

				if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected)
				{
					if (jumpingTimer.HasMillisecondsPassed(1250) && SkyUtility::IsEventKey(evn, NativeFunctions::GetMappedKey(nullptr, "Jump", 0xFF)))
					{
						jumpingTimer.StartTimer();
						NetworkHandler::SendData("StartJumping");
					}

					if (SkyUtility::IsEventKey(evn, NativeFunctions::GetMappedKey(nullptr, "Sneak", 0xFF)))
					{
						NetworkHandler::PrintNote("Sneak pressed.");

						if (IsSneaking(GameState::skyrimVMRegistry, 0, *g_thePlayer))
							NetworkHandler::SendData("StartSneaking");
						else
							NetworkHandler::SendData("StopSneaking");
					}

					if (SkyUtility::IsEventKey(evn, NativeFunctions::GetMappedKey(nullptr, "Shout", 0xFF)))
					{
						if (GetEquippedShout(GameState::skyrimVMRegistry, 0, *g_thePlayer) && GetVoiceRecoveryTime(GameState::skyrimVMRegistry, 0, *g_thePlayer) == 0)
							NetworkHandler::SendData((char*)("ShoutRelease," + std::to_string(GameState::plState.fShoutId)).c_str());
					}

					if (SkyUtility::instance->IsPlayerWeaponDrawn() && SkyUtility::IsEventKey(evn, NativeFunctions::GetMappedKey(nullptr, "Left Attack/Block", 0xFF)))
					{
						int EquipType = GetEquippedItemType(GameState::skyrimVMRegistry, 0, *g_thePlayer, 0);
						string netData = "";

						if (EquipType != 0)
						{
							if (EquipType == 10 || EquipType == 5 || EquipType == 6)
							{
								netData = "BlockStart";
								GameState::plState.bIsBlocking = true;
							}
							else if (EquipType == 7 || EquipType == 12)
								netData = "BashAttack";

							NetworkHandler::SendData((char*)netData.c_str());
						}
					}

					else
					{
						if (GameState::plState.bIsBlocking)
						{
							NetworkHandler::SendData("BlockStop");
							GameState::plState.bIsBlocking = false;
						}
					}
				}
			}

			return EventResult::kEvent_Continue;
		}
	};

	class MagicEffectApplyEventHandler : public BSTEventSink <MagicEffectApplyEvent>
	{
	public:
		EventResult ReceiveEvent(MagicEffectApplyEvent * evn, EventDispatcher<MagicEffectApplyEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected)
			{
				if (evn && evn->target && evn->source)
				{
					if (evn->target->formID == (*g_thePlayer)->formID && evn->source->formID == (*g_thePlayer)->formID)
						SkyUtility::instance->selfCasted = true;
				}
			}

			return kEvent_Continue;
		}
	};

	class TESContainerChangedEventHandler : public BSTEventSink <TESContainerChangedEvent>
	{
	public:
		EventResult ReceiveEvent(TESContainerChangedEvent * evn, EventDispatcher<TESContainerChangedEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected && !GameState::IsMenuOpen)
			{
				if (evn && evn->fromFormId == (*g_thePlayer)->formID)
				{
					string netData;
					netData = "dropItem,";
					netData += std::to_string(evn->itemFormId) + ","
						+ std::to_string(evn->count);
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}

			return kEvent_Continue;
		}
	};

	class LocalPackageEventHandler : public BSTEventSink <PackageEvent>
	{
	public:
		tArray<TESObjectCELL*>* cellList = (tArray<TESObjectCELL*>*)0x12E3C64;

		EventResult	ReceiveEvent(PackageEvent * evn, EventDispatcher<PackageEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (firstNpcRequest)
			{
				firstNpcRequest = false;
				npcListTimer.StartTimer();

				//Retrieve all cells, and organize them in a map by location.
				//We will use this to look up npcs, using the player location.
				ExtraLocation* xLocation;
				TESObjectCELL* tCell;
				BSExtraData* tExtra;
				BGSLocationExtended* extLoc;

				if (cellList)
				{
					for (int i = 0; (*cellList)[i]; i++)
					{
						tCell = (*cellList)[i];
						tExtra = tCell->extraData;
						for (; tExtra->next; tExtra = tExtra->next)
						{
							if (tExtra->GetType() == kExtraData_Location)
							{
								xLocation = static_cast<ExtraLocation*>(tExtra);
								NetworkHandler::LocChildCellLookupTable[xLocation->location].push_back((*cellList)[i]);
								extLoc = (BGSLocationExtended*)xLocation->location;
								NetworkHandler::LocChildLocationLookupTable[extLoc->parent].push_back(xLocation->location);
								break;
							}
						}
					}
				}

				hasLocationsLoaded = true;
			}

			if (NetworkHandler::PreventingUnauthorizedSpawn(evn->source))
				return kEvent_Continue;

			NetworkHandler::UpdateLocalNpcList(evn->source);

			return kEvent_Continue;
		}
	};

	class LocalObjectLoadedEventHandler : public BSTEventSink <TESObjectLoadedEvent>
	{
	public:
		Actor* tTarget;
		bool tDisabled = false;

		EventResult ReceiveEvent(TESObjectLoadedEvent * evn, EventDispatcher <TESObjectLoadedEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			tTarget = DYNAMIC_CAST(LookupFormByID(evn->formId), TESForm, Actor);

			if (!tTarget || NetworkHandler::PreventingUnauthorizedSpawn(tTarget))
				return kEvent_Continue;

			if (tTarget->loadedState && NetworkHandler::LocalNpcMap.count(tTarget->formID) == 0)
				NetworkHandler::AddLocalNpcList(tTarget);

			return EventResult::kEvent_Continue;
		}
	};

	//This event is called when the player, or NPC dies.
	class LocalDeathEventHandler : public BSTEventSink <TESDeathEvent>
	{
	public:
		EventResult ReceiveEvent(TESDeathEvent * evn, EventDispatcher <TESDeathEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (evn->source->formID != (*g_thePlayer)->formID)
			{
				if (NetworkHandler::HasInitialized && SkyUtility::instance->complete && NetworkState::bIsConnected && !GameState::IsMenuOpen)
				{
					if (!NetworkHandler::IsLocationMaster())
					{
						Actor* deadNpc = DYNAMIC_CAST(evn->source, TESObjectREFR, Actor);

						if (NetworkHandler::RemoteNpcMap[deadNpc->formID].object && !NetworkHandler::RemoteNpcMap[deadNpc->formID].isDead)
						{
							NetworkHandler::RemoteNpcMap[deadNpc->formID].isDead = true;
							std::string deathFlag = "dFlag," + to_string(NetworkHandler::RemoteNpcMap[deadNpc->formID].formId) + ",1";
							NetworkHandler::SendData((char*)deathFlag.c_str());
						}
					}
				}
			}

			else
				SkyUtility::instance->complete = false;

			return EventResult::kEvent_Continue;
		}
	};

	class LocalMenuEventHandler : public BSTEventSink<MenuOpenCloseEvent>
	{
	public:
		EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, EventDispatcher<MenuOpenCloseEvent> * dispatcher)
		{
			SkyUtility::instance->Run();

			if (!evn)
				return EventResult::kEvent_Continue;

			if (evn->opening)
			{
				if (papyrusUI::IsMenuOpen(nullptr, "Loading Menu") && !loadingMenuOpen)
				{
					loadingMenuOpen = true;
					GameState::IsLoading = true;
					NetworkHandler::ignoreSpawns = true; //Stop disabling new spawns
				}

				if (papyrusUI::IsMenuOpen(nullptr, "Mist Menu") && !mistMenuOpen)
				{
					mistMenuOpen = true;
					GameState::IsLoading = true;
					NetworkHandler::ignoreSpawns = true; //Stop disabling new spawns
				}
			}

			else
			{
				if (loadingMenuOpen && !papyrusUI::IsMenuOpen(nullptr, "Loading Menu"))
					loadingMenuOpen = false;
				else if (mistMenuOpen && !papyrusUI::IsMenuOpen(nullptr, "Mist Menu"))
					mistMenuOpen = false;

				//Only refresh if we've already populated our master list, a loading menu was open, and is now closed.
				if (!loadingMenuOpen && !mistMenuOpen && GameState::IsLoading)
				{
					GameState::IsLoading = false;
					GameState::IsRefreshing = true;
				}
			}

			return EventResult::kEvent_Continue;
		}
	};
};