#pragma once

#include "NativeFunctions.h"

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

	static bool isTesting = false;
	static bool isDebugging = false;
	static void (*ActiveTest)() = EnableStateTest; // What test are we running?

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