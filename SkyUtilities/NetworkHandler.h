#ifndef __NetworkHandler_H
#define __NetworkHandler_H

#include "Networking.h"
#include "Utilities.h"
#include "enums.h"

#include "NetworkState.h"
#include "GameState.h"
#include "Tests.h"

#include <exception>
#include <atlstr.h>
#include <thread>

#include <cstdlib>
#include <queue>
#include "NativeFunctions.h"
#include <mutex>

#define _USE_MATH_DEFINES
#include <math.h>

using namespace std;
using namespace Utilities;
using namespace ExitGames;
using namespace ExitGames::Common;

/* Used to convert mod id's (0xFF000000) between players. Players will have different load orders.
The same references will have different mod ids, this maps them between players. */
static map<UInt32, map<UInt32, UInt32>> modIdMap;

/* Stores the reference id (0x00FFFFFF) separate from the mod id (0xFF000000) so that we can use them separately
and universally between players. */
struct FormID
{
	FormID(UInt32 refId)
	{
		formId = refId - ((refId / 0x01000000) * 0x01000000);
		modId = refId - formId;
	}

	FormID(UInt32 refId, UInt32 networkId)
	{
		formId = refId - ((refId / 0x01000000) * 0x01000000);
		modId = refId - formId;
		modId = (modIdMap[networkId].count(modId) > 0 ? modIdMap[networkId][modId] : modId);
	}

	int getId()
	{
		return formId;
	}

	UInt32 getMod()
	{
		return modId;
	}

	UInt32 getFull()
	{
		return modId + formId;
	}

	bool isInstanced()
	{
		return modId == 0xFF000000;
	}

	//Change the mod id.
	void setMod(UInt32 newModId)
	{
		modId = newModId;
	}

	//Change the formId without changing the modId
	void setFormId(UInt32 newFormId)
	{
		formId = newFormId - ((newFormId / 0x01000000) * 0x01000000);
	}

	UInt32 convertFull(UInt32 networkId)
	{
		modId = modIdMap[networkId][modId];
		return modId + formId;
	}

	string convertFullString(UInt32 networkId, bool decimal = false)
	{
		modId = (modIdMap[networkId].count(modId) > 0 ? modIdMap[networkId][modId] : modId);
		return to_string(decimal);
	}

	string to_string(bool decimal = false)
	{
		if (decimal)
			return std::to_string(modId + formId);
		else
		{
			std::stringstream buffer;
			buffer.str(std::string());
			buffer << "0x" << std::hex << std::setw(8) << std::setfill('0') << (modId + formId);
			return buffer.str();
		}
	}

private:
	UInt32 modId;
	int formId;
};

struct PlayerData
{
private:
	float xRotRaw, yRotRaw, zRotRaw;
public:
	UInt32 networkId;
	bool disabled, isDead, isFlying, inCombat, sitting, remotePlayer, bleedingOut, isArrested, isBribed, isAlarmed, isTeammate, isSneaking, isUnconscious;
	float x, y, z, xRot, yRot, zRot, height, jumpZOrigin;
	UInt32 formId, baseId, combatTarget, sitType, positionControllerFormId, targetControllerFormId;
	TESObjectCELL* lastCell;
	ActorEx *positionControllerActor, *actor;
	TESObjectREFR *targetController, *mount, *object, *positionController;
	bool isMounted, leftAttack, rightAttack, jump, alert, firstUpdate, equippedSpell, spawned;
	string sitTarget, name, lastLocation, sitState, sitAnim;
	int64 location;
	int playerNr;

	PlayerData() = default;

	PlayerData(int64 loc, UInt32 id) : networkId(id), location(loc){}

	//0 (x), 1 (y), 2 (z)
	float GetRawRot(int id)
	{
		if (id == 0)
			return xRotRaw;
		else if (id == 1)
			return yRotRaw;
		else if (id == 2)
			return zRotRaw;
	}

	void SetRawRot(int id, float val)
	{
		if (id == 0)
			xRotRaw = val;
		else if (id == 1)
			yRotRaw = val;
		else if (id == 2)
			zRotRaw = val;
	}

	Hashtable Serialize()
	{
		if (!actor)
			actor = (ActorEx*)DYNAMIC_CAST(LookupFormByID(formId), TESForm, Actor);

		Hashtable serializedData = Hashtable();

		serializedData.put<JString, float>("x", actor->pos.x);
		serializedData.put<JString, float>("y", actor->pos.y);
		serializedData.put<JString, float>("z", actor->pos.z);

		serializedData.put<JString, float>("xRot", GetAngleX((*g_skyrimVM)->GetClassRegistry(), 0, actor));

		if (yRotRaw != actor->rot.y)
		{
			yRotRaw = actor->rot.y;
			serializedData.put<JString, float>("yRot", GetAngleY((*g_skyrimVM)->GetClassRegistry(), 0, actor));
		}

		if (zRotRaw != actor->rot.z)
		{
			zRotRaw = actor->rot.z;
			serializedData.put<JString, float>("zRot", GetAngleZ((*g_skyrimVM)->GetClassRegistry(), 0, actor));
		}

		serializedData.put<JString, int64>("formId", formId);
		serializedData.put<JString, int64>("baseId", baseId);

		return serializedData;
	}

	static void Deserialize(Hashtable serializedData, PlayerData &newPlayer)
	{
		newPlayer.x = ValueObject<float>(serializedData.getValue("x")).getDataCopy();
		newPlayer.y = ValueObject<float>(serializedData.getValue("y")).getDataCopy();
		newPlayer.z = ValueObject<float>(serializedData.getValue("z")).getDataCopy();

		newPlayer.xRot = ValueObject<float>(serializedData.getValue("xRot")).getDataCopy();

		if (serializedData.contains("yRot"))
			newPlayer.yRot = ValueObject<float>(serializedData.getValue("yRot")).getDataCopy();
		else
			newPlayer.yRot = -10000000;

		if (serializedData.contains("zRot"))
			newPlayer.zRot = ValueObject<float>(serializedData.getValue("zRot")).getDataCopy();
		else
			newPlayer.zRot = -10000000;

		newPlayer.formId = ValueObject<int64>(serializedData.getValue("formId")).getDataCopy();
		newPlayer.baseId = ValueObject<int64>(serializedData.getValue("baseId")).getDataCopy();
	}
};

struct PapyrusData
{
public:
	string msg;
	UInt32 guid;
	UInt32 target;
	VMResultArray<BSFixedString> values;

	PapyrusData() = default;

	PapyrusData(string tMsg, UInt32 tGuid, UInt32 tTarget, VMResultArray<BSFixedString> tValues) : msg(tMsg), guid(tGuid), target(tTarget), values(tValues) {}
};

struct LockData
{
	UInt32 id;
	bool isLocked;
};

class NetworkHandler
{
public:
	NetworkHandler() = default;

	/* Pending commands, these will be processed by our papyrus script as frequently as the script allows. Commands have been moved to 
	papyrus scripts if they are prone to induce crashing when implemented in c++. Either due to blocking, or memory issues. If you're up
	for debugging, a significant performance increase can probably be obtained by moving these back into c++ and solving dealing with the
	source of the crashes. */
	static queue<PapyrusData> papyrusQueue;
	static mutex papyrus_mtx;
	static TESObjectREFR* PlayerRef;
	// Contains all the cells within a given location, this is used to help locate NPCs within interiors.
	static map<BGSLocation*, vector<TESObjectCELL*>> LocChildCellLookupTable;
	// Contains all the sub-locations within a given location, this is used to help locate NPCs within interiors.
	static map<BGSLocation*, vector<BGSLocation*>> LocChildLocationLookupTable;

	// RemotePlayerMap[networkId] - A map of all of the currently connected players [PlayerData], accessed using their network "networkId".
	static map<UInt32, PlayerData> RemotePlayerMap;
	/* LocalNpcMap[(Actor*)someNpc->formId] - "LocalNpcMap" is a map of all of the NPCs currently near the player, this map contains the NPCs that are 
	sent to remote players	when updating or spawning NPCs. RemoteNpcMap[(Actor*)someNpc->formId] - "RemoteNpcMap" is a map of all of the NPCs spawned 
	by other players, they exist if	we are not the host of the current location.*/
	static map<UInt32, PlayerData> LocalNpcMap, RemoteNpcMap;
	/* Contains pending NPC updates. Each entry in the queue contains the associated "networkId", and the new [PlayerData] for the NPC.
	The updates are quickly pushed into the queue to minimize blocking, and processed individually in the main update loop. */
	static queue<pair<UInt32, PlayerData>> RemoteNpcUpdates;
	// disabledNpcs[(Actor*)someNpc->formId] - All the currently disabled NPCs. Used to keep track of, and restore NPCs disabled by the network.
	static set<UInt32> disabledNpcs;
	// Will prevent the "PreventingUnauthorizedSpawn" function from disabling this NPC (referenceId). This should be cleared out as needed. (npcs no longer being needed)
	static map<UInt32, bool> ForceSpawn;
	// A simple list of player reference id's. Used to determine whether or not an npc is/was a player.
	static vector<UInt32> playerLookup;

	// Manages how often NPCs update to limit crashing from overly frequent calls.
	static map<UInt32, Timer> KOFATimers;
	// Signal for the remoteNpcs to be cleared on the next update.
	static bool fullRefresh;

	// ignoreSpawn - Temporarily disable "PreventingUnauthorizedSpawn" for the player.
	static bool ignoreSpawns;
	// dropTimer - Used to prevent item drop glitches.
	static Timer dropTimer;

	static bool HasInitialized;
	/* Locks are updated over the network in intervals. This is a list of all of the locks that have changed state since the last interval. */
	static vector<LockData> lockList;
	/* Determines how many lock/unlock events should we be ignoring from this lock. This is so that we don't get infinite locking/unlocking triggers. */
	static map<UInt32, int> ignoreLock;
	/* A list of every mod in our load order. The name of each one is turned into an MD5 hash, this list is sent to remote players
	and used to compare mods. When the same mods are found their prefixes (00-FF) are mapped to our local list. */
	static vector<string> myLoadOrder;
	static Timer receivedTime, dayTimer;

	//Used for both the debug console, and in-game display
	inline static void PrintNote(const char* message)
	{
		Tests::debug(message);
		Notification(GameState::skyrimVMRegistry, 0, nullptr, &BSFixedString(message));
	}

	inline static void DisableRemoteNpcs()
	{
		ActorEx* actor = NULL;
		for (map<UInt32, PlayerData>::iterator it = RemoteNpcMap.begin(); it != RemoteNpcMap.end(); ++it)
		{
			actor = (ActorEx*)DYNAMIC_CAST(LookupFormByID(it->second.formId), TESForm, Actor);

			if (actor)
				actor->ClearKeepOffsetFromActor();
		}
	}

	inline static bool IsEnabled(Actor* ref)
	{
		if (!ref)
			return false;

		return !IsDisabled(GameState::skyrimVMRegistry, 0, ref);
	}

	inline static bool DisableNet(Actor* ref)
	{
		if (!ref)
			return false;

		if (IsEnabled(ref))
		{
			NativeFunctions::ExecuteCommand("Disable 0", ref);

			//Only add the NPC if we can confirm its not a remote player.
			for (map<UInt32, PlayerData>::iterator it = RemotePlayerMap.begin(); it != RemotePlayerMap.end(); it++)
			{
				if (it->second.formId == ref->formID)
					return true;
			}

			disabledNpcs.insert(ref->formID);

			return true;
		}
		return false;
	}

	inline static bool EnableNet(Actor* ref)
	{
		if (!ref)
			return false;

		if (!IsEnabled(ref))
		{
			NativeFunctions::ExecuteCommand("Enable 0", ref);
			return true;
		}
		else
			return false;
	}

	/* Enable any NPCs that were disabled by the location master, and empty the disabled list. */
	inline static void RestoreLocalNpcs()
	{
		Actor* actor = NULL;
		for (set<UInt32>::iterator it = disabledNpcs.begin(); it != disabledNpcs.end(); ++it)
		{
			actor = DYNAMIC_CAST(LookupFormByID(*it), TESForm, Actor);
			EnableNet(actor);
		}

		disabledNpcs.clear();

		for (map<UInt32, PlayerData>::iterator it = LocalNpcMap.begin(); it != LocalNpcMap.end(); ++it)
		{
			actor = DYNAMIC_CAST(LookupFormByID(it->first), TESForm, Actor);

			if (actor)
				NativeFunctions::ExecuteCommand("ResetAI", actor);
		}
	}

	inline static void Disconnect()
	{
		DisableRemoteNpcs();
		RestoreLocalNpcs();
		modIdMap.clear();
		papyrusQueue = queue<PapyrusData>();
		RemoteNpcUpdates = queue<pair<UInt32, PlayerData>>();

		if (RemotePlayerMap.size() > 0)
		{
			for (map<UInt32, PlayerData>::reverse_iterator it = RemotePlayerMap.rbegin(); it != RemotePlayerMap.rend(); ++it)
				OnExit(it->first);
		}

		HasInitialized = false;
		locationMaster = false;
	}

	inline static void OnDisconnection()
	{
		PrintNote("Disconnected.");
	}

	inline static void NpcSpawn(PlayerData &remoteNpc, UInt32 networkId)
	{
		FormID npcId = FormID(remoteNpc.formId, networkId);
		UInt32 npcRefId = npcId.getFull();

		TESObjectREFR* positionRef = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, *g_thePlayer, LookupFormByID(ID_TESObjectSTAT::TGRItemMarker), 1, true, false);

		if (!positionRef)
			return;

		RemoteNpcMap[npcRefId].x = remoteNpc.x;
		RemoteNpcMap[npcRefId].y = remoteNpc.y;
		RemoteNpcMap[npcRefId].z = remoteNpc.z;

		RemoteNpcMap[npcRefId].xRot = remoteNpc.xRot;
		RemoteNpcMap[npcRefId].yRot = remoteNpc.yRot;
		RemoteNpcMap[npcRefId].zRot = remoteNpc.zRot;

		NativeFunctions::ExecuteCommand(("SetPos x " + to_string(RemoteNpcMap[npcRefId].x)).c_str(), positionRef);
		NativeFunctions::ExecuteCommand(("SetPos y " + to_string(RemoteNpcMap[npcRefId].y)).c_str(), positionRef);
		NativeFunctions::ExecuteCommand(("SetPos z " + to_string(RemoteNpcMap[npcRefId].z)).c_str(), positionRef);

		TESForm* npcBaseForm = LookupFormByID(FormID(remoteNpc.baseId, networkId).getFull());

		ignoreSpawns = true;

		TESObjectREFR* originalNpc = DYNAMIC_CAST(LookupFormByID(npcRefId), TESForm, TESObjectREFR);
		// Check if we have found an instance of that npc, if so, set it as our reference.
		if (originalNpc)
		{
			if (npcId.isInstanced())
			{
				if (!npcBaseForm)
					return;

				RemoteNpcMap[npcRefId].object = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, positionRef, npcBaseForm, 1, true, false);
				RemoteNpcMap[npcRefId].actor = (ActorEx*)LookupFormByID(RemoteNpcMap[npcRefId].object->formID);
			}
			else
			{
				RemoteNpcMap[npcRefId].object = originalNpc;

				if (!IsEnabled((Actor*)RemoteNpcMap[npcRefId].object))
				{
					NativeFunctions::ExecuteCommand("Enable 0", RemoteNpcMap[npcRefId].object);

					/* If the npc is disabled even after we enabled it, then it likely has an "enable parent".
					Because enabling an "enable parent" can also spawn additional, unwanted child npcs, we will instead spawn a copy
					based on the actors baseid. */
					if (RemoteNpcMap[npcRefId].object->baseForm && !IsEnabled((Actor*)RemoteNpcMap[npcRefId].object))
						RemoteNpcMap[npcRefId].object = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, positionRef, RemoteNpcMap[npcRefId].object->baseForm, 1, true, false);
				}

				NativeFunctions::ExecuteCommand("Resurrect 1", RemoteNpcMap[npcRefId].object); // For stubborn NPCs that don't want to return.
				RemoteNpcMap[npcRefId].actor = (ActorEx*)DYNAMIC_CAST(LookupFormByID(RemoteNpcMap[npcRefId].object->formID), TESForm, Actor);
			}
		}

		//If not, spawn a copy of our reference.
		else
		{
			if (npcBaseForm)
			{
				RemoteNpcMap[npcRefId].object = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, positionRef, npcBaseForm, 1, true, false);
				RemoteNpcMap[npcRefId].actor = (ActorEx*)DYNAMIC_CAST(RemoteNpcMap[npcRefId].object, TESObjectREFR, Actor);
			}
			else
				NativeFunctions::ExecuteCommand("Disable 0", positionRef);
		}

		ignoreSpawns = false;

		if (!RemoteNpcMap[npcRefId].actor)
			return;

		NativeFunctions::ExecuteCommand(((string)"MoveTo " + Utilities::GetFormIDString(positionRef->formID) + " 0 0 0").c_str(), RemoteNpcMap[npcRefId].object);

		//NativeFunctions::ExecuteCommand((char*)("SETPV IsTeammate " + (std::string)(npcValues[j + 13] == "1" ? "true" : "false")).c_str(), spawnController.back());

		// Disabling these makes the Actor easier to intentionally control.
		NativeFunctions::ExecuteCommand("sgv bAllowRotation 0", RemoteNpcMap[npcRefId].actor);
		NativeFunctions::ExecuteCommand("sgv bMotionDriven 0", RemoteNpcMap[npcRefId].actor);
		NativeFunctions::ExecuteCommand("sgv bAnimationDriven 0", RemoteNpcMap[npcRefId].actor);
		NativeFunctions::ExecuteCommand("sgv bHeadTracking 0", RemoteNpcMap[npcRefId].actor);
		NativeFunctions::ExecuteCommand("sgv bHeadTrackSpine 0", RemoteNpcMap[npcRefId].actor);

		RemoteNpcMap[npcRefId].formId = npcRefId;
		RemoteNpcMap[npcRefId].positionControllerFormId = positionRef->formID;
		RemoteNpcMap[npcRefId].spawned = true;

		KOFA(RemoteNpcMap[npcRefId].actor, positionRef, true);
	}

	// This will push the message onto a queue for papyrus to call.
	inline static void PushToSkyrim(string msg, UInt32 tGuid, TESObjectREFR* target)
	{
		if (!target)
			return;

		std::lock_guard<std::mutex> lock(papyrus_mtx);
		papyrusQueue.emplace(PapyrusData(msg, tGuid, target->formID, VMResultArray<BSFixedString>()));
	}

	// This will push the message onto a queue for papyrus to call, along with any additional parameters.
	inline static void PushToSkyrim(string msg, UInt32 tGuid, TESObjectREFR* target, vector<string> values)
	{
		if (!target)
			return;

		VMResultArray<BSFixedString> tArray;

		for (int i = 0; i < values.size(); i++)
			tArray.push_back(BSFixedString(values[i].c_str()));

		std::lock_guard<std::mutex> lock(papyrus_mtx);
		papyrusQueue.emplace(PapyrusData(msg, tGuid, target->formID, tArray));
	}

	inline static void SpawnPlayer(int64 networkId, Hashtable* data)
	{
		if (PlayerRef == nullptr)
			PlayerRef = (TESObjectREFR*)(*g_thePlayer);

		string lorder = (string)ValueObject<JString>(data->getValue(1)).getDataCopy().UTF8Representation();

		// This maps the remote player's load order to our local remote order. Mapping the prefixes from their game to ours. 
		std::map<std::string, std::string> plModMap;
		vector<string> values = split(lorder, ',');

		for (int i = 1; i < values.size(); i++)
		{
			for (int j = 0; j < myLoadOrder.size(); j++)
			{
				if (values[i] == myLoadOrder[j])
				{
					// modIdMap[networkId][remotePlayerModId] = localModId
					modIdMap[networkId][(i - 1) * 0x01000000] = (j * 0x01000000);
					break;
				}
			}
		}

		string name = (string)ValueObject<JString>(data->getValue(0)).getDataCopy().UTF8Representation();
		UInt32 raceId = FormID((UInt32)ValueObject<int64>(data->getValue(2)).getDataCopy()).convertFull(networkId);
		string raceName = (string)ValueObject<JString>(data->getValue(3)).getDataCopy().UTF8Representation();
		bool sex = (UInt32)ValueObject<int64>(data->getValue(4)).getDataCopy();
		UInt32 weight = (UInt32)ValueObject<int64>(data->getValue(5)).getDataCopy();
		UInt32 rightWeaponId = FormID((UInt32)ValueObject<int64>(data->getValue(6)).getDataCopy()).convertFull(networkId);
		UInt32 leftWeaponId = FormID((UInt32)ValueObject<int64>(data->getValue(7)).getDataCopy()).convertFull(networkId);
		UInt32 headArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(8)).getDataCopy()).convertFull(networkId);
		UInt32 hairTypeId = FormID((UInt32)ValueObject<int64>(data->getValue(9)).getDataCopy()).convertFull(networkId);
		UInt32 hairLongId = FormID((UInt32)ValueObject<int64>(data->getValue(10)).getDataCopy()).convertFull(networkId);
		UInt32 bodyArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(11)).getDataCopy()).convertFull(networkId);
		UInt32 handsArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(12)).getDataCopy()).convertFull(networkId);
		UInt32 forearmArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(13)).getDataCopy()).convertFull(networkId);
		UInt32 amuletArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(14)).getDataCopy()).convertFull(networkId);
		UInt32 ringArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(15)).getDataCopy()).convertFull(networkId);
		UInt32 feetArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(16)).getDataCopy()).convertFull(networkId);
		UInt32 calvesArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(17)).getDataCopy()).convertFull(networkId);
		UInt32 shieldArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(18)).getDataCopy()).convertFull(networkId);
		UInt32 circletArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(19)).getDataCopy()).convertFull(networkId);
		UInt32 mouthId = FormID((UInt32)ValueObject<int64>(data->getValue(20)).getDataCopy()).convertFull(networkId);
		UInt32 headId = FormID((UInt32)ValueObject<int64>(data->getValue(21)).getDataCopy()).convertFull(networkId);
		UInt32 eyesId = FormID((UInt32)ValueObject<int64>(data->getValue(22)).getDataCopy()).convertFull(networkId);
		UInt32 hairId = FormID((UInt32)ValueObject<int64>(data->getValue(23)).getDataCopy()).convertFull(networkId);
		UInt32 beardId = FormID((UInt32)ValueObject<int64>(data->getValue(24)).getDataCopy()).convertFull(networkId);
		UInt32 scarId = FormID((UInt32)ValueObject<int64>(data->getValue(25)).getDataCopy()).convertFull(networkId);
		UInt32 browId = FormID((UInt32)ValueObject<int64>(data->getValue(26)).getDataCopy()).convertFull(networkId);
		float height = (float)ValueObject<float>(data->getValue(27)).getDataCopy();
		UInt32 faceset = FormID((UInt32)ValueObject<int64>(data->getValue(28)).getDataCopy()).convertFull(networkId);
		UInt32 hairColor = (UInt32)ValueObject<int64>(data->getValue(29)).getDataCopy();
		UInt32 voiceId = FormID((UInt32)ValueObject<int64>(data->getValue(30)).getDataCopy()).convertFull(networkId);

		TESForm* emptyBase = LookupFormByID(0x0008A91A);
		TESObjectREFR* movementController = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, PlayerRef, emptyBase, 1, true, false);
		TESObjectREFR* targetController = PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, PlayerRef, emptyBase, 1, true, false);

		bool setRaceDirect = false;
		TESNPC* playerBase = NULL;

		if (raceId == 79683 || raceId == 559168) // High Elves
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079BED), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x0005EF9C), TESForm, TESNPC);
		}

		else if (raceId == 79680 || raceId == 559162) // Argonians
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x000B2E11), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00043E57), TESForm, TESNPC);
		}

		else if (raceId == 79689 || raceId == 559236) // Wood Elves
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079CD3), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x0005EF9A), TESForm, TESNPC);
		}

		else if (raceId == 79681 || raceId == 559164) // Bretons
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F65), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F6A), TESForm, TESNPC);
		}

		else if (raceId == 79682 || raceId == 559165) // Dark Elves
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F5B), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x0005EFA7), TESForm, TESNPC);
		}

		else if (raceId == 79684 || raceId == 559172) // Imperials
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F66), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00026921), TESForm, TESNPC);
		}

		else if (raceId == 79685 || raceId == 559173) // Khajiit
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x000EE856), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00043E59), TESForm, TESNPC);
		}

		else if (raceId == 79686 || raceId == 558996) // Nords
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F68), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x0001750C), TESForm, TESNPC);
		}

		else if (raceId == 79687 || raceId == 688825) // Orcs
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F4E), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F69), TESForm, TESNPC);
		}

		else if (raceId == 79688 || raceId == 559174) // Redguard
		{
			if (sex)
				playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F67), TESForm, TESNPC);
			else
				playerBase = DYNAMIC_CAST(LookupFormByID(0x0005B4F8), TESForm, TESNPC);
		}

		else // Custom races, animals, monsters, etc...
		{
			playerBase = DYNAMIC_CAST(LookupFormByID(0x00079F67), TESForm, TESNPC); //The race will be changed after the character is spawned.
			setRaceDirect = true;
		}

		SetInvulnerable(GameState::skyrimVMRegistry, 0, playerBase, true);
		NativeFunctions::ExecuteCommand(((string)"SetEssential " + Utilities::GetFormIDString(playerBase->formID) + " 1").c_str(), NULL);
		NativeFunctions::ExecuteCommand(("SetPos x " + to_string(PlayerRef->pos.x)).c_str(), movementController);
		NativeFunctions::ExecuteCommand(("SetPos y " + to_string(PlayerRef->pos.y - 2000)).c_str(), movementController);
		NativeFunctions::ExecuteCommand(("SetPos z " + to_string(PlayerRef->pos.z)).c_str(), movementController);

		Actor* newPlayer = DYNAMIC_CAST(PlaceAtMe_Native(GameState::skyrimVMRegistry, 0, movementController, playerBase, 1, true, false), TESObjectREFR, Actor);

		if (std::find(playerLookup.begin(), playerLookup.end(), newPlayer->formID) == playerLookup.end())
			playerLookup.push_back(newPlayer->formID);

		EnableNet(newPlayer);

		if (setRaceDirect)
			SetRace(NULL, newPlayer, BSFixedString(raceName.c_str()));

		NativeFunctions::SetVoice(newPlayer, voiceId);
		NativeFunctions::SetHairColor(newPlayer, hairColor);
		NativeFunctions::SetFaceTextureSet(newPlayer, faceset);
		NativeFunctions::SetHeight(newPlayer, height);

		IgnoreFriendlyHits(GameState::skyrimVMRegistry, 0, newPlayer, true);
		SetRelationshipRank(GameState::skyrimVMRegistry, 0, newPlayer, *g_thePlayer, 1);
		APS(NULL, newPlayer, "SkyTools InitiateTracking");
		RemoveAllItems(GameState::skyrimVMRegistry, 0, newPlayer, false, true);
		SetDisplayName(NULL, newPlayer, name.c_str());

		NativeFunctions::ChangeHeadPart(newPlayer, mouthId);
		NativeFunctions::ChangeHeadPart(newPlayer, headId);
		NativeFunctions::ChangeHeadPart(newPlayer, eyesId);
		NativeFunctions::ChangeHeadPart(newPlayer, hairId);
		NativeFunctions::ChangeHeadPart(newPlayer, beardId);
		NativeFunctions::ChangeHeadPart(newPlayer, scarId);
		NativeFunctions::ChangeHeadPart(newPlayer, browId);
		NativeFunctions::QueueNiNodeUpdate(newPlayer);

		TESForm* rawEquip = LookupFormByID(rightWeaponId);

		if (rawEquip)
		{
			if (rawEquip->GetFormType() == 41)
				NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1 right").c_str(), newPlayer);
			else if (rawEquip->GetFormType() == 22)
				EquipSpell(GameState::skyrimVMRegistry, 0, newPlayer, DYNAMIC_CAST(rawEquip, TESForm, SpellItem), 1);
		}

		rawEquip = LookupFormByID(leftWeaponId);

		if (rawEquip)
		{
			if (rawEquip->GetFormType() == 41)
				NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1 left").c_str(), newPlayer);
			else if (rawEquip->GetFormType() == 22)
				EquipSpell(GameState::skyrimVMRegistry, 0, newPlayer, DYNAMIC_CAST(rawEquip, TESForm, SpellItem), 0);
		}

		rawEquip = LookupFormByID(headArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(hairTypeId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(hairLongId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(bodyArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(handsArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(forearmArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(amuletArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(ringArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(feetArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(calvesArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(shieldArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);
		rawEquip = LookupFormByID(circletArmorId);
		if (rawEquip)
			NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(rawEquip->formID) + " 1").c_str(), newPlayer);

		NativeFunctions::ExecuteCommand("SetRestrained 1", newPlayer);
		MoveTo(GameState::skyrimVMRegistry, 0, newPlayer, newPlayer, 0, 0, 0, true); // Releases the player's movement
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("Blindness"), 10000000000000);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("ShoutRecoveryMult"), 0);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("VoiceRate"), 10000000000000);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("VoicePoints"), 10000000000000);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("Health"), 10000000000000);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("Magicka"), 10000000000000);
		ForceActorValue(GameState::skyrimVMRegistry, 0, newPlayer, new BSFixedString("Stamina"), 10000000000000);

		InitializeNewPlayer(NULL, newPlayer->formID, movementController->formID, targetController->formID, networkId);
		RemotePlayerMap[networkId].name = name;
		Networking::instance->OnRoomEnter();
	}

	inline static void OnUserExit()
	{
		PrintNote("A user has left the room.");
	}

	inline static void SetMyLoadOrder(vector<std::string> order)
	{
		myLoadOrder = vector<std::string>(order);
	}

	inline static bool AddLocalNpcList(Actor* tActor)
	{
		if (!tActor)
			return false;

		//Ensure that the npc is not already in the list.
		if (LocalNpcMap.count(tActor->formID) == 1)
			return false;

		//Ensure that the npc is not the player.
		if (tActor->formID == (*g_thePlayer)->formID)
			return false;

		//Ensure that the npc is not another player.
		for (std::map<UInt32, PlayerData>::iterator it = RemotePlayerMap.begin(); it != RemotePlayerMap.end(); ++it)
		{
			if (it->second.formId == tActor->formID)
				return false;
		}

		if (tActor->baseForm)
			LocalNpcMap[tActor->formID].baseId = tActor->baseForm->formID;
		else
			LocalNpcMap[tActor->formID].baseId = tActor->formID;

		LocalNpcMap[tActor->formID].formId = tActor->formID;
		LocalNpcMap[tActor->formID].baseId = tActor->baseForm->formID;
		LocalNpcMap[tActor->formID].object = tActor;
		LocalNpcMap[tActor->formID].location = NetworkState::locationId;
		LocalNpcMap[tActor->formID].remotePlayer = false;
		LocalNpcMap[tActor->formID].actor = (ActorEx*)tActor;
		LocalNpcMap[tActor->formID].alert = false;
		LocalNpcMap[tActor->formID].inCombat = false;
		LocalNpcMap[tActor->formID].isDead = IsDead(GameState::skyrimVMRegistry, 0, LocalNpcMap[tActor->formID].actor);
		LocalNpcMap[tActor->formID].isSneaking = false;
		LocalNpcMap[tActor->formID].isUnconscious = false;

		return true;
	}

	inline static bool IsLocationMaster()
	{
		return Networking::instance->getIsHost();
	}

	inline static bool IsPlayer(UInt32 formId)
	{
		if (std::find(playerLookup.begin(), playerLookup.end(), formId) == playerLookup.end())
			return false;
		return true;
	}

	// When a player disconnects, disable them, and move their Actor to a holding area.
	inline static void OnExit(UInt32 networkId)
	{
		NativeFunctions::ExecuteCommand("MoveTo player 0 -2000 0", RemotePlayerMap[networkId].actor);
		NativeFunctions::ExecuteCommand("MoveTo player 0 -2000 0", RemotePlayerMap[networkId].positionController);
		NativeFunctions::ExecuteCommand("APS SkyTools DisableTracking", RemotePlayerMap[networkId].actor);
	}

	// Finalize the player's spawn
	inline static void InitializeNewPlayer(StaticFunctionTag* base, UInt32 newPlayer, UInt32 movementController, UInt32 targetController, UInt32 networkId)
	{
		RemotePlayerMap[networkId].networkId = networkId;
		RemotePlayerMap[networkId].disabled = false;
		RemotePlayerMap[networkId].actor = (ActorEx*)LookupFormByID(newPlayer);
		RemotePlayerMap[networkId].object = RemotePlayerMap[networkId].actor;
		RemotePlayerMap[networkId].lastLocation = "";
		RemotePlayerMap[networkId].lastCell = RemotePlayerMap[networkId].actor->parentCell;
		RemotePlayerMap[networkId].positionController = (TESObjectREFR*)LookupFormByID(movementController);
		RemotePlayerMap[networkId].positionControllerActor = (ActorEx*)RemotePlayerMap[networkId].positionController;
		RemotePlayerMap[networkId].height = ((TESNPC*)RemotePlayerMap[networkId].actor->baseForm)->height;
		RemotePlayerMap[networkId].formId = newPlayer;
		RemotePlayerMap[networkId].positionControllerFormId = RemotePlayerMap[networkId].positionController->formID;
		RemotePlayerMap[networkId].targetController = (TESObjectREFR*)LookupFormByID(targetController);
		RemotePlayerMap[networkId].targetControllerFormId = RemotePlayerMap[networkId].targetController->formID;
		RemotePlayerMap[networkId].equippedSpell = false;
		RemotePlayerMap[networkId].mount = NULL;
		RemotePlayerMap[networkId].isMounted = false;
		RemotePlayerMap[networkId].leftAttack = false;
		RemotePlayerMap[networkId].rightAttack = false;
		RemotePlayerMap[networkId].jump = false;
		RemotePlayerMap[networkId].firstUpdate = true;
		RemotePlayerMap[networkId].alert = false;
		RemotePlayerMap[networkId].sitType = 0;

		// These values cannot be set in papyrus, they lead to errors.
		NativeFunctions::ExecuteCommand("forceav Confidence 4", RemotePlayerMap[networkId].actor);
		NativeFunctions::ExecuteCommand("forceav Aggression 0", RemotePlayerMap[networkId].actor);
		
		NativeFunctions::ExecuteCommand("sgv bAllowRotation 0", RemotePlayerMap[networkId].actor);
		// Ensures that the actor always responds to movement requests.If this is on, the actor will sometimes refuse to walk.
		NativeFunctions::ExecuteCommand("sgv bMotionDriven 0", RemotePlayerMap[networkId].actor);
		NativeFunctions::ExecuteCommand("sgv bAnimationDriven 0", RemotePlayerMap[networkId].actor);
		// Keeps actor from turning their body to face someone, does not actually affect head tracking.
		NativeFunctions::ExecuteCommand("sgv bHeadTracking 0", RemotePlayerMap[networkId].actor);
		NativeFunctions::ExecuteCommand("sgv bHeadTrackSpine 0", RemotePlayerMap[networkId].actor);
		NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "IdleForceDefaultState");

		// Ensure that the player isn't being treated like an NPC
		if (LocalNpcMap.count(RemotePlayerMap[networkId].formId))
		{
			for (map<UInt32, PlayerData>::iterator it = LocalNpcMap.begin(); it != LocalNpcMap.end(); ++it)
			{
				if (it->first == RemotePlayerMap[networkId].formId)
				{
					LocalNpcMap.erase(RemotePlayerMap[networkId].formId);
					break;
				}
			}
		}

		RemotePlayerMap[networkId].spawned = true;
	}

	inline static bool IsConnected()
	{
		return NetworkState::bIsConnected;
	}

	/* Disable the NPC if its not in the list that the location master sent us. If we are the location master, ignore it. */
	inline static bool PreventingUnauthorizedSpawn(TESObjectREFR* obj)
	{
		if (!obj)
			return false;

		if (!IsLocationMaster())
		{
			if (!IsPlayer(obj->formID) && IsConnected() && !ignoreSpawns && obj->loadedState)
			{
				//If we have been sent an NPC list, check if this NPC is contained. If not, disable it.
				if (obj->GetFormType() == kFormType_Character && RemoteNpcMap.count(obj->formID) == 0 && ForceSpawn.count(obj->formID) == 0)
					DisableNet(DYNAMIC_CAST(obj, TESObjectREFR, Actor));
			}
			return true;
		}
		return false;
	}

	inline static bool IsAlone()
	{
		if (Networking::instance->getPlayerCount() > 1)
			return false;
		return true;
	}

	// Add the actor to the local NPC list, and reset its AI so that it can move freely.
	inline static void UpdateLocalNpcList(Actor* actor)
	{
		if (actor && actor->loadedState && NetworkHandler::AddLocalNpcList(actor))
			NativeFunctions::ExecuteCommand("ResetAI", actor);
	}

	inline static bool DistanceExceeded(TESObjectREFR* source, TESObjectREFR* target, float distance)
	{
		if (!source || !target)
			return true;

		return (abs(target->pos.x - source->pos.x) + abs(target->pos.y - source->pos.y) + abs(target->pos.z - source->pos.z)) > distance;
	}

	inline static void UpdateNpc(PlayerData &tNpc, UInt32 networkId)
	{
		UInt32 npcNum = FormID(tNpc.formId, networkId).getFull();

		//If the NPC is new, spawn them and continue.
		if (!RemoteNpcMap[npcNum].spawned)
		{
			NpcSpawn(tNpc, networkId);
			return;
		}

		Actor *actor = DYNAMIC_CAST(LookupFormByID(RemoteNpcMap[npcNum].formId), TESForm, Actor),
			*cActor = (Actor*)DYNAMIC_CAST(LookupFormByID(RemoteNpcMap[npcNum].positionControllerFormId), TESForm, TESObjectREFR);

		if (!actor || !cActor)
			return;

		if (actor->pos.x != tNpc.x || actor->pos.y != tNpc.y || actor->pos.z != tNpc.z)
		{
			/* Get the difference between our last update and this update
			and extrapolate (albeit simply) to our next predicted position. 
			Which the npc will move towards. */
			float interX = (tNpc.x - RemoteNpcMap[npcNum].x) + tNpc.x;
			float interY = (tNpc.y - RemoteNpcMap[npcNum].y) + tNpc.y;
			float interZ = (tNpc.z - RemoteNpcMap[npcNum].z) + tNpc.z;

			RemoteNpcMap[npcNum].x = tNpc.x;
			RemoteNpcMap[npcNum].y = tNpc.y;
			RemoteNpcMap[npcNum].z = tNpc.z;

			NativeFunctions::ExecuteCommand(("SetPos x " + to_string(interX)).c_str(), cActor);
			NativeFunctions::ExecuteCommand(("SetPos y " + to_string(interY)).c_str(), cActor);
			NativeFunctions::ExecuteCommand(("SetPos z " + to_string(interZ)).c_str(), cActor);

			if (DistanceExceeded(actor, cActor, 512))
			{
				NativeFunctions::ExecuteCommand(((string)"MoveTo " + Utilities::GetFormIDString(cActor->formID) + " 0 0 0").c_str(), actor);
				KOFA(actor, cActor, true);
			}
			else
				KOFA(actor, cActor, true);
		}

		if (RemoteNpcMap[npcNum].xRot != tNpc.xRot)
		{
			NativeFunctions::ExecuteCommand(("SetAngle x " + to_string(tNpc.xRot)).c_str(), actor);
			RemoteNpcMap[npcNum].xRot = tNpc.xRot;
		}

		if (tNpc.yRot != -10000000)
		{
			NativeFunctions::ExecuteCommand(("SetAngle y " + to_string(tNpc.yRot)).c_str(), actor);
			RemoteNpcMap[npcNum].yRot = tNpc.yRot;
		}

		if (tNpc.zRot != -10000000)
		{
			NativeFunctions::ExecuteCommand(("SetAngle z " + to_string(tNpc.zRot)).c_str(), actor);
			RemoteNpcMap[npcNum].zRot = tNpc.zRot;
		}
	}

	// Papyrus interface, returns the count of elements in the papyrusQueue.
	inline static UInt32 GetMessageCount(StaticFunctionTag* base)
	{
		return papyrusQueue.size();
	}

	// Papyrus interface, returns the network id at the front of the papyrusQueue.
	inline static UInt32 ReadMessageGuid(StaticFunctionTag* base)
	{
		if (papyrusQueue.size() <= 0)
			return 0;

		return papyrusQueue.front().guid;
	}

	// Papyrus interface, returns the command at the front of the papyrusQueue.
	inline static BSFixedString ReadMessageName(StaticFunctionTag* base)
	{
		if (papyrusQueue.size() <= 0)
			return BSFixedString();

		return BSFixedString(papyrusQueue.front().msg.c_str());
	}

	// Papyrus interface, returns the target at the front of the papyrusQueue.
	inline static TESForm* ReadMessageRef(StaticFunctionTag* base)
	{
		if (papyrusQueue.size() <= 0)
			return NULL;

		return LookupFormByID(papyrusQueue.front().target);
	}

	// Papyrus interface, returns the parameters at the front of the papyrusQueue.
	inline static VMResultArray<BSFixedString> ReadMessageValues(StaticFunctionTag* base)
	{
		if (papyrusQueue.size() <= 0)
			return VMResultArray<BSFixedString>();

		VMResultArray<BSFixedString> tVal = papyrusQueue.front().values;
		std::lock_guard<std::mutex> lock(papyrus_mtx);
		papyrusQueue.pop();
		return tVal;
	}

	// Papyrus interface
	inline static BSFixedString GetSitAnimation(StaticFunctionTag* base, Actor* ref)
	{
		if (!ref)
			return "";

		return BSFixedString(NativeFunctions::GetSitAnimation(ref));
	}

	// Papyrus interface
	inline static void MountActor(StaticFunctionTag* base, Actor* ref, TESObjectREFR* mountRef)
	{
		if (!ref || !mountRef)
			return;

		NativeFunctions::ExecuteCommand(((string)"mountactor " + Utilities::GetFormIDString(mountRef->formID)).c_str(), ref);
	}

	// Papyrus interface
	inline static void ResurrectExtended(StaticFunctionTag* base, Actor* ref)
	{
		if (!ref)
			return;

		NativeFunctions::ExecuteCommand("Resurrect 1", ref);
	}

	// Papyrus interface
	inline static void SetRace(StaticFunctionTag* base, Actor* ref, BSFixedString raceName)
	{
		if (!ref)
			return;

		NativeFunctions::ExecuteCommand(((string)"SetRace " + raceName.data).c_str(), ref);
	}

	/* We may in the future want to replace all of the console commands with a generic console command function
	We'll have to weigh the potential for exploitation. */
	inline static void APS(StaticFunctionTag* base, Actor* ref, BSFixedString command)
	{
		if (!ref)
			return;

		NativeFunctions::ExecuteCommand(((string)"APS " + command.data).c_str(), ref);
	}

	// Papyrus interface
	inline static void SetDisplayName(StaticFunctionTag* base, Actor* ref, BSFixedString dName)
	{
		if (!ref)
			return;

		NativeFunctions::SetDisplayName(ref, dName.data, true);
	}

	// Papyrus interface
	inline static void StartTimer(StaticFunctionTag* base, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "receivedTime")
			receivedTime.StartTimer();
		else if (normalizedName == "dropTimer")
			dropTimer.StartTimer();
	}

	// Papyrus interface
	inline static bool GetRemotePlayerDataBool(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "isMounted")
			return RemotePlayerMap[networkId].isMounted;
		else if (normalizedName == "alert")
			return RemotePlayerMap[networkId].alert;
		else if (normalizedName == "firstUpdate")
			return RemotePlayerMap[networkId].firstUpdate;
		else if (normalizedName == "disabled")
			return RemotePlayerMap[networkId].disabled;
		else if (normalizedName == "jump")
			return RemotePlayerMap[networkId].jump;
		else if (normalizedName == "equippedSpell")
			return RemotePlayerMap[networkId].equippedSpell;
		else
			return false;
	}

	// Papyrus interface
	inline static float GetRemotePlayerDataFloat(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "height")
			return RemotePlayerMap[networkId].height;
		else if (normalizedName == "jumpZOrigin")
			return RemotePlayerMap[networkId].jumpZOrigin;
		else
			return -1;
	}

	// Papyrus interface
	inline static UInt32 GetRemotePlayerDataInt(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "sitType")
			return RemotePlayerMap[networkId].sitType;
		else
			return -1;
	}

	// Papyrus interface
	inline static BSFixedString GetRemotePlayerDataString(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName)
	{
		string normalizedName = tName.data;
		
		if (normalizedName == "lastLocation")
			return RemotePlayerMap[networkId].lastLocation.c_str();
		else if (normalizedName == "locationid")
			return to_string(NetworkState::locationId).c_str();
		else if (normalizedName == "location")
			return to_string(RemotePlayerMap[networkId].location).c_str();
		else if (normalizedName  == "sitAnim")
			return RemotePlayerMap[networkId].sitAnim.c_str();
		else if (normalizedName == "horse")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[networkId].mount->formID)).c_str();
		else if (normalizedName == "targetController")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[networkId].targetController->formID)).c_str();
		else if (normalizedName == "positionController")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[networkId].positionController->formID)).c_str();
		else
			return "";
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataBool(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName, bool val)
	{
		string normalizedName = tName.data;
		UInt32 refId = networkId; //To clarify when we're using a refId in place of the player number for papyrus global function calls.

		if (normalizedName == "jump")
			RemotePlayerMap[networkId].jump = val;
		else if (normalizedName == "alert")
			RemotePlayerMap[networkId].alert = val;
		else if (normalizedName == "equippedSpell")
			RemotePlayerMap[networkId].equippedSpell = val;
		else if (normalizedName == "isMounted")
			RemotePlayerMap[networkId].isMounted = val;
		else if (normalizedName == "firstUpdate")
			RemotePlayerMap[networkId].firstUpdate = val;
		else if (normalizedName == "ignoreSpawns")
			ignoreSpawns = val;
		else if (normalizedName == "ForceSpawn")
			ForceSpawn[refId] = val;
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataFloat(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName, float val)
	{
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataInt(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName, UInt32 val)
	{
		string normalizedName = tName.data;

		if (normalizedName == "sitType")
			RemotePlayerMap[networkId].sitType = val;
		else if (normalizedName == "horse")
			RemotePlayerMap[networkId].mount = (TESObjectREFR*)LookupFormByID(val);
		else if (normalizedName == "Player")
			playerLookup.push_back(val);
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataString(StaticFunctionTag* base, UInt32 networkId, BSFixedString tName, BSFixedString val)
	{
		string normalizedName = tName.data;

		if (normalizedName == "name")
			RemotePlayerMap[networkId].name = val.data;
	}

	// Papyrus interface
	inline static bool HasSecondsPassed(StaticFunctionTag* base, BSFixedString tName, float seconds)
	{
		if (tName.data == "dropTimer")
		{
			if (dropTimer.HasMillisecondsPassed(seconds * 1000))
				return true;
			else
				return false;
		}
		return false;
	}

	// Papyrus interface
	inline static bool HasMillisecondsPassed(StaticFunctionTag* base, BSFixedString tName, float milliseconds)
	{
		if (tName.data == "receivedTime")
		{
			if (receivedTime.HasMillisecondsPassed(milliseconds))
				return true;
			else
				return false;
		}
		return false;
	}

	inline static void SendData(char* data)
	{
		if (!IsConnected())
			return;

		Hashtable hashData = Hashtable();
		hashData.put<int, int64>(0, NetworkState::networkId);
		hashData.put<int, JString>(1, JString(data));
		Networking::instance->sendEvent<Hashtable>(true, hashData, NetworkState::EV::ID_MESSAGE, NetworkState::CHANNEL::EVENT);
	}

	// Enables/disables NPCs that players that enter/leave the current area. Determines the current locations master for NPC synchronization, weather, etc...
	inline static void OnLocationUpdate()
	{
		if (IsAlone())
			return;

		if (IsLocationMaster())
		{
			if (!locationMaster)
			{
				locationMaster = true;
				DisableRemoteNpcs();
				RestoreLocalNpcs();
			}
		}
		else
		{
			if (locationMaster)
				locationMaster = false;
		}
	}

	inline static void ProcessLock(Hashtable* data)
	{
		UInt32 lockId;
		bool isLocked;

		for (int i = 0; i < data->getSize() - 1; i += 2)
		{
			lockId = (UInt32)ValueObject<int64>(data->getValue(i)).getDataCopy();
			isLocked = ValueObject<bool>(data->getValue(i+1)).getDataCopy();

			ignoreLock[lockId] += 1;

			if (isLocked)
				NativeFunctions::ExecuteCommand(("\"" + Utilities::GetFormIDString(lockId) + "\"" + ".lock").c_str(), 0);
			else
				NativeFunctions::ExecuteCommand(("\"" + Utilities::GetFormIDString(lockId) + "\"" + ".unlock").c_str(), 0);
		}
	}

	// "KeepOffsetFromActor" with limits on how often its called, to prevent crashing.
	inline static void KOFA(Actor* source, TESObjectREFR* target, bool npc = false)
	{
		if (!source || !target)
			return;

		if (KOFATimers[source->formID].HasMillisecondsPassed(250))
		{
			KOFATimers[source->formID].StartTimer();
			// The NPCs have a slower update rate, so they are allowed to lag behind a bit further than other players.
			if (npc)
				((ActorEx*)source)->KeepOffsetFromActor((Actor*)target, 0, 0, 0, 0, 0, 0, 500, 20);
			else
				((ActorEx*)source)->KeepOffsetFromActor((Actor*)target, 0, 0, 0, 0, 0, 0, 128, 20);
		}
	}

	inline static bool IsSpawned(const UInt32 networkId)
	{
		if (RemotePlayerMap.count(networkId) < 1 || !RemotePlayerMap[networkId].spawned || !RemotePlayerMap[networkId].actor)
			return false;
		return true;
	}

	inline static void ReceiveEvent(const int playerNr, const nByte eventCode, const Object& eventContent)
	{
		if (eventCode == NetworkState::EV::ID_MESSAGE)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(hashData.getValue<int>(0)).getDataCopy();

			if (!IsSpawned(networkId))
				return;

			OnPublicMessage(networkId, ValueObject<JString>(hashData.getValue(1)).getDataCopy());
			Tests::debug("ID_MESSAGE received.");
		}

		else if (eventCode == NetworkState::EV::ID_SPAWN_PLAYER)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			UInt32 networkId = ValueObject<int64>(hashData.getValue<int>(31)).getDataCopy();

			if (IsSpawned(networkId))
			{
				RemotePlayerMap[networkId].playerNr = playerNr;
				return;
			}

			SpawnPlayer(networkId, &hashData);
			Tests::debug("ID_SPAWN_PLAYER received");
		}

		else if (eventCode == NetworkState::EV::ID_LOCK_UPDATE)
		{
			Hashtable lockData = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(lockData.getValue<int>(lockData.getSize() - 1)).getDataCopy();

			if (!IsSpawned(networkId))
				return;

			ProcessLock(&lockData);
			Tests::debug("ID_LOCK_UPDATE received.");
		}

		else if (eventCode == NetworkState::EV::ID_POSITION_UPDATE)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(hashData.getValue(6)).getDataCopy();

			if (!IsSpawned(networkId))
				return;

			float x = ValueObject<float>(hashData.getValue(0)).getDataCopy(),
				y = ValueObject<float>(hashData.getValue(1)).getDataCopy(),
				z = ValueObject<float>(hashData.getValue(2)).getDataCopy();

			int	xRot = ValueObject<int>(hashData.getValue(3)).getDataCopy(),
				yRot = ValueObject<int>(hashData.getValue(4)).getDataCopy(),
				zRot = ValueObject<int>(hashData.getValue(5)).getDataCopy();

			OnLocationUpdate();

			Tests::debug("ID_POSITION_UPDATE received.");

			if (IsEnabled(RemotePlayerMap[networkId].actor))
			{
				Tests::debug("ID_POSITION_UPDATE processing.");

				// Update the position to guide the Actor towards the positionController.
				NativeFunctions::ExecuteCommand(("SetPos x " + to_string(x)).c_str(), RemotePlayerMap[networkId].positionController);
				NativeFunctions::ExecuteCommand(("SetPos y " + to_string(y)).c_str(), RemotePlayerMap[networkId].positionController);
				NativeFunctions::ExecuteCommand(("SetPos z " + to_string(z)).c_str(), RemotePlayerMap[networkId].positionController);

				/* If someone would like to test a quest based replacement for this that would be great, as "KeepOffsetFromActor" does no obstacle avoidance.
				Which is good, and bad I suppose. Additionally "KeepOffsetFromActor" requires successive calls to ensure that the built in AI does not take over. */
				KOFA(RemotePlayerMap[networkId].actor, RemotePlayerMap[networkId].positionController);

				if (DistanceExceeded(RemotePlayerMap[networkId].positionController, RemotePlayerMap[networkId].actor, 1024))
				{
					NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[networkId].positionController->formID) + " 0 0 0").c_str(), RemotePlayerMap[networkId].actor);
					KOFA(RemotePlayerMap[networkId].actor, RemotePlayerMap[networkId].positionController);
				}

				if (RemotePlayerMap[networkId].sitType != 0 || RemotePlayerMap[networkId].firstUpdate)
				{
					NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[networkId].positionController->formID) + " 0 0 0").c_str(), RemotePlayerMap[networkId].actor);
					RemotePlayerMap[networkId].firstUpdate = false;
					KOFA(RemotePlayerMap[networkId].actor, RemotePlayerMap[networkId].positionController);
				}

				if (RemotePlayerMap[networkId].actor->IsOnMount())
				{
					NativeFunctions::ExecuteCommand(("SetAngle x " + to_string(xRot)).c_str(), RemotePlayerMap[networkId].mount);
					NativeFunctions::ExecuteCommand(("SetAngle y " + to_string(yRot)).c_str(), RemotePlayerMap[networkId].mount);
					NativeFunctions::ExecuteCommand(("SetAngle z " + to_string(zRot)).c_str(), RemotePlayerMap[networkId].mount);
				}
				else
				{
					NativeFunctions::ExecuteCommand(("SetAngle x " + to_string(xRot)).c_str(), RemotePlayerMap[networkId].actor);
					NativeFunctions::ExecuteCommand(("SetAngle y " + to_string(yRot)).c_str(), RemotePlayerMap[networkId].actor);
					NativeFunctions::ExecuteCommand(("SetAngle z " + to_string(zRot)).c_str(), RemotePlayerMap[networkId].actor);
				}

				if (RemotePlayerMap[networkId].sitAnim == "")
				{
					// The Actor exits a sit, or idle animation.
					if (RemotePlayerMap[networkId].sitType == 1000)
					{
						SetDontMove(GameState::skyrimVMRegistry, 0, RemotePlayerMap[networkId].actor, false);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "IdleForceDefaultState");
						NativeFunctions::ExecuteCommand("SetUnconscious 0", RemotePlayerMap[networkId].actor);
						RemotePlayerMap[networkId].sitType = 0;
					}
					// Shake the Actor out of its current sit/idle animation if it isn't supposed to be sitting/idling.
					else if (RemotePlayerMap[networkId].sitType == 0)
					{
						if (GetSitAnimation(NULL, RemotePlayerMap[networkId].actor).data && GetSitAnimation(NULL, RemotePlayerMap[networkId].actor).data != "")
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "IdleForceDefaultState");
					}
				}

				else
				{
					// The Actor enters the a new sit, or idle animation.
					if (RemotePlayerMap[networkId].sitType == 0)
					{
						RemotePlayerMap[networkId].sitType = 1000;
						SetDontMove(GameState::skyrimVMRegistry, 0, RemotePlayerMap[networkId].actor, true);
						RemotePlayerMap[networkId].actor->ClearKeepOffsetFromActor();
						NativeFunctions::ExecuteCommand("SetUnconscious 1", RemotePlayerMap[networkId].actor);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, RemotePlayerMap[networkId].sitAnim.c_str());
					}
				}

				// Used to alter where the Actor looks, and is used when the Actor is casting magic, or firing a bow. This is unfinished.
				float xOffset = 1000 * sin(yRot), yOffset = 1000 * cos(yRot), zOffset = yRot + (RemotePlayerMap[networkId].height * 0.75);

				//Move target controller
				NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[networkId].actor->formID) + " " + to_string(xOffset) + " " + to_string(yOffset) + " " + to_string(zOffset)).c_str(), RemotePlayerMap[networkId].targetController);

				//SetLookAt(GameState::skyrimVMRegistry, 0, RemotePlayerMap[networkId].actor, RemotePlayerMap[networkId].targetController, false);

				// Signals the end of the Actor's current jump state.
				if (RemotePlayerMap[networkId].jump && RemotePlayerMap[networkId].actor->pos.z <= RemotePlayerMap[networkId].jumpZOrigin)
					RemotePlayerMap[networkId].jump = false;

				// Toggle the actor's alert state.
				if (RemotePlayerMap[networkId].actor->actorState.IsWeaponDrawn())
				{
					if (!RemotePlayerMap[networkId].alert)
					{
						NativeFunctions::ExecuteCommand("SetAlert 0", RemotePlayerMap[networkId].actor);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "Unequip");
					}
				}

				else
				{
					if (RemotePlayerMap[networkId].alert)
					{
						NativeFunctions::ExecuteCommand("SetAlert 1", RemotePlayerMap[networkId].actor);

						if (RemotePlayerMap[networkId].equippedSpell)
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "Magic_Equip");
						else
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[networkId].actor, "weapEquip");
					}
				}
			}
		}

		else if (eventCode == NetworkState::EV::ID_SET_TIME)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();

			float daysPassed = (float)ValueObject<float>(hashData.getValue(0)).getDataCopy(),
				tDay = (float)ValueObject<float>(hashData.getValue(1)).getDataCopy(),
				tHour = (float)ValueObject<float>(hashData.getValue(2)).getDataCopy(),
				tMonth = (float)ValueObject<float>(hashData.getValue(3)).getDataCopy(),
				tYear = (float)ValueObject<float>(hashData.getValue(4)).getDataCopy();

			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDaysPassed), daysPassed);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameHour), tHour);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDay), tDay);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameMonth), tMonth);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameYear), tYear);

			Tests::debug("ID_SET_TIME received.");
		}

		else if (eventCode == NetworkState::EV::ID_SET_WEATHER)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(hashData.getValue(1)).getDataCopy();

			if (!IsSpawned(networkId))
				return;

			UInt32 weatherId = (int64)ValueObject<int64>(hashData.getValue(0)).getDataCopy();
			if (!IsLocationMaster() && IsEnabled(RemotePlayerMap[networkId].actor))
				PushToSkyrim("Weather", networkId, *g_thePlayer, vector<string>{ to_string(weatherId) });

			Tests::debug("ID_SET_WEATHER received.");
		}

		else if (eventCode == NetworkState::EV::ID_SET_ANIMATION)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(hashData.getValue(1)).getDataCopy();

			if (!IsSpawned(networkId))
				return;

			// The player's animation is set here, and processed on the next "ID_POSITION_UPDATE"
			JString dataString = ValueObject<JString>(hashData.getValue(0)).getDataCopy();
			RemotePlayerMap[networkId].sitAnim = dataString.UTF8Representation().cstr();
			Tests::debug("ID_SET_ANIMATION received.");
		}

		else if (eventCode == NetworkState::EV::ID_NPC_UPDATE)
		{
			Hashtable tNpcPlayerHash = ValueObject<Hashtable>(eventContent).getDataCopy();
			int64 networkId = ValueObject<int64>(tNpcPlayerHash.getValue<int>(tNpcPlayerHash.getSize() - 1)).getDataCopy();

			if (!IsSpawned(networkId) || IsLocationMaster())
				return;

			// We check to see if this player is enabled in our game. If they are then they are in the same location as us.
			if (IsEnabled(RemotePlayerMap[networkId].actor))
			{
				PlayerData tNpcPlayer;

				for (int i = 0; i < tNpcPlayerHash.getKeys().getSize() - 1; i++)
				{
					PlayerData::Deserialize(ValueObject<Hashtable>(tNpcPlayerHash.getValue(i)).getDataCopy(), tNpcPlayer);
					RemoteNpcUpdates.push(pair<UInt32, PlayerData>(networkId, tNpcPlayer));
				}
			}
			Tests::debug("ID_NPC_UPDATE received.");
		}
	}

	// Receive various events, these can all be converted and moved to "ReceiveEvent" at some point. As sending strings is unnecessary.
	inline static void OnPublicMessage(UInt32 networkId, JString message)
	{
		string sMesg = message.UTF8Representation();
		char* dest = (char*)sMesg.c_str();
		vector<string> values = split(dest, ',');

		if (values[0] == "StartSneaking")
			PushToSkyrim("StartSneaking", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "StopSneaking")
			PushToSkyrim("StopSneaking", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "StartJumping")
		{
			RemotePlayerMap[networkId].jumpZOrigin = RemotePlayerMap[networkId].actor->pos.z;
			PushToSkyrim("StartJumping", networkId, RemotePlayerMap[networkId].actor);
			RemotePlayerMap[networkId].jump = true;
		}
		else if (values[0] == "StartDraw")
			RemotePlayerMap[networkId].alert = true;
		else if (values[0] == "StartSheath")
		{
			RemotePlayerMap[networkId].alert = false;
			PushToSkyrim("StartSheath", networkId, RemotePlayerMap[networkId].actor);
		}
		else if (values[0] == "AttackRight")
			PushToSkyrim("AttackRight", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "AttackLeft")
			PushToSkyrim("AttackLeft", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "actorValues")
			PushToSkyrim("actorValues", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "AttackDual")
			PushToSkyrim("AttackDual", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "InterruptCast")
			PushToSkyrim("InterruptCast", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "InterruptCastLeft")
			PushToSkyrim("InterruptCastLeft", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "InterruptCastRight")
			PushToSkyrim("InterruptCastRight", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "CastLeft")
			PushToSkyrim("CastLeft", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "CastRight")
			PushToSkyrim("CastRight", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "DualCast")
			PushToSkyrim("DualCast", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "BlockStart")
			PushToSkyrim("BlockStart", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "BlockStop")
			PushToSkyrim("BlockStop", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "BowAttack")
			PushToSkyrim("BowAttack", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "FireBow")
			PushToSkyrim("FireBow", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "BashAttack")
			PushToSkyrim("BashAttack", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "ShoutRelease")
			PushToSkyrim("ShoutRelease", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "meleeDamage")
			PushToSkyrim("MeleeDamage", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "unarmedDamage")
			PushToSkyrim("UnarmedDamage", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "paralysis")
			PushToSkyrim("Paralysis", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "invisibility")
			PushToSkyrim("Invisibility", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "waterBreathing")
			PushToSkyrim("WaterBreathing", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "waterWalking")
			PushToSkyrim("WaterWalking", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "attackDamageMult")
			PushToSkyrim("AttackDamageMult", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "speedMult")
			PushToSkyrim("SpeedMult", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "damageResist")
			PushToSkyrim("DamageResist", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "magicResist")
			PushToSkyrim("MagicResist", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "equip")
			PushToSkyrim("Equip", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "equipspell")
			PushToSkyrim("EquipSpell", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "removeSpellLeft")
			PushToSkyrim("RemoveSpellLeft", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "removeSpellRight")
			PushToSkyrim("RemoveSpellRight", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "equipShout")
			PushToSkyrim("EquipShout", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "faceset")
			NativeFunctions::SetFaceTextureSet(RemotePlayerMap[networkId].object, FormID(strtoul(values[1].c_str(), nullptr, 0), networkId).getFull());
		else if (values[0] == "brow")
			PushToSkyrim("SetBrow", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "scar")
			PushToSkyrim("SetScar", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "beard")
			PushToSkyrim("SetBeard", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "hair")
			PushToSkyrim("SetHair", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "eyes")
			PushToSkyrim("SetEyes", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "head")
			PushToSkyrim("SetHead", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "mouth")
			PushToSkyrim("SetMouth", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "button")
			PushToSkyrim("ActivateButton", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "activate")
			PushToSkyrim("ActivateAnimation", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "setstage")
			PushToSkyrim("SetStage", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "ridehorse")
			PushToSkyrim("RideHorse", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "dismounthorse")
			PushToSkyrim("DismountHorse", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "getup")
			PushToSkyrim("Getup", networkId, RemotePlayerMap[networkId].actor);
		else if (values[0] == "dropItem")
			PushToSkyrim("DropItem", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "setpv")
			NativeFunctions::ExecuteCommand((char*)("SETPV " + values[1] + " " + values[2]).c_str(), RemotePlayerMap[networkId].object);
		else if (values[0] == "consolecommand")
		{
			std::string commandStripped = values[1].substr(7);
			NativeFunctions::ExecuteCommand((char*)commandStripped.c_str(), RemotePlayerMap[networkId].object);
		}
		else if (values[0] == "dFlag")
			PushToSkyrim("DeathFlag", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		else if (values[0] == "wthr")
		{
			//The weather is changed if the client is not the master
			if (!IsLocationMaster())
				PushToSkyrim("Weather", networkId, RemotePlayerMap[networkId].actor, *Utilities::StripFront(&values));
		}
		else
			return;
	}

	// Update other players of changes to local NPCs, if we are the location's master.
	inline static void SendNpcUpdate()
	{
		if (!IsLocationMaster() || IsAlone())
			return;
		
		Hashtable npcOutHash = Hashtable();
		int count = 0;
		for (map<UInt32, PlayerData>::iterator it = LocalNpcMap.begin(); it != LocalNpcMap.end(); ++it)
		{
			if (!it->second.actor || !IsEnabled(it->second.actor) || it->second.isDead)
				continue;

			npcOutHash.put<int, Hashtable>(count, it->second.Serialize());
			count++;
		}

		npcOutHash.put<int, int64>(count, NetworkState::networkId);

		if (count > 0)
			Networking::instance->sendEvent<Hashtable>(true, npcOutHash, NetworkState::EV::ID_NPC_UPDATE, NetworkState::CHANNEL::EVENT);
	}

	inline static void UpdateTOD()
	{
		// The host is responsible for updating the game time of everyone connected.
		if (Networking::instance->IsHost())
		{
			ExitGames::Common::Hashtable jUserVar = Hashtable();

			jUserVar.put<int, float>(0, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDaysPassed)));
			jUserVar.put<int, float>(1, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDay)));
			jUserVar.put<int, float>(2, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameHour)));
			jUserVar.put<int, float>(3, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameMonth)));
			jUserVar.put<int, float>(4, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameYear)));

			Networking::instance->sendEvent<Hashtable>(false, jUserVar, NetworkState::EV::ID_SET_TIME, NetworkState::CHANNEL::EVENT);
		}

		// One user per area is responsible for updating other nearby users of the current weather.
		if (dayTimer.HasMillisecondsPassed(10000))
		{
			if (IsLocationMaster())
			{
				TESWeather* weather = GetCurrentWeather(GameState::skyrimVMRegistry, 0, nullptr);

				Hashtable hashData = Hashtable();
				hashData.put<int, int64>(0, (int64)weather->formID);
				hashData.put<int, int64>(1, NetworkState::networkId);

				if (weather)
					Networking::instance->sendEvent<Hashtable>(true, hashData, NetworkState::EV::ID_SET_WEATHER, NetworkState::CHANNEL::EVENT);
			}

			dayTimer.StartTimer();
		}
	}

	// Update other players of changes to the local player's position/location.
	inline static void SendModifiedPosition(float x, float y, float z, int xRot, int yRot, int zRot)
	{
		Hashtable hashData = Hashtable();

		hashData.put<int, float>(0, x);
		hashData.put<int, float>(1, y);
		hashData.put<int, float>(2, z);
		hashData.put<int, int>(3, xRot);
		hashData.put<int, int>(4, yRot);
		hashData.put<int, int>(5, zRot);
		hashData.put<int, int64>(6, NetworkState::networkId);

		Networking::instance->sendEvent<Hashtable>(false, hashData, NetworkState::EV::ID_POSITION_UPDATE, NetworkState::CHANNEL::EVENT);
	}

	// Update other players of any local changes to locks within this location.
	inline static void SendLockMessage()
	{
		if (lockList.size() == 0)
			return;

		Hashtable hashData = Hashtable();

		int count = 0;
		for (int i = 0; i < lockList.size(); i++, count+=2)
		{
			hashData.put<int, int64>(count, lockList[i].id);
			hashData.put<int, bool>(count + 1, lockList[i].isLocked);
		}

		hashData.put<int, int64>(count + 2, NetworkState::networkId);

		Networking::instance->sendEvent<Hashtable>(true, hashData, NetworkState::EV::ID_LOCK_UPDATE, NetworkState::CHANNEL::EVENT);
		lockList.clear();
	}

	// Send the local player's current idle animation. This is for sitting, activation animations, etc... 
	inline static void SendAnimation(const char* anim)
	{
		Hashtable hashData = Hashtable();
		hashData.put<int, JString>(0, JString(anim));
		hashData.put<int, int64>(1, NetworkState::networkId);

		Networking::instance->sendEvent<Hashtable>(false, hashData, NetworkState::EV::ID_SET_ANIMATION, NetworkState::CHANNEL::EVENT);
	}

	// Send the local player state either for the first time or in response to a request from another player.
	inline static void SendPlayerSpawn(string name, string lorder, UInt32 raceId, string raceName, UInt32 sex, UInt32 weight, UInt32 rightWeaponId, UInt32 leftWeaponId,
		UInt32 headArmorId, UInt32 hairTypeId, UInt32 hairLongId, UInt32 bodyArmorId, UInt32 handsArmorId, UInt32 forearmArmorId, UInt32 amuletArmorId,
		UInt32 ringArmorId, UInt32 feetArmorId, UInt32 calvesArmorId, UInt32 shieldArmorId, UInt32 circletArmorId, UInt32 mouthId, UInt32 headId,
		UInt32 eyesId, UInt32 hairId, UInt32 beardId, UInt32 scarId, UInt32 browId, float height, UInt32 faceset, UInt32 hairColor, UInt32 voiceId)
	{
		ExitGames::Common::Hashtable jUserVar = Hashtable();

		jUserVar.put<int, JString>(0, JString(name.c_str()));
		jUserVar.put<int, JString>(1, JString(lorder.c_str()));
		jUserVar.put<int, int64>(2, raceId);
		jUserVar.put<int, JString>(3, JString(raceName.c_str()));
		jUserVar.put<int, int64>(4, sex);
		jUserVar.put<int, int64>(5, weight);
		jUserVar.put<int, int64>(6, rightWeaponId);
		jUserVar.put<int, int64>(7, leftWeaponId);
		jUserVar.put<int, int64>(8, headArmorId);
		jUserVar.put<int, int64>(9, hairTypeId);
		jUserVar.put<int, int64>(10, hairLongId);
		jUserVar.put<int, int64>(11, bodyArmorId);
		jUserVar.put<int, int64>(12, handsArmorId);
		jUserVar.put<int, int64>(13, forearmArmorId);
		jUserVar.put<int, int64>(14, amuletArmorId);
		jUserVar.put<int, int64>(15, ringArmorId);
		jUserVar.put<int, int64>(16, feetArmorId);
		jUserVar.put<int, int64>(17, calvesArmorId);
		jUserVar.put<int, int64>(18, shieldArmorId);
		jUserVar.put<int, int64>(19, circletArmorId);
		jUserVar.put<int, int64>(20, mouthId);
		jUserVar.put<int, int64>(21, headId);
		jUserVar.put<int, int64>(22, eyesId);
		jUserVar.put<int, int64>(23, hairId);
		jUserVar.put<int, int64>(24, beardId);
		jUserVar.put<int, int64>(25, scarId);
		jUserVar.put<int, int64>(26, browId);
		jUserVar.put<int, float>(27, height);
		jUserVar.put<int, int64>(28, faceset);
		jUserVar.put<int, int64>(29, hairColor);
		jUserVar.put<int, int64>(30, voiceId);
		jUserVar.put<int, int64>(31, NetworkState::networkId);

		// This is supposed to be cached, but the cached version results in a crash....
		Networking::instance->sendEvent<Hashtable>(true, jUserVar, NetworkState::EV::ID_SPAWN_PLAYER, NetworkState::CHANNEL::EVENT);
	}

	private:
		static bool locationMaster;
};
#endif
