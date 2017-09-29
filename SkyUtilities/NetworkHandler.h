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
static map<int, map<UInt32, UInt32>> modIdMap;

/* Stores the reference id (0x00FFFFFF) separate from the mod id (0xFF000000) so that we can use them separately
and universally between players. */
struct FormID
{
	FormID(UInt32 refId)
	{
		formId = refId - ((refId / 0x01000000) * 0x01000000);
		modId = refId - formId;
	}

	FormID(UInt32 refId, int playerNr)
	{
		formId = refId - ((refId / 0x01000000) * 0x01000000);
		modId = refId - formId;
		modId = (modIdMap[playerNr].count(modId) > 0 ? modIdMap[playerNr][modId] : modId);
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

	UInt32 convertFull(int playerNr)
	{
		modId = modIdMap[playerNr][modId];
		return modId + formId;
	}

	string convertFullString(int playerNr, bool decimal = false)
	{
		modId = (modIdMap[playerNr].count(modId) > 0 ? modIdMap[playerNr][modId] : modId);
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
	int x, y, z;
	float xRot, yRot, zRot, height, jumpZOrigin;
	float locationEntryTime;
	UInt32 formId, baseId, combatTarget, sitType, positionControllerFormId, targetControllerFormId;
	TESObjectCELL* lastCell;
	ActorEx *positionControllerActor, *actor;
	TESObjectREFR *targetController, *mount, *object, *positionController;
	bool isMounted, leftAttack, rightAttack, jump, alert, firstUpdate, equippedSpell, forceTeleport, spawned;
	string sitTarget, name, lastLocation, sitState, sitAnim;
	int64 location;

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

		serializedData.put<JString, int>("x", actor->pos.x);
		serializedData.put<JString, int>("y", actor->pos.y);
		serializedData.put<JString, int>("z", actor->pos.z);

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
		newPlayer.x = ValueObject<int>(serializedData.getValue("x")).getDataCopy();
		newPlayer.y = ValueObject<int>(serializedData.getValue("y")).getDataCopy();
		newPlayer.z = ValueObject<int>(serializedData.getValue("z")).getDataCopy();

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

	// RemotePlayerMap[playerNr] - A map of all of the currently connected players [PlayerData], accessed using their network "playerNr".
	static map<UInt32, PlayerData> RemotePlayerMap;
	/* LocalNpcMap[(Actor*)someNpc->formId] - "LocalNpcMap" is a map of all of the NPCs currently near the player, this map contains the NPCs that are 
	sent to remote players	when updating or spawning NPCs. RemoteNpcMap[(Actor*)someNpc->formId] - "RemoteNpcMap" is a map of all of the NPCs spawned 
	by other players, they exist if	we are not the host of the current location.*/
	static map<UInt32, PlayerData> LocalNpcMap, RemoteNpcMap;
	/* Contains pending NPC updates. Each entry in the queue contains the associated "playerNr", and the new [PlayerData] for the NPC.
	The updates are quickly pushed into the queue to minimize blocking, and processed individually in the main update loop. */
	static queue<pair<int, PlayerData>> RemoteNpcUpdates;
	// disabledNpcs[(Actor*)someNpc->formId] - All the currently disabled NPCs. Used to keep track of, and restore NPCs disabled by the network.
	static set<UInt32> disabledNpcs;
	// Will prevent the "PreventingUnauthorizedSpawn" function from disabling this NPC (referenceId). This should be cleared out as needed. (npcs no longer being needed)
	static map<UInt32, bool> ForceSpawn;
	// A simple list of player reference id's. Used to determine whether or not an npc is/was a player.
	static vector<UInt32> NetworkHandler::playerLookup;

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
	/* The time at the last update interval. The value of the global variable "GameDaysPassed" is used as the time reference, and is updated every ~1 sec */
	static float lastTimeReference;
	// The time we entered the current location. This value isn't realtime, it's assigned the most recent "lastTimeReference".
	static float locationEntryTime;

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
			if (RemotePlayerMap.count(ref->formID) == 0)
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
		RemoteNpcUpdates = queue<pair<int, PlayerData>>();

		if (RemotePlayerMap.size() > 0)
		{
			for (map<UInt32, PlayerData>::reverse_iterator it = RemotePlayerMap.rbegin(); it != RemotePlayerMap.rend(); ++it)
				OnExit(it->first);
			RemotePlayerMap.clear();
		}

		HasInitialized = false;
		locationMaster = false;
		NetworkHandler::locationEntryTime = INT_MAX;
		NetworkHandler::lastTimeReference = INT_MAX;
	}

	inline static void OnDisconnection()
	{
		PrintNote("Disconnected.");
	}

	inline static void NpcSpawn(PlayerData &remoteNpc, int playerNr)
	{
		FormID npcId = FormID(remoteNpc.formId, playerNr);
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

		TESForm* npcBaseForm = LookupFormByID(FormID(remoteNpc.baseId, playerNr).getFull());

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

	inline static void SpawnPlayer(const int playerNr, Hashtable* data)
	{
		if (PlayerRef == nullptr)
			PlayerRef = (TESObjectREFR*)(*g_thePlayer);

		string name = (string)ValueObject<JString>(data->getValue(0)).getDataCopy().UTF8Representation();
		string lorder = (string)ValueObject<JString>(data->getValue(1)).getDataCopy().UTF8Representation();
		FormID raceId = FormID((UInt32)ValueObject<int64>(data->getValue(2)).getDataCopy());
		string raceName = (string)ValueObject<JString>(data->getValue(3)).getDataCopy().UTF8Representation();
		UInt32 sex = (UInt32)ValueObject<int64>(data->getValue(4)).getDataCopy();
		UInt32 weight = (UInt32)ValueObject<int64>(data->getValue(5)).getDataCopy();
		FormID rightWeaponId = FormID((UInt32)ValueObject<int64>(data->getValue(6)).getDataCopy());
		FormID leftWeaponId = FormID((UInt32)ValueObject<int64>(data->getValue(7)).getDataCopy());
		FormID headArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(8)).getDataCopy());
		FormID hairTypeId = FormID((UInt32)ValueObject<int64>(data->getValue(9)).getDataCopy());
		FormID hairLongId = FormID((UInt32)ValueObject<int64>(data->getValue(10)).getDataCopy());
		FormID bodyArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(11)).getDataCopy());
		FormID handsArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(12)).getDataCopy());
		FormID forearmArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(13)).getDataCopy());
		FormID amuletArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(14)).getDataCopy());
		FormID ringArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(15)).getDataCopy());
		FormID feetArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(16)).getDataCopy());
		FormID calvesArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(17)).getDataCopy());
		FormID shieldArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(18)).getDataCopy());
		FormID circletArmorId = FormID((UInt32)ValueObject<int64>(data->getValue(19)).getDataCopy());
		FormID mouthId = FormID((UInt32)ValueObject<int64>(data->getValue(20)).getDataCopy());
		FormID headId = FormID((UInt32)ValueObject<int64>(data->getValue(21)).getDataCopy());
		FormID eyesId = FormID((UInt32)ValueObject<int64>(data->getValue(22)).getDataCopy());
		FormID hairId = FormID((UInt32)ValueObject<int64>(data->getValue(23)).getDataCopy());
		FormID beardId = FormID((UInt32)ValueObject<int64>(data->getValue(24)).getDataCopy());
		FormID scarId = FormID((UInt32)ValueObject<int64>(data->getValue(25)).getDataCopy());
		FormID browId = FormID((UInt32)ValueObject<int64>(data->getValue(26)).getDataCopy());
		float height = (float)ValueObject<float>(data->getValue(27)).getDataCopy();
		FormID faceset = FormID((UInt32)ValueObject<int64>(data->getValue(28)).getDataCopy());
		UInt32 hairColor = (UInt32)ValueObject<int64>(data->getValue(29)).getDataCopy();
		FormID voiceId = FormID((UInt32)ValueObject<int64>(data->getValue(30)).getDataCopy());

		// This maps the remote player's load order to our local remote order. Mapping the prefixes from their game to ours. 
		std::map<std::string, std::string> plModMap;
		vector<string> values = split(lorder, ',');

		for (int i = 1; i < values.size(); i++)
		{
			for (int j = 0; j < myLoadOrder.size(); j++)
			{
				if (values[i] == myLoadOrder[j])
				{
					// modIdMap[playerNr][remotePlayerModId] = localModId
					modIdMap[playerNr][(i - 1) * 0x01000000] = (j * 0x01000000);
					break;
				}
			}
		}

		vector<string> spawnValues{ raceId.convertFullString(playerNr, true), to_string(sex), raceName, 
			rightWeaponId.convertFullString(playerNr, true), leftWeaponId.convertFullString(playerNr, true), headArmorId.convertFullString(playerNr, true),
			hairTypeId.convertFullString(playerNr, true),	hairLongId.convertFullString(playerNr, true), bodyArmorId.convertFullString(playerNr, true),
			handsArmorId.convertFullString(playerNr, true), forearmArmorId.convertFullString(playerNr, true), amuletArmorId.convertFullString(playerNr, true),	
			ringArmorId.convertFullString(playerNr, true), feetArmorId.convertFullString(playerNr, true), calvesArmorId.convertFullString(playerNr, true),
			shieldArmorId.convertFullString(playerNr, true), circletArmorId.convertFullString(playerNr, true), mouthId.convertFullString(playerNr, true), 
			headId.convertFullString(playerNr, true), eyesId.convertFullString(playerNr, true), hairId.convertFullString(playerNr, true), 
			beardId.convertFullString(playerNr, true), scarId.convertFullString(playerNr, true), browId.convertFullString(playerNr, true), to_string(height), 
			faceset.convertFullString(playerNr, true), to_string(hairColor), voiceId.convertFullString(playerNr, true), to_string(weight), name, to_string(playerNr) };

		PushToSkyrim("SpawnPlayer", playerNr, PlayerRef, spawnValues);
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
	inline static void OnExit(int playerNr)
	{
		RemotePlayerMap[playerNr].disabled = true;
		DisableNet(RemotePlayerMap[playerNr].actor);
		MoveTo(GameState::skyrimVMRegistry, 0, RemotePlayerMap[playerNr].actor, (TESObjectREFR*)LookupFormByID(ID_TESObjectREFR::E3demoMarker), 0, 0, 0, true);
		NativeFunctions::ExecuteCommand("APS SkyTools DisableTracking", RemotePlayerMap[playerNr].actor);
	}

	// Finalize the player's spawn
	inline static void InitializeNewPlayer(StaticFunctionTag* base, UInt32 newPlayer, UInt32 movementController, UInt32 targetController, UInt32 bUser)
	{
		RemotePlayerMap[bUser].networkId = bUser;
		RemotePlayerMap[bUser].disabled = false;
		RemotePlayerMap[bUser].actor = (ActorEx*)LookupFormByID(newPlayer);
		RemotePlayerMap[bUser].object = RemotePlayerMap[bUser].actor;
		RemotePlayerMap[bUser].lastLocation = "";
		RemotePlayerMap[bUser].lastCell = RemotePlayerMap[bUser].actor->parentCell;
		RemotePlayerMap[bUser].positionController = (TESObjectREFR*)LookupFormByID(movementController);
		RemotePlayerMap[bUser].positionControllerActor = (ActorEx*)RemotePlayerMap[bUser].positionController;
		RemotePlayerMap[bUser].forceTeleport = false;
		RemotePlayerMap[bUser].height = ((TESNPC*)RemotePlayerMap[bUser].actor->baseForm)->height;
		RemotePlayerMap[bUser].formId = newPlayer;
		RemotePlayerMap[bUser].positionControllerFormId = RemotePlayerMap[bUser].positionController->formID;
		RemotePlayerMap[bUser].targetController = (TESObjectREFR*)LookupFormByID(targetController);
		RemotePlayerMap[bUser].targetControllerFormId = RemotePlayerMap[bUser].targetController->formID;
		RemotePlayerMap[bUser].equippedSpell = false;
		RemotePlayerMap[bUser].mount = NULL;
		RemotePlayerMap[bUser].isMounted = false;
		RemotePlayerMap[bUser].leftAttack = false;
		RemotePlayerMap[bUser].rightAttack = false;
		RemotePlayerMap[bUser].jump = false;
		RemotePlayerMap[bUser].firstUpdate = true;
		RemotePlayerMap[bUser].alert = false;
		RemotePlayerMap[bUser].sitType = 0;
		RemotePlayerMap[bUser].locationEntryTime = INT_MIN;

		// These values cannot be set in papyrus, they lead to errors.
		NativeFunctions::ExecuteCommand("forceav Confidence 4", RemotePlayerMap[bUser].actor);
		NativeFunctions::ExecuteCommand("forceav Aggression 0", RemotePlayerMap[bUser].actor);

		// Ensure that the player isn't being treated like an NPC
		if (LocalNpcMap.count(RemotePlayerMap[bUser].formId))
		{
			for (map<UInt32, PlayerData>::iterator it = LocalNpcMap.begin(); it != LocalNpcMap.end(); ++it)
			{
				if (it->first == RemotePlayerMap[bUser].formId)
				{
					LocalNpcMap.erase(RemotePlayerMap[bUser].formId);
					break;
				}
			}
		}

		RemotePlayerMap[bUser].spawned = true;
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
		if (Networking::instance->getPlayerCount() > 0)
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

	inline static void UpdateNpc(PlayerData &tNpc, int playerNr)
	{
		UInt32 npcNum = FormID(tNpc.formId, playerNr).getFull();

		//If the NPC is new, spawn them and continue.
		if (!RemoteNpcMap[npcNum].spawned)
		{
			NpcSpawn(tNpc, playerNr);
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
			int interX = (tNpc.x - RemoteNpcMap[npcNum].x) + tNpc.x;
			int interY = (tNpc.y - RemoteNpcMap[npcNum].y) + tNpc.y;
			int interZ = (tNpc.z - RemoteNpcMap[npcNum].z) + tNpc.z;

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
	inline static bool GetRemotePlayerDataBool(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "isMounted")
			return RemotePlayerMap[playerNr].isMounted;
		else if (normalizedName == "alert")
			return RemotePlayerMap[playerNr].alert;
		else if (normalizedName == "firstUpdate")
			return RemotePlayerMap[playerNr].firstUpdate;
		else if (normalizedName == "forceTeleport")
			return RemotePlayerMap[playerNr].forceTeleport;
		else if (normalizedName == "disabled")
			return RemotePlayerMap[playerNr].disabled;
		else if (normalizedName == "jump")
			return RemotePlayerMap[playerNr].jump;
		else if (normalizedName == "equippedSpell")
			return RemotePlayerMap[playerNr].equippedSpell;
		else
			return false;
	}

	// Papyrus interface
	inline static float GetRemotePlayerDataFloat(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "height")
			return RemotePlayerMap[playerNr].height;
		else if (normalizedName == "jumpZOrigin")
			return RemotePlayerMap[playerNr].jumpZOrigin;
		else
			return -1;
	}

	// Papyrus interface
	inline static UInt32 GetRemotePlayerDataInt(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName)
	{
		string normalizedName = tName.data;

		if (normalizedName == "sitType")
			return RemotePlayerMap[playerNr].sitType;
		else
			return -1;
	}

	// Papyrus interface
	inline static BSFixedString GetRemotePlayerDataString(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName)
	{
		string normalizedName = tName.data;
		
		if (normalizedName == "lastLocation")
			return RemotePlayerMap[playerNr].lastLocation.c_str();
		else if (normalizedName == "locationid")
			return to_string(NetworkState::locationId).c_str();
		else if (normalizedName == "location")
			return to_string(RemotePlayerMap[playerNr].location).c_str();
		else if (normalizedName  == "sitAnim")
			return RemotePlayerMap[playerNr].sitAnim.c_str();
		else if (normalizedName == "horse")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[playerNr].mount->formID)).c_str();
		else if (normalizedName == "targetController")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[playerNr].targetController->formID)).c_str();
		else if (normalizedName == "positionController")
			return ((string)"0x" + Utilities::GetFormIDString(RemotePlayerMap[playerNr].positionController->formID)).c_str();
		else
			return "";
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataBool(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName, bool val)
	{
		string normalizedName = tName.data;
		UInt32 refId = playerNr; //To clarify when we're using a refId in place of the player number for papyrus global function calls.

		if (normalizedName == "jump")
			RemotePlayerMap[playerNr].jump = val;
		else if (normalizedName == "alert")
			RemotePlayerMap[playerNr].alert = val;
		else if (normalizedName == "equippedSpell")
			RemotePlayerMap[playerNr].equippedSpell = val;
		else if (normalizedName == "isMounted")
			RemotePlayerMap[playerNr].isMounted = val;
		else if (normalizedName == "firstUpdate")
			RemotePlayerMap[playerNr].firstUpdate = val;
		else if (normalizedName == "forceTeleport")
			RemotePlayerMap[playerNr].forceTeleport = val;
		else if (normalizedName == "ignoreSpawns")
			ignoreSpawns = val;
		else if (normalizedName == "ForceSpawn")
			ForceSpawn[refId] = val;
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataFloat(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName, float val)
	{
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataInt(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName, UInt32 val)
	{
		string normalizedName = tName.data;

		if (normalizedName == "sitType")
			RemotePlayerMap[playerNr].sitType = val;
		else if (normalizedName == "horse")
			RemotePlayerMap[playerNr].mount = (TESObjectREFR*)LookupFormByID(val);
		else if (normalizedName == "Player")
			playerLookup.push_back(val);
	}

	// Papyrus interface
	inline static void SetRemotePlayerDataString(StaticFunctionTag* base, UInt32 playerNr, BSFixedString tName, BSFixedString val)
	{
		string normalizedName = tName.data;

		if (normalizedName == "name")
			RemotePlayerMap[playerNr].name = val.data;
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

		JString jData = data;
		Networking::instance->sendEvent<JString>(true, jData, NetworkState::EV::ID_MESSAGE, NetworkState::CHANNEL::EVENT);
	}

	// Enables/disables NPCs that players that enter/leave the current area. Determines the current locations master for NPC synchronization, weather, etc...
	inline static void OnLocationUpdate()
	{
		if (IsAlone())
			return;

		for (map<UInt32, PlayerData>::iterator it = NetworkHandler::RemotePlayerMap.begin(); it != NetworkHandler::RemotePlayerMap.end(); ++it)
		{
			if (IsSpawned(it->first))
			{
				if (it->second.disabled)
				{
					EnableNet(it->second.actor);
					it->second.disabled = false;
					it->second.forceTeleport = true;
				}
			}
		}

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

	inline static void ProcessLock(int playerNr, Hashtable* data)
	{
		UInt32 lockId;
		bool isLocked;

		for (int i = 0; i < data->getSize(); i += 2)
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

	inline static bool IsSpawned(const int playerNr)
	{
		if (RemotePlayerMap.count(playerNr) < 1 || !RemotePlayerMap[playerNr].spawned || !RemotePlayerMap[playerNr].actor)
			return false;
		return true;
	}

	inline static void ReceiveEvent(const int playerNr, const nByte eventCode, const Object& eventContent)
	{
		if (eventCode == NetworkState::EV::ID_MESSAGE)
		{
			if (!IsSpawned(playerNr))
				return;

			OnPublicMessage(playerNr, ValueObject<JString>(eventContent).getDataCopy());
			Tests::debug("ID_MESSAGE received.");
		}

		else if (eventCode == NetworkState::EV::ID_SPAWN_PLAYER)
		{
			if (IsSpawned(playerNr))
				return;

			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();
			SpawnPlayer(playerNr, &hashData);
			Tests::debug("ID_SPAWN_PLAYER received");
		}

		else if (eventCode == NetworkState::EV::ID_LOCK_UPDATE)
		{
			if (!IsSpawned(playerNr))
				return;

			Hashtable lockData = ValueObject<Hashtable>(eventContent).getDataCopy();
			ProcessLock(playerNr, &lockData);
			Tests::debug("ID_LOCK_UPDATE received.");
		}

		else if (eventCode == NetworkState::EV::ID_POSITION_UPDATE)
		{
			if (!IsSpawned(playerNr))
				return;

			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();

			int x = ValueObject<int>(hashData.getValue(0)).getDataCopy(), 
				y = ValueObject<int>(hashData.getValue(1)).getDataCopy(), 
				z = ValueObject<int>(hashData.getValue(2)).getDataCopy(), 
				xRot = ValueObject<int>(hashData.getValue(3)).getDataCopy(),
				yRot = ValueObject<int>(hashData.getValue(4)).getDataCopy(),
				zRot = ValueObject<int>(hashData.getValue(5)).getDataCopy();

			float newLocationEntryTime = ValueObject<float>(hashData.getValue(6)).getDataCopy();

			//If the player has already spawned, update their location data. And unlock their movement.
			RemotePlayerMap[playerNr].locationEntryTime = newLocationEntryTime;
			OnLocationUpdate();

			Tests::debug("ID_POSITION_UPDATE received.");

			if (IsEnabled(RemotePlayerMap[playerNr].actor))
			{
				Tests::debug("ID_POSITION_UPDATE processing.");
				//Move the position controller to the player's worldspace/cell.
				if (RemotePlayerMap[playerNr].forceTeleport)
				{
					NativeFunctions::ExecuteCommand("MoveTo player 0 -2000 0", RemotePlayerMap[playerNr].positionController);
					NativeFunctions::ExecuteCommand("MoveTo player 0 -2000 0", RemotePlayerMap[playerNr].actor);
					RemotePlayerMap[playerNr].sitType = 0;
					RemotePlayerMap[playerNr].forceTeleport = false;
				}

				// Update the position to guide the Actor towards the positionController.
				NativeFunctions::ExecuteCommand(("SetPos x " + to_string(x)).c_str(), RemotePlayerMap[playerNr].positionController);
				NativeFunctions::ExecuteCommand(("SetPos y " + to_string(y)).c_str(), RemotePlayerMap[playerNr].positionController);
				NativeFunctions::ExecuteCommand(("SetPos z " + to_string(z)).c_str(), RemotePlayerMap[playerNr].positionController);

				/* If someone would like to test a quest based replacement for this that would be great, as "KeepOffsetFromActor" does no obstacle avoidance.
				Which is good, and bad I suppose. Additionally "KeepOffsetFromActor" requires successive calls to ensure that the built in AI does not take over. */
				KOFA(RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].positionController);

				if (DistanceExceeded(RemotePlayerMap[playerNr].positionController, RemotePlayerMap[playerNr].actor, 1024))
				{
					NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[playerNr].positionController->formID) + " 0 0 0").c_str(), RemotePlayerMap[playerNr].actor);
					KOFA(RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].positionController);
				}

				if (RemotePlayerMap[playerNr].sitType != 0 || RemotePlayerMap[playerNr].firstUpdate)
				{
					NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[playerNr].positionController->formID) + " 0 0 0").c_str(), RemotePlayerMap[playerNr].actor);
					RemotePlayerMap[playerNr].firstUpdate = false;
					KOFA(RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].positionController);
				}

				if (RemotePlayerMap[playerNr].actor->IsOnMount())
				{
					NativeFunctions::ExecuteCommand(("SetAngle x " + to_string(xRot)).c_str(), RemotePlayerMap[playerNr].mount);
					NativeFunctions::ExecuteCommand(("SetAngle y " + to_string(yRot)).c_str(), RemotePlayerMap[playerNr].mount);
					NativeFunctions::ExecuteCommand(("SetAngle z " + to_string(zRot)).c_str(), RemotePlayerMap[playerNr].mount);
				}
				else
				{
					NativeFunctions::ExecuteCommand(("SetAngle x " + to_string(xRot)).c_str(), RemotePlayerMap[playerNr].actor);
					NativeFunctions::ExecuteCommand(("SetAngle y " + to_string(yRot)).c_str(), RemotePlayerMap[playerNr].actor);
					NativeFunctions::ExecuteCommand(("SetAngle z " + to_string(zRot)).c_str(), RemotePlayerMap[playerNr].actor);
				}

				if (RemotePlayerMap[playerNr].sitAnim == "")
				{
					// The Actor exits a sit, or idle animation.
					if (RemotePlayerMap[playerNr].sitType == 1000)
					{
						SetDontMove(GameState::skyrimVMRegistry, 0, RemotePlayerMap[playerNr].actor, false);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, "IdleForceDefaultState");
						NativeFunctions::ExecuteCommand("SetUnconscious 0", RemotePlayerMap[playerNr].actor);
						RemotePlayerMap[playerNr].sitType = 0;
					}
					// Shake the Actor out of its current sit/idle animation if it isn't supposed to be sitting/idling.
					else if (RemotePlayerMap[playerNr].sitType == 0)
					{
						if (GetSitAnimation(NULL, RemotePlayerMap[playerNr].actor).data && GetSitAnimation(NULL, RemotePlayerMap[playerNr].actor).data != "")
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, "IdleForceDefaultState");
					}
				}

				else
				{
					// The Actor enters the a new sit, or idle animation.
					if (RemotePlayerMap[playerNr].sitType == 0)
					{
						RemotePlayerMap[playerNr].sitType = 1000;
						SetDontMove(GameState::skyrimVMRegistry, 0, RemotePlayerMap[playerNr].actor, true);
						RemotePlayerMap[playerNr].actor->ClearKeepOffsetFromActor();
						NativeFunctions::ExecuteCommand("SetUnconscious 1", RemotePlayerMap[playerNr].actor);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].sitAnim.c_str());
					}
				}

				// Used to alter where the Actor looks, and is used when the Actor is casting magic, or firing a bow. This is unfinished.
				float xOffset = 1000 * sin(yRot), yOffset = 1000 * cos(yRot), zOffset = yRot + (RemotePlayerMap[playerNr].height * 0.75);

				//Move target controller
				NativeFunctions::ExecuteCommand(("MoveTo " + Utilities::GetFormIDString(RemotePlayerMap[playerNr].actor->formID) + " " + to_string(xOffset) + " " + to_string(yOffset) + " " + to_string(zOffset)).c_str(), RemotePlayerMap[playerNr].targetController);

				//SetLookAt(GameState::skyrimVMRegistry, 0, RemotePlayerMap[playerNr].actor, RemotePlayerMap[playerNr].targetController, false);

				// Signals the end of the Actor's current jump state.
				if (RemotePlayerMap[playerNr].jump && RemotePlayerMap[playerNr].actor->pos.z <= RemotePlayerMap[playerNr].jumpZOrigin)
					RemotePlayerMap[playerNr].jump = false;

				// Toggle the actor's alert state.
				if (RemotePlayerMap[playerNr].actor->actorState.IsWeaponDrawn())
				{
					if (!RemotePlayerMap[playerNr].alert)
					{
						NativeFunctions::ExecuteCommand("SetAlert 0", RemotePlayerMap[playerNr].actor);
						NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, "Unequip");
					}
				}

				else
				{
					if (RemotePlayerMap[playerNr].alert)
					{
						NativeFunctions::ExecuteCommand("SetAlert 1", RemotePlayerMap[playerNr].actor);

						if (RemotePlayerMap[playerNr].equippedSpell)
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, "Magic_Equip");
						else
							NativeFunctions::SendAnimationEvent(RemotePlayerMap[playerNr].actor, "weapEquip");
					}
				}
			}
		}

		else if (eventCode == NetworkState::EV::ID_SET_TIME)
		{
			Hashtable hashData = ValueObject<Hashtable>(eventContent).getDataCopy();

			lastTimeReference = (float)ValueObject<float>(hashData.getValue(0)).getDataCopy();
			float tDay = (float)ValueObject<float>(hashData.getValue(1)).getDataCopy(),
				tHour = (float)ValueObject<float>(hashData.getValue(2)).getDataCopy(),
				tMonth = (float)ValueObject<float>(hashData.getValue(3)).getDataCopy(),
				tYear = (float)ValueObject<float>(hashData.getValue(4)).getDataCopy();

			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDaysPassed), lastTimeReference);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameHour), tHour);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDay), tDay);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameMonth), tMonth);
			SetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameYear), tYear);

			//If we are not the host, update our location entry time for the first time.
			if (locationEntryTime == INT_MAX)
				locationEntryTime = lastTimeReference;

			Tests::debug("ID_SET_TIME received.");
		}

		else if (eventCode == NetworkState::EV::ID_SET_WEATHER)
		{
			if (!IsSpawned(playerNr))
				return;

			UInt32 weatherId = (int64)ValueObject<int64>(eventContent).getDataCopy();
			if (!IsLocationMaster() && IsEnabled(RemotePlayerMap[playerNr].actor))
				PushToSkyrim("Weather", playerNr, *g_thePlayer, vector<string>{ to_string(weatherId) });

			Tests::debug("ID_SET_WEATHER received.");
		}

		else if (eventCode == NetworkState::EV::ID_SET_ANIMATION)
		{
			// The player's animation is set here, and processed on the next "ID_POSITION_UPDATE"
			JString dataString = ValueObject<JString>(eventContent).getDataCopy();
			RemotePlayerMap[playerNr].sitAnim = dataString.UTF8Representation().cstr();
			Tests::debug("ID_SET_ANIMATION received.");
		}

		else if (eventCode == NetworkState::EV::ID_NPC_UPDATE)
		{
			if (!IsSpawned(playerNr) || IsLocationMaster())
				return;

			// We check to see if this player is enabled in our game. If they are then they are in the same location as us.
			if (IsEnabled(RemotePlayerMap[playerNr].actor))
			{
				Hashtable tNpcPlayerHash = ValueObject<Hashtable>(eventContent).getDataCopy();
				PlayerData tNpcPlayer;

				for (int i = 0; i < tNpcPlayerHash.getKeys().getSize(); i++)
				{
					PlayerData::Deserialize(ValueObject<Hashtable>(tNpcPlayerHash.getValue(i)).getDataCopy(), tNpcPlayer);
					RemoteNpcUpdates.push(pair<int, PlayerData>(playerNr, tNpcPlayer));
				}
			}
			Tests::debug("ID_NPC_UPDATE received.");
		}
	}

	// Receive various events, these can all be converted and moved to "ReceiveEvent" at some point. As sending strings is unnecessary.
	inline static void OnPublicMessage(int playerNr, JString message)
	{
		string sMesg = message.UTF8Representation();
		char* dest = (char*)sMesg.c_str();
		vector<string> values = split(dest, ',');
		int id = playerNr;

		if (values[0] == "StartSneaking")
			PushToSkyrim("StartSneaking", id, RemotePlayerMap[id].actor);
		else if (values[0] == "StopSneaking")
			PushToSkyrim("StopSneaking", id, RemotePlayerMap[id].actor);
		else if (values[0] == "StartJumping")
		{
			RemotePlayerMap[id].jumpZOrigin = RemotePlayerMap[id].actor->pos.z;
			PushToSkyrim("StartJumping", id, RemotePlayerMap[id].actor);
			RemotePlayerMap[id].jump = true;
		}
		else if (values[0] == "StartDraw")
			RemotePlayerMap[playerNr].alert = true;
		else if (values[0] == "StartSheath")
		{
			RemotePlayerMap[playerNr].alert = false;
			PushToSkyrim("StartSheath", id, RemotePlayerMap[id].actor);
		}
		else if (values[0] == "AttackRight")
			PushToSkyrim("AttackRight", id, RemotePlayerMap[id].actor);
		else if (values[0] == "AttackLeft")
			PushToSkyrim("AttackLeft", id, RemotePlayerMap[id].actor);
		else if (values[0] == "actorValues")
			PushToSkyrim("actorValues", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "AttackDual")
			PushToSkyrim("AttackDual", id, RemotePlayerMap[id].actor);
		else if (values[0] == "InterruptCast")
			PushToSkyrim("InterruptCast", id, RemotePlayerMap[id].actor);
		else if (values[0] == "InterruptCastLeft")
			PushToSkyrim("InterruptCastLeft", id, RemotePlayerMap[id].actor);
		else if (values[0] == "InterruptCastRight")
			PushToSkyrim("InterruptCastRight", id, RemotePlayerMap[id].actor);
		else if (values[0] == "CastLeft")
			PushToSkyrim("CastLeft", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "CastRight")
			PushToSkyrim("CastRight", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "DualCast")
			PushToSkyrim("DualCast", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "BlockStart")
			PushToSkyrim("BlockStart", id, RemotePlayerMap[id].actor);
		else if (values[0] == "BlockStop")
			PushToSkyrim("BlockStop", id, RemotePlayerMap[id].actor);
		else if (values[0] == "BowAttack")
			PushToSkyrim("BowAttack", id, RemotePlayerMap[id].actor);
		else if (values[0] == "FireBow")
			PushToSkyrim("FireBow", id, RemotePlayerMap[id].actor);
		else if (values[0] == "BashAttack")
			PushToSkyrim("BashAttack", id, RemotePlayerMap[id].actor);
		else if (values[0] == "ShoutRelease")
			PushToSkyrim("ShoutRelease", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "meleeDamage")
			PushToSkyrim("MeleeDamage", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "unarmedDamage")
			PushToSkyrim("UnarmedDamage", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "paralysis")
			PushToSkyrim("Paralysis", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "invisibility")
			PushToSkyrim("Invisibility", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "waterBreathing")
			PushToSkyrim("WaterBreathing", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "waterWalking")
			PushToSkyrim("WaterWalking", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "attackDamageMult")
			PushToSkyrim("AttackDamageMult", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "speedMult")
			PushToSkyrim("SpeedMult", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "damageResist")
			PushToSkyrim("DamageResist", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "magicResist")
			PushToSkyrim("MagicResist", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "equip")
			PushToSkyrim("Equip", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "equipspell")
			PushToSkyrim("EquipSpell", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "removeSpellLeft")
			PushToSkyrim("RemoveSpellLeft", id, RemotePlayerMap[id].actor);
		else if (values[0] == "removeSpellRight")
			PushToSkyrim("RemoveSpellRight", id, RemotePlayerMap[id].actor);
		else if (values[0] == "equipShout")
			PushToSkyrim("EquipShout", id, RemotePlayerMap[id].actor);
		else if (values[0] == "faceset")
			NativeFunctions::SetFaceTextureSet(RemotePlayerMap[id].object, FormID(strtoul(values[1].c_str(), nullptr, 0), playerNr).getFull());
		else if (values[0] == "brow")
			PushToSkyrim("SetBrow", id, RemotePlayerMap[id].actor);
		else if (values[0] == "scar")
			PushToSkyrim("SetScar", id, RemotePlayerMap[id].actor);
		else if (values[0] == "beard")
			PushToSkyrim("SetBeard", id, RemotePlayerMap[id].actor);
		else if (values[0] == "hair")
			PushToSkyrim("SetHair", id, RemotePlayerMap[id].actor);
		else if (values[0] == "eyes")
			PushToSkyrim("SetEyes", id, RemotePlayerMap[id].actor);
		else if (values[0] == "head")
			PushToSkyrim("SetHead", id, RemotePlayerMap[id].actor);
		else if (values[0] == "mouth")
			PushToSkyrim("SetMouth", id, RemotePlayerMap[id].actor);
		else if (values[0] == "button")
			PushToSkyrim("ActivateButton", id, RemotePlayerMap[id].actor);
		else if (values[0] == "activate")
			PushToSkyrim("ActivateAnimation", id, RemotePlayerMap[id].actor);
		else if (values[0] == "setstage")
			PushToSkyrim("SetStage", id, RemotePlayerMap[id].actor);
		else if (values[0] == "ridehorse")
			PushToSkyrim("RideHorse", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "dismounthorse")
			PushToSkyrim("DismountHorse", id, RemotePlayerMap[id].actor);
		else if (values[0] == "getup")
			PushToSkyrim("Getup", id, RemotePlayerMap[id].actor);
		else if (values[0] == "dropItem")
			PushToSkyrim("DropItem", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "setpv")
			NativeFunctions::ExecuteCommand((char*)("SETPV " + values[1] + " " + values[2]).c_str(), RemotePlayerMap[id].object);
		else if (values[0] == "consolecommand")
		{
			std::string commandStripped = values[1].substr(7);
			NativeFunctions::ExecuteCommand((char*)commandStripped.c_str(), RemotePlayerMap[id].object);
		}
		else if (values[0] == "dFlag")
			PushToSkyrim("DeathFlag", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
		else if (values[0] == "wthr")
		{
			//The weather is changed if the client is not the master
			if (!IsLocationMaster())
				PushToSkyrim("Weather", id, RemotePlayerMap[id].actor, *Utilities::StripFront(&values));
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

		if (count > 0)
			Networking::instance->sendEvent<Hashtable>(true, npcOutHash, NetworkState::EV::ID_NPC_UPDATE, NetworkState::CHANNEL::EVENT);
	}

	inline static void UpdateTOD()
	{
		// The host is responsible for updating the game time of everyone connected.
		if (Networking::instance->IsHost())
		{
			lastTimeReference = GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDaysPassed));

			ExitGames::Common::Hashtable jUserVar = Hashtable();

			jUserVar.put<int, float>(0, lastTimeReference);
			jUserVar.put<int, float>(1, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameDay)));
			jUserVar.put<int, float>(2, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameHour)));
			jUserVar.put<int, float>(3, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameMonth)));
			jUserVar.put<int, float>(4, GetValue(GameState::skyrimVMRegistry, 0, (TESGlobal*)LookupFormByID(ID_TESGlobal::GameYear)));

			Networking::instance->sendEvent<Hashtable>(false, jUserVar, NetworkState::EV::ID_SET_TIME, NetworkState::CHANNEL::EVENT);

			//If we are the host, update our location entry time for the first time.
			if (locationEntryTime == INT_MAX)
				locationEntryTime = lastTimeReference - 1;
		}

		// One user per area is responsible for updating other nearby users of the current weather.
		if (dayTimer.HasMillisecondsPassed(10000))
		{
			if (IsLocationMaster())
			{
				TESWeather* weather = GetCurrentWeather(GameState::skyrimVMRegistry, 0, nullptr);

				if (weather)
					Networking::instance->sendEvent<int64>(true, (int64)weather->formID, NetworkState::EV::ID_SET_WEATHER, NetworkState::CHANNEL::EVENT);
			}

			dayTimer.StartTimer();
		}
	}

	// Update other players of changes to the local player's position/location.
	inline static void SendModifiedPosition(int x, int y, int z, int xRot, int yRot, int zRot)
	{
		Hashtable hashData = Hashtable();

		hashData.put<int, int>(0, x);
		hashData.put<int, int>(1, y);
		hashData.put<int, int>(2, z);
		hashData.put<int, int>(3, xRot);
		hashData.put<int, int>(4, yRot);
		hashData.put<int, int>(5, zRot);
		hashData.put<int, float>(6, locationEntryTime);

		Networking::instance->sendEvent<Hashtable>(false, hashData, NetworkState::EV::ID_POSITION_UPDATE, NetworkState::CHANNEL::EVENT);
	}

	// Update other players of any local changes to locks within this location.
	inline static void SendLockMessage()
	{
		if (lockList.size() == 0)
			return;

		Hashtable hashData = Hashtable();

		for (int i = 0, count = 0; i < lockList.size(); i++, count+=2)
		{
			hashData.put<int, int64>(count, lockList[i].id);
			hashData.put<int, bool>(count + 1, lockList[i].isLocked);
		}

		Networking::instance->sendEvent<Hashtable>(true, hashData, NetworkState::EV::ID_LOCK_UPDATE, NetworkState::CHANNEL::EVENT);
		lockList.clear();
	}

	// Send the local player's current idle animation. This is for sitting, activation animations, etc... 
	inline static void SendAnimation(const char* anim)
	{
		Networking::instance->sendEvent<JString>(false, JString(anim), NetworkState::EV::ID_SET_ANIMATION, NetworkState::CHANNEL::EVENT);
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

		// This is supposed to be cached, but the cached version results in a crash....
		Networking::instance->sendEventCached<Hashtable>(true, jUserVar, NetworkState::EV::ID_SPAWN_PLAYER, NetworkState::CHANNEL::EVENT);
	}

	private:
		static bool locationMaster;
};
#endif
