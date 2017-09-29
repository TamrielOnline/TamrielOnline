#pragma once

#include "NativeFunctions.h"
#include <thread>

using namespace ExitGames::Common;

namespace Tests
{
	static Actor* testPlayer;
	static Actor* sTestPlayer;
	static tArray<UInt32> hostileList;

	static void LoadTest()
	{
		if (!testPlayer)
			testPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);

		if (testPlayer->GetNiNode())
			_MESSAGE("a They exist");
		else
		{
			_MESSAGE("a They dont exist");
			return;
		}

		DisableNoWait((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, false);

		if (testPlayer->GetNiNode())
			_MESSAGE("b They exist");
		else
			_MESSAGE("b They dont exist");
	}

	static void DeadTest()
	{
		if (!testPlayer)
			testPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);

		if (!testPlayer->IsDead(1))
		{
			_MESSAGE("a Alive");
		}
		else
		{
			_MESSAGE("a Dead");
			Resurrect((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer);
			return;
		}

		KillSilent((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, testPlayer);

		if (!testPlayer->IsDead(1))
			_MESSAGE("b Alive");
		else
			_MESSAGE("b Dead");
	}

	static void HostileTest()
	{
		if (!testPlayer)
			testPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);

		if (!sTestPlayer)
		{
			sTestPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);
			StartCombat((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, sTestPlayer);
			StartCombat((*g_skyrimVM)->GetClassRegistry(), 0, sTestPlayer, testPlayer);
		}

		hostileList = (*g_thePlayer)->hostileHandles;

		TESObjectREFR* tempHostile;
		for (int i = 0; i < hostileList.count; i++)
		{
			LookupREFRByHandle(&hostileList[i], &tempHostile);
			_MESSAGE(to_string(tempHostile->formID).c_str());
		}
	}

	static void UnconsciousTest()
	{
		if (!testPlayer)
			testPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);

		if (true)
		{
			_MESSAGE("I am unconscious");
			SetUnconscious((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, false);
		}
		else
		{
			_MESSAGE("I am conscious");
			SetUnconscious((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, true);
		}
	}

	static void TimeTest()
	{
		_MESSAGE("Here it is.");
		_MESSAGE(to_string(GetValue((*g_skyrimVM)->GetClassRegistry(), 23000, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDaysPassed))).c_str());
		_MESSAGE(to_string(GetValue((*g_skyrimVM)->GetClassRegistry(), 23000, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDay))).c_str());
		_MESSAGE(to_string(GetValue((*g_skyrimVM)->GetClassRegistry(), 23000, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameHour))).c_str());
		_MESSAGE(to_string(GetValue((*g_skyrimVM)->GetClassRegistry(), 23000, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameMonth))).c_str());
		_MESSAGE(to_string(GetValue((*g_skyrimVM)->GetClassRegistry(), 23000, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameYear))).c_str());
	}

	static void EnableStateTest()
	{
		if (!testPlayer)
			testPlayer = (Actor*)PlaceAtMe_Native((*g_skyrimVM)->GetClassRegistry(), 0, *g_thePlayer, LookupFormByID(0x00079F66), 1, true, false);

		if (IsDisabled((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer))
		{
			_MESSAGE("So we're not good.");
			return;
		}

		_MESSAGE("So we're good.");

		DisableNoWait((*g_skyrimVM)->GetClassRegistry(), 0, testPlayer, false);
	}

	static void CacheTest()
	{
		if (!NetworkState::bIsConnected)
			return;

		ExitGames::Common::Hashtable testTable = ExitGames::Common::Hashtable();
		
		testTable.put<ExitGames::Common::JString>("This is a test.");

		Networking::instance->sendEventCached<ExitGames::Common::Hashtable>(true, testTable, NetworkState::EV::ID_DEBUG, NetworkState::CHANNEL::EVENT);
	}

	static void ThreadTest()
	{
		_MESSAGE((char*)&std::this_thread::get_id());
	}

	static void LocationTest()
	{
		static Utilities::Timer locationTimer;

		if (locationTimer.HasMillisecondsPassed(2000))
		{
			locationTimer.StartTimer();

			TESWorldSpace* tWorldspace = CALL_MEMBER_FN(*g_thePlayer, GetWorldspace)();
			TESWorldSpace* altWorldspace = NULL;
			TESWorldSpace* reWorldspace = NULL;
			TESObjectCELL* altCell = (*g_thePlayer)->parentCell;
			Actor* randomActor = NULL;
			TESObjectREFR* randomObject = NULL;

			_MESSAGE("altCell Members");

			// "Fully" representative of the interior cell.
			if (altCell)
			{
				Actor* aRef = NULL;
				TESObjectREFR* instanceRef = NULL;
				for (int i = 0; altCell->objectList.GetNthItem(i, instanceRef); i++)
				{
					if (!instanceRef || instanceRef->formID == (*g_thePlayer)->formID)
						continue;

					aRef = DYNAMIC_CAST(instanceRef, TESObjectREFR, Actor);

					if (aRef)
					{
						if (aRef->loadedState)
						{
							_MESSAGE(to_string(aRef->formID).c_str());
							_MESSAGE(CALL_MEMBER_FN(aRef, GetReferenceName)());
							randomActor = aRef;
						}
					}
					else
						randomObject = instanceRef;
				}
			}

			if (randomActor)
			{
				ExtraLocation* eLocation = static_cast<ExtraLocation*>(randomActor->extraData.GetByType(kExtraData_Location));
				BGSLocation* tLocation = (eLocation ? eLocation->location : NULL);
				ExtraLocation* rLocation = static_cast<ExtraLocation*>(randomObject->extraData.GetByType(kExtraData_Location));
				BGSLocation* reLocation = (rLocation ? rLocation->location : NULL);
				ExtraLocation* pLocation = static_cast<ExtraLocation*>((*g_thePlayer)->extraData.GetByType(kExtraData_Location));
				BGSLocation* plLocation = (pLocation ? pLocation->location : NULL);
				ExtraPersistentCell* eCell = static_cast<ExtraPersistentCell*>(randomActor->extraData.GetByType(kExtraData_PersistentCell));
				TESObjectCELL* tCell = (eCell ? eCell->cell : NULL);
				ExtraPersistentCell* iCell = static_cast<ExtraPersistentCell*>(((BaseExtraList*)altCell->extraData)->GetByType(kExtraData_PersistentCell));
				TESObjectCELL* ieCell = (iCell ? iCell->cell : NULL);
				ExtraPersistentCell* rCell = static_cast<ExtraPersistentCell*>(randomObject->extraData.GetByType(kExtraData_PersistentCell));
				TESObjectCELL* reCell = (rCell ? rCell->cell : NULL);

				_MESSAGE("tCell Members");

				// "Fully" representative of exterior cell.
				if (tCell)
				{
					Actor* aRef = NULL;
					TESObjectREFR* instanceRef = NULL;
					for (int i = 0; tCell->objectList.GetNthItem(i, instanceRef); i++)
					{
						if (!instanceRef || instanceRef->formID == (*g_thePlayer)->formID)
							continue;

						aRef = DYNAMIC_CAST(instanceRef, TESObjectREFR, Actor);

						if (aRef && aRef->loadedState)
						{
							_MESSAGE(to_string(aRef->formID).c_str());
							_MESSAGE(CALL_MEMBER_FN(aRef, GetReferenceName)());
						}
					}
				}

				_MESSAGE("reCell Members");

				// Also "fully" representative of exterior cell.
				if (reCell)
				{
					Actor* aRef = NULL;
					TESObjectREFR* instanceRef = NULL;
					for (int i = 0; reCell->objectList.GetNthItem(i, instanceRef); i++)
					{
						if (!instanceRef || instanceRef->formID == (*g_thePlayer)->formID)
							continue;

						aRef = DYNAMIC_CAST(instanceRef, TESObjectREFR, Actor);

						if (aRef && aRef->loadedState)
						{
							_MESSAGE(to_string(aRef->formID).c_str());
							_MESSAGE(CALL_MEMBER_FN(aRef, GetReferenceName)());
						}
					}
				}

				_MESSAGE("tLocation");
				if (tLocation)
					_MESSAGE(to_string(tLocation->formID).c_str());
				_MESSAGE(to_string(GetCurrentLocation(GameState::skyrimVMRegistry, 0, *g_thePlayer)->formID).c_str());

				// Not equivalent to any of the former.
				_MESSAGE("reLocation");
				if (reLocation)
					_MESSAGE(to_string(reLocation->formID).c_str());

				// Not equivalent to any of the former.
				_MESSAGE("plLocation");
				if (plLocation)
					_MESSAGE(to_string(plLocation->formID).c_str());

				_MESSAGE("tWorldspace");
				if (tWorldspace)
					_MESSAGE(to_string(tWorldspace->formID).c_str());

				_MESSAGE("player Worldspace");

				if (GetWorldSpace(GameState::skyrimVMRegistry, 0, *g_thePlayer))
					_MESSAGE(to_string(GetWorldSpace(GameState::skyrimVMRegistry, 0, *g_thePlayer)->formID).c_str());

				_MESSAGE("altWorldspace");

				altWorldspace = CALL_MEMBER_FN(randomActor, GetWorldspace)();
				if (altWorldspace)
					_MESSAGE(to_string(altWorldspace->formID).c_str());

				_MESSAGE("reWorldspace");

				reWorldspace = CALL_MEMBER_FN(randomObject, GetWorldspace)();
				if (reWorldspace)
					_MESSAGE(to_string(reWorldspace->formID).c_str());

				_MESSAGE("altCell Worldspace");

				// All prior worldspaces are equivalent except for this one, and they are all NULL when a location is loading.
				if (altCell->unk84)
					_MESSAGE(to_string(altCell->unk84->formID).c_str());

				_MESSAGE("tCell");

				// Not equivalent
				if (tCell)
					_MESSAGE(to_string(tCell->formID).c_str());
				_MESSAGE(to_string((*g_thePlayer)->parentCell->formID).c_str());

				_MESSAGE("ieCell");

				if (ieCell)
					_MESSAGE(to_string(ieCell->formID).c_str());
			}
		}
	}

	static bool isTesting = false;
	static bool isDebugging = false;
	static void (*ActiveTest)() = LocationTest; // What test are we running?

	static void RunTests()
	{
		if (!isTesting)
			return;

		ActiveTest();
	}

	static void debug(string str)
	{
		if (!isDebugging)
			return;

		_MESSAGE(str.c_str());
	}
}