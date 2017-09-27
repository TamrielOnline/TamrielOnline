#pragma once

#include "NetworkHandler.h"
#include "common/IMemPool.h"
#include "skse/GameThreads.h"

struct TESEquipEvent
{
	Actor* actor; // 00
	UInt32 equippedFormID; // 04
	UInt32 unk08; // 08  (always 0)  specific ObjectReference FormID if item has one?
	UInt16 unk0C; // 0C  (always 0)
	bool isEquipping; // 0E
	// more?
};

class SkyUtility
{
private:
	map<string, vector<string> >* consoleMap;
	string lastCommandEntered { "" }, tempLoc { "" };
	TESQuest* mainQuest { nullptr };
	bool completeInitA { false }, completeInitB { false };
	bool InitialPosition { true }, bowStart { false };
	BYTE clientKey;
	TESObjectREFR* refHolder { nullptr };
	Timer periodicTimeCheckUtilities, positionTimer, inactivityTimer, connectTimer, databaseTimer, npcTimer, questTimer, cellTimer, lockTimer;
	int newX, newY, newZ, rNewX, rNewY, rNewZ, activationId;

public:
	SkyUtility() = default;

	static std::shared_ptr<SkyUtility> instance;
	bool complete { false }, selfCasted { false };

	static bool RegisterFuncs(VMClassRegistry* registry);
	static void SetINIFloatEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection,
		BSFixedString iniVariable, float value);
	static void SetINIBoolEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection,
		BSFixedString iniVariable, bool value);
	static void SetINIIntEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection,
		BSFixedString iniVariable, UInt32 value);
	static void SetINIStringEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection,
		BSFixedString iniVariable, BSFixedString value);
	static void PlayerKOFA(StaticFunctionTag* base, Actor* target, UInt32 positionSelection);
	static void EquipItem(StaticFunctionTag* base, Actor* ref, TESForm* item, UInt32 slot);
	static void SetTransform(StaticFunctionTag* base, BSFixedString ref, UInt32 x, UInt32 y, UInt32 z, UInt32 xRot, UInt32 yRot, UInt32 zRot);
	static void EnableNetPlayer(StaticFunctionTag* base, Actor* target);
	static void DisableNet(StaticFunctionTag* base, Actor* target);
	static void OnConnected();
	static void OnDisconnected();
	static void ReceiveEvent(const int playerNr, const nByte eventCode, const Object& eventContent);

	bool IsKeyPressed(const char* localKey);
	bool IsEventKey(InputEvent** evn, UInt32 keyValue);
	bool IsPlayerWeaponDrawn();
	bool ArmorCheck(UInt32 armorFormId, UInt32* idRef, const char* tName);
	bool ArmorCheck(UInt32 armorFormId, UInt32* idRef, bool isEquipping, UInt32 itemId, const char* tName);
	bool IsInGame();

	UInt32 GetStartupKey();

	void CheckEquipEvent(TESEquipEvent e);
	void CheckCombatAction(SKSEActionEvent e);
	void SetupCallbacks();
	void UpdateCheck();
	void RefreshLocation();
	void Connect();
	void GetInitialPlayerData(bool response = false);
	void Run();
	void ConnectionStabilizer();
};
