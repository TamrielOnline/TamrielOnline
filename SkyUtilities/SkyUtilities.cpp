#include "SkyUtilities.h"
#include "md5.h"
#include <atlstr.h>
#include <strsafe.h>
#include <map>
#include <stdlib.h>
#include <signal.h>
#include <mutex>
#include "Tests.h"

std::shared_ptr<SkyUtility> SkyUtility::instance;

bool SkyUtility::IsPlayerWeaponDrawn()
{
	return (*g_thePlayer)->actorState.IsWeaponDrawn();
}

//Is this key the same as the key being pressed in the event?
bool SkyUtility::IsEventKey(InputEvent** evn, UInt32 keyValue)
{
	ButtonEvent* e = (ButtonEvent*)*evn;

	// Make sure this is really a button event
	if (!e || e->eventType != InputEvent::kEventType_Button)
		return false;

	UInt32 keyCode = e->keyMask;

	if (keyCode == keyValue)
		return true;
	return false;
}

bool SkyUtility::IsKeyPressed(const char* localKey)
{
	return NativeFunctions::IsKeyPressed(nullptr, NativeFunctions::GetMappedKey(nullptr, localKey, 0xFF));
}

UInt32 SkyUtility::GetStartupKey()
{
	return clientKey;
}

void SkyUtility::ReceiveEvent(const int playerNr, const nByte eventCode, const Object& eventContent)
{
	NetworkHandler::ReceiveEvent(playerNr, eventCode, eventContent);

	if (eventCode == NetworkState::EV::ID_SPAWN_PLAYER)
		instance->GetInitialPlayerData(true); //Send our spawn data directly to the player.
}

void SkyUtility::SetupCallbacks()
{
	Networking::instance->_MESSAGE = NetworkHandler::PrintNote;
	Networking::instance->OnConnected = OnConnected;
	Networking::instance->OnDisconnected = OnDisconnected;
	Networking::instance->OnReceiveEvent = SkyUtility::ReceiveEvent;
	Networking::instance->OnExit = NetworkHandler::OnExit;
}

void SkyUtility::OnConnected()
{
	NetworkState::bIsConnected = true;
	instance->GetInitialPlayerData();
}

void SkyUtility::OnDisconnected()
{
	NetworkState::bIsConnected = false;
}

/* LOGIC FUNCTIONS */

bool SkyUtility::IsInGame()
{
	return *g_skyrimVM && *g_thePlayer && !NativeFunctions::IsWindowOpen("Fader Menu") && !NativeFunctions::IsWindowOpen("Loading Menu") && !NativeFunctions::IsWindowOpen("Mist Menu") && !NativeFunctions::IsWindowOpen("Main Menu") && !NativeFunctions::IsWindowOpen("Top Menu");
}

std::mutex pos_mtx, lock_mtx, update_mtx, npc_mtx, npc_update_mtx;
void SkyUtility::Run()
{
	if (!complete)
	{
		if (!completeInitA)
		{
			if (IsInGame())
			{
				SetupCallbacks();
				completeInitA = true;
			}
			else
				return;
		}

		if (!completeInitB)
		{
			if (GameState::mainQuest == nullptr)
			{
				clientKey = (BYTE)strtoul(Utilities::GetIniVar(CONFIG_FILE, "main", "clientKey").c_str(), nullptr, 0);
				GameState::mainQuest = DYNAMIC_CAST(LookupFormByID(0x0003372B), TESForm, TESQuest); //Don't start until we leave the starting area (created our character)
				SetINIBool(GameState::skyrimVMRegistry, 23000, nullptr, &BSFixedString("bUseFaceGenPreprocessedHeads:General"), false);
				SetINIBool(GameState::skyrimVMRegistry, 23000, nullptr, &BSFixedString("bAlwaysActive:General"), true);
			}

			// This is the earliest that we can run the mod without crashing. Just after helgen.
			if (GetCurrentStageID(GameState::skyrimVMRegistry, 23000, GameState::mainQuest) >= 160)
				completeInitB = true;
			else
				return;
		}

		NetworkHandler::APS(nullptr, *g_thePlayer, "SkyBrothers LoadPlayer");
		GameState::plState.displayName = NativeFunctions::GetDisplayName(*g_thePlayer).data;

		string notificationText = "[TamrielOnline] started, press '" + GetKeyName(clientKey) + "' to use";
		Notification(GameState::skyrimVMRegistry, 0, nullptr, &BSFixedString(notificationText.c_str()));

		complete = true;
	}

	if (GameState::IsMenuOpen || GameState::IsLoading || NativeFunctions::IsWindowOpen("Fader Menu") || NativeFunctions::IsWindowOpen("TweenMenu") ||
		NativeFunctions::IsWindowOpen("Console") || NativeFunctions::IsWindowOpen("Console Native UI Menu"))
	{
		Networking::instance->sendKeepAlive();
		Networking::instance->runBasic();
		return;
	}

	Tests::RunTests();

	bool clientKeyPressed = GetKeyPressed(GetStartupKey());

	if (clientKeyPressed)
	{
		if (connectTimer.HasMillisecondsPassed(5000))
		{
			connectTimer.StartTimer();
			Connect();
		}
	}

	Networking::instance->run();

	if (NetworkState::bIsConnected)
	{
		if (NetworkHandler::HasInitialized)
		{
			if (!NetworkHandler::RemoteNpcUpdates.empty())
			{
				std::unique_lock<std::mutex> lock(npc_update_mtx, std::try_to_lock);

				if (lock.owns_lock())
				{
					if (NetworkHandler::clearRemoteNpcs)
					{
						NetworkHandler::RemoteNpcMap.clear();
						NetworkHandler::RemoteNpcUpdates = queue<pair<int, PlayerData>>();
						NetworkHandler::clearRemoteNpcs = false;
					}
					else
					{
						while (!NetworkHandler::RemoteNpcUpdates.empty())
						{
							NetworkHandler::UpdateNpc(NetworkHandler::RemoteNpcUpdates.front().second, NetworkHandler::RemoteNpcUpdates.front().first);
							NetworkHandler::RemoteNpcUpdates.pop();
						}
					}
				}
			}

			//Stagger the player updates from the NPC updates
			if (npcTimer.HasMillisecondsPassed(1000))
			{
				std::unique_lock<std::mutex> lock(npc_mtx, std::try_to_lock);

				if (lock.owns_lock())
				{
					npcTimer.StartTimer();
					NetworkHandler::SendNpcUpdate();
				}
			}

			if (cellTimer.HasMillisecondsPassed(1000))
			{
				std::unique_lock<std::mutex> lock(update_mtx, std::try_to_lock);

				if (lock.owns_lock())
				{
					cellTimer.StartTimer();
					NetworkHandler::UpdateTOD();

					if (NetworkHandler::fullRefresh)
					{
						vector<TESObjectREFR*> tempNpcList;

						//If it's an interior, use the interior search
						if ((*g_thePlayer)->parentCell && IsInterior(GameState::skyrimVMRegistry, 0, (*g_thePlayer)->parentCell))
						{
							vector<TESObjectCELL*> childCells = NetworkHandler::LocChildCellLookupTable[NetworkHandler::PlayerCurrentLocation];
							vector<BGSLocation*> childLocations = NetworkHandler::LocChildLocationLookupTable[NetworkHandler::PlayerCurrentLocation];

							vector<TESObjectCELL*> tempChildCells;
							for (int i = 0; i < childLocations.size(); i++)
							{
								tempChildCells = NetworkHandler::LocChildCellLookupTable[childLocations[i]];

								for (int j = 0; j < tempChildCells.size(); j++)
									childCells.push_back(tempChildCells[j]);
							}

							for (int i = 0; i < childCells.size(); i++)
							{
								tempNpcList = NativeFunctions::GetCellMembers(childCells[i]);

								for (auto& tempNpc : tempNpcList)
									NetworkHandler::AddLocalNpcList(DYNAMIC_CAST(tempNpc, TESObjectREFR, Actor));
							}
						}

						//If not, get exterior references
						else
						{
							tempNpcList = NativeFunctions::GetLocationReferences(NetworkHandler::PlayerCurrentLocation);

							for (auto& tempNpc : tempNpcList)
								NetworkHandler::AddLocalNpcList(DYNAMIC_CAST(tempNpc, TESObjectREFR, Actor));
						}

						NetworkHandler::fullRefresh = false;
					}
					else
					{
						if ((*g_thePlayer)->parentCell)
						{
							Actor* aRef = NULL;
							TESObjectREFR* instanceRef = NULL;
							for (int i = 0; (*g_thePlayer)->parentCell->objectList.GetNthItem(i, instanceRef); i++)
							{
								if (!instanceRef || instanceRef->formID == (*g_thePlayer)->formID)
									continue;

								aRef = DYNAMIC_CAST(instanceRef, TESObjectREFR, Actor);

								if (aRef && aRef->loadedState)
								{
									if (NetworkHandler::IsLocationMaster())
										NetworkHandler::UpdateLocalNpcList(aRef);
									else
										NetworkHandler::PreventingUnauthorizedSpawn(instanceRef);
								}
							}
						}
					}
				}
			}

			if (positionTimer.HasMillisecondsPassed(250))
			{
				std::unique_lock<std::mutex> lock(pos_mtx, std::try_to_lock);

				if (lock.owns_lock())
				{
					positionTimer.StartTimer();
					UpdateCheck();
				}
			}

			if (lockTimer.HasMillisecondsPassed(3000))
			{
				std::unique_lock<std::mutex> lock(lock_mtx, std::try_to_lock);

				if (lock.owns_lock())
				{
					lockTimer.StartTimer();
					NetworkHandler::SendLockMessage();
				}
			}
		}
	}
}

void SkyUtility::GetInitialPlayerData(bool response)
{
	string netData;

	TESNPC* playerBase = DYNAMIC_CAST((*g_thePlayer)->baseForm, TESForm, TESNPC);

	GameState::plState.sex = CALL_MEMBER_FN(playerBase, GetSex)();

	if (GetWorldSpace(GameState::skyrimVMRegistry, 0, *g_thePlayer))
		NetworkHandler::worldid = Utilities::GetFormIDString(GetWorldSpace(GameState::skyrimVMRegistry, 0, *g_thePlayer)->formID).substr(2, 6);

	if (GetCurrentLocation(GameState::skyrimVMRegistry, 0, *g_thePlayer))
	{
		NetworkHandler::locationid = Utilities::GetFormIDString(GetCurrentLocation(GameState::skyrimVMRegistry, 0, *g_thePlayer)->formID).substr(2, 6)
			+ NetworkHandler::worldid;
	}

	if (NetworkHandler::myLoadOrder.empty())
	{
		vector<string> tModMap;
		GameState::loadOrder = "lorder";

		DataHandler* dHandler = DataHandler::GetSingleton();
		for (int i = 0; i < dHandler->modList.loadedModCount; i++)
		{
			tModMap.push_back(md5(dHandler->modList.loadedMods[i]->name));
			GameState::loadOrder += ("," + tModMap.back());
		}

		NetworkHandler::SetMyLoadOrder(tModMap);
	}

	GameState::plState.raceName = NativeFunctions::GetName((*g_thePlayer)->race).data;

	GameState::plState.fRaceId = (*g_thePlayer)->race->formID;
	GameState::plState.weight = playerBase->weight;
	GameState::plState.height = playerBase->height;

	if (GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, false))
		GameState::plState.fRightWeaponId = GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, false)->formID;
	else
	{
		if (GetEquippedSpell(GameState::skyrimVMRegistry, 23000, *g_thePlayer, 1))
			GameState::plState.fRightWeaponId = GetEquippedSpell(GameState::skyrimVMRegistry, 23000, *g_thePlayer, 1)->formID;
	}

	if (GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, true))
		GameState::plState.fLeftWeaponId = GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, true)->formID;
	else
	{
		if (GetEquippedSpell(GameState::skyrimVMRegistry, 23000, *g_thePlayer, 0))
			GameState::plState.fLeftWeaponId = GetEquippedSpell(GameState::skyrimVMRegistry, 23000, *g_thePlayer, 0)->formID;
	}

	ArmorCheck(0x00000800, &GameState::plState.fHairLongId, "hairlong");
	ArmorCheck(0x00000001, &GameState::plState.fHeadArmorId, "head");
	ArmorCheck(0x00000004, &GameState::plState.fBodyArmorId, "body");
	ArmorCheck(0x00000008, &GameState::plState.fHandsArmorId, "hands");
	ArmorCheck(0x00000010, &GameState::plState.fForeArmArmorId, "forearm");
	ArmorCheck(0x00000020, &GameState::plState.fAmuletArmorId, "amulet");
	ArmorCheck(0x00000040, &GameState::plState.fRingArmorId, "ring");
	ArmorCheck(0x00000080, &GameState::plState.fFeetArmorId, "feet");
	ArmorCheck(0x00000100, &GameState::plState.fCalvesArmorId, "calves");
	ArmorCheck(0x00000200, &GameState::plState.fShieldArmorId, "right");
	ArmorCheck(0x00001000, &GameState::plState.fCircletArmorId, "circlet");

	if (NativeFunctions::GetFaceTextureSet(playerBase))
		GameState::plState.fFaceset = NativeFunctions::GetFaceTextureSet(playerBase)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 0))
		GameState::plState.fMouthId = NativeFunctions::GetNthHeadPart(playerBase, 0)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 1))
		GameState::plState.fHeadId = NativeFunctions::GetNthHeadPart(playerBase, 1)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 2))
		GameState::plState.fEyesId = NativeFunctions::GetNthHeadPart(playerBase, 2)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 3))
		GameState::plState.fHairId = NativeFunctions::GetNthHeadPart(playerBase, 3)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 4))
		GameState::plState.fBeardId = NativeFunctions::GetNthHeadPart(playerBase, 4)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 5))
		GameState::plState.fScarId = NativeFunctions::GetNthHeadPart(playerBase, 5)->formID;
	if (NativeFunctions::GetNthHeadPart(playerBase, 6))
		GameState::plState.fBrowId = NativeFunctions::GetNthHeadPart(playerBase, 6)->formID;

	if (playerBase->actorData.voiceType)
		GameState::plState.fVoiceId = playerBase->actorData.voiceType->formID;

	if (playerBase->headData)
		GameState::plState.fHairColor = playerBase->headData->hairColor->formID;

	//We should add the ammoId in here at some point
	NetworkHandler::SendPlayerSpawn(GameState::plState.displayName, GameState::loadOrder, GameState::plState.fRaceId, GameState::plState.raceName, GameState::plState.sex, GameState::plState.weight, GameState::plState.fRightWeaponId, GameState::plState.fLeftWeaponId, GameState::plState.fHeadArmorId,
	                              GameState::plState.fHairTypeId, GameState::plState.fHairLongId, GameState::plState.fBodyArmorId, GameState::plState.fHandsArmorId, GameState::plState.fForeArmArmorId, GameState::plState.fAmuletArmorId, GameState::plState.fRingArmorId, GameState::plState.fFeetArmorId, GameState::plState.fCalvesArmorId, GameState::plState.fShieldArmorId, GameState::plState.fCircletArmorId, GameState::plState.fMouthId,
	                              GameState::plState.fHeadId, GameState::plState.fEyesId, GameState::plState.fHairId, GameState::plState.fBeardId, GameState::plState.fScarId, GameState::plState.fBrowId, GameState::plState.height, GameState::plState.fFaceset, GameState::plState.fHairColor, GameState::plState.fVoiceId, response);

	GameState::plState.av.skOneHanded = NativeFunctions::GetAV(*g_thePlayer, "OneHanded");
	GameState::plState.av.skTwoHanded = NativeFunctions::GetAV(*g_thePlayer, "TwoHanded");
	GameState::plState.av.skMarksman = NativeFunctions::GetAV(*g_thePlayer, "Marksman");
	GameState::plState.av.skBlock = NativeFunctions::GetAV(*g_thePlayer, "Block");
	GameState::plState.av.skSmithing = NativeFunctions::GetAV(*g_thePlayer, "Smithing");
	GameState::plState.av.skHeavyArmor = NativeFunctions::GetAV(*g_thePlayer, "HeavyArmor");
	GameState::plState.av.skLightArmor = NativeFunctions::GetAV(*g_thePlayer, "LightArmor");
	GameState::plState.av.skPickpocket = NativeFunctions::GetAV(*g_thePlayer, "Pickpocket");
	GameState::plState.av.skSneak = NativeFunctions::GetAV(*g_thePlayer, "Sneak");
	GameState::plState.av.skAlchemy = NativeFunctions::GetAV(*g_thePlayer, "Alchemy");
	GameState::plState.av.skSpeechcraft = NativeFunctions::GetAV(*g_thePlayer, "Speechcraft");
	GameState::plState.av.skAlteration = NativeFunctions::GetAV(*g_thePlayer, "Alteration");
	GameState::plState.av.skConjuration = NativeFunctions::GetAV(*g_thePlayer, "Conjuration");
	GameState::plState.av.skDestruction = NativeFunctions::GetAV(*g_thePlayer, "Destruction");
	GameState::plState.av.skIllusion = NativeFunctions::GetAV(*g_thePlayer, "Illusion");
	GameState::plState.av.skRestoration = NativeFunctions::GetAV(*g_thePlayer, "Restoration");
	GameState::plState.av.skEnchanting = NativeFunctions::GetAV(*g_thePlayer, "Enchanting");
	GameState::plState.av.atHealth = NativeFunctions::GetAV(*g_thePlayer, "Health");
	GameState::plState.av.atMagicka = NativeFunctions::GetAV(*g_thePlayer, "Magicka");
	GameState::plState.av.atStamina = NativeFunctions::GetAV(*g_thePlayer, "Stamina");
	GameState::plState.av.mtSpeed = NativeFunctions::GetAV(*g_thePlayer, "SpeedMult");
	GameState::plState.av.stInventoryWeight = NativeFunctions::GetAV(*g_thePlayer, "InventoryWeight");
	GameState::plState.av.stCarryWeight = INT_MAX;//NativeFunctions::GetAV(*g_thePlayer, "CarryWeight");
	GameState::plState.av.stCritChance = NativeFunctions::GetAV(*g_thePlayer, "CritChance") * 100;
	GameState::plState.av.stMeleeDamage = NativeFunctions::GetAV(*g_thePlayer, "MeleeDamage");
	GameState::plState.av.stUnarmedDamage = NativeFunctions::GetAV(*g_thePlayer, "UnarmedDamage");
	GameState::plState.av.stMass = NativeFunctions::GetAV(*g_thePlayer, "Mass");
	GameState::plState.av.ssParalysis = NativeFunctions::GetAV(*g_thePlayer, "Paralysis");
	GameState::plState.av.ssInvisibility = NativeFunctions::GetAV(*g_thePlayer, "Invisibility");
	GameState::plState.av.ssWaterBreathing = NativeFunctions::GetAV(*g_thePlayer, "WaterBreathing");
	GameState::plState.av.ssWaterWalking = NativeFunctions::GetAV(*g_thePlayer, "WaterWalking");
	GameState::plState.av.stBlindness = NativeFunctions::GetAV(*g_thePlayer, "Blindness");
	GameState::plState.av.mtWeaponSpeed = NativeFunctions::GetAV(*g_thePlayer, "WeaponSpeedMult");
	GameState::plState.av.stBowStaggerBonus = NativeFunctions::GetAV(*g_thePlayer, "BowStaggerBonus");
	GameState::plState.av.mtMovementNoise = NativeFunctions::GetAV(*g_thePlayer, "MovementNoiseMult");
	GameState::plState.av.mtLeftWeaponSpeed = NativeFunctions::GetAV(*g_thePlayer, "LeftWeaponSpeedMult");
	GameState::plState.av.stDragonSouls = NativeFunctions::GetAV(*g_thePlayer, "DragonSouls");
	GameState::plState.av.mtAttackDamage = NativeFunctions::GetAV(*g_thePlayer, "AttackDamageMult");
	GameState::plState.av.stReflectDamage = NativeFunctions::GetAV(*g_thePlayer, "ReflectDamage");
	GameState::plState.av.rtDamage = NativeFunctions::GetAV(*g_thePlayer, "DamageResist");
	GameState::plState.av.rtMagic = NativeFunctions::GetAV(*g_thePlayer, "MagicResist");

	netData = "actorValues,";
	netData += to_string(GameState::plState.av.skOneHanded) + "," + to_string(GameState::plState.av.skTwoHanded) + "," + to_string(GameState::plState.av.skMarksman) + ","
		+ to_string(GameState::plState.av.skBlock) + "," + to_string(GameState::plState.av.skSmithing) + "," + to_string(GameState::plState.av.skHeavyArmor) + ","
		+ to_string(GameState::plState.av.skLightArmor) + "," + to_string(GameState::plState.av.skPickpocket) + "," + to_string(GameState::plState.av.skSneak) + ","
		+ to_string(GameState::plState.av.skAlchemy) + "," + to_string(GameState::plState.av.skSpeechcraft) + "," + to_string(GameState::plState.av.skAlteration) + ","
		+ to_string(GameState::plState.av.skConjuration) + "," + to_string(GameState::plState.av.skDestruction) + "," + to_string(GameState::plState.av.skIllusion) + ","
		+ to_string(GameState::plState.av.skRestoration) + "," + to_string(GameState::plState.av.skEnchanting) + "," + to_string(GameState::plState.av.atHealth) + ","
		+ to_string(GameState::plState.av.atMagicka) + "," + to_string(GameState::plState.av.atStamina) + "," + to_string(GameState::plState.av.mtSpeed) + ","
		+ to_string(GameState::plState.av.stInventoryWeight) + "," + to_string(GameState::plState.av.stCarryWeight) + "," + to_string(GameState::plState.av.stCritChance) + ","
		+ to_string(GameState::plState.av.stMeleeDamage) + "," + to_string(GameState::plState.av.stUnarmedDamage) + "," + to_string(GameState::plState.av.stMass) + ","
		+ to_string(GameState::plState.av.ssParalysis) + "," + to_string(GameState::plState.av.ssInvisibility) + "," + to_string(GameState::plState.av.ssWaterBreathing) + ","
		+ to_string(GameState::plState.av.ssWaterWalking) + "," + to_string(GameState::plState.av.stBlindness) + "," + to_string(GameState::plState.av.mtWeaponSpeed) + ","
		+ to_string(GameState::plState.av.stBowStaggerBonus) + "," + to_string(GameState::plState.av.mtMovementNoise) + "," + to_string(GameState::plState.av.mtLeftWeaponSpeed)
		+ "," + to_string(GameState::plState.av.stDragonSouls) + "," + to_string(GameState::plState.av.mtAttackDamage) + "," + to_string(GameState::plState.av.stReflectDamage)
		+ "," + to_string(GameState::plState.av.rtDamage) + "," + to_string(GameState::plState.av.rtMagic);
	NetworkHandler::SendData((char*)netData.c_str());
	NetworkHandler::HasInitialized = true;
}

void SkyUtility::RefreshLocation()
{
	NetworkHandler::PlayerCurrentLocation = GetCurrentLocation(GameState::skyrimVMRegistry, 0, *g_thePlayer);
	TESWorldSpace* plRefreshWorldspace = GetWorldSpace(GameState::skyrimVMRegistry, 0, *g_thePlayer);

	if (NetworkHandler::PlayerCurrentLocation)
		tempLoc = Utilities::GetFormIDString(NetworkHandler::PlayerCurrentLocation->formID).substr(2, 6);

	if (plRefreshWorldspace)
		NetworkHandler::worldid = Utilities::GetFormIDString(plRefreshWorldspace->formID).substr(2, 6);

	if (tempLoc != "" && tempLoc + NetworkHandler::worldid != NetworkHandler::locationid)
	{
		NetworkHandler::locationid = tempLoc + NetworkHandler::worldid;

		if (NetworkState::bIsConnected)
			NetworkHandler::locationEntryTime = NetworkHandler::lastTimeReference;

		//Re-enable any npc's that may have gotten disabled in the previous area, by the networking.
		NetworkHandler::RestoreLocalNpcs();

		//We're moving to a new location, clear out our current npc list
		NetworkHandler::LocalNpcMap.clear();
		NetworkHandler::clearRemoteNpcs = true;
		NetworkHandler::fullRefresh = true;
	}
}

bool SkyUtility::ArmorCheck(UInt32 armorFormId, UInt32* idRef, const char* tName = "")
{
	TESForm* wornItem = NativeFunctions::GetWornForm(*g_thePlayer, armorFormId);

	if (wornItem)
	{
		long tArmor = wornItem->formID;
		if (tArmor != *idRef)
		{
			*idRef = wornItem->formID;

			if (NetworkHandler::HasInitialized)
			{
				string netData;
				netData = "equip,";
				netData += to_string(*idRef) + "," + (strcmp(tName, "") == 0 ? to_string(*idRef) : tName);
				NetworkHandler::SendData((char*)netData.c_str());
			}

			return true;
		}
	}

	else
	{
		if (0 != *idRef)
		{
			*idRef = 0;

			if (NetworkHandler::HasInitialized)
			{
				string netData;
				netData = "equip,";
				netData += to_string(*idRef) + "," + (strcmp(tName, "") == 0 ? to_string(*idRef) : tName);
				NetworkHandler::SendData((char*)netData.c_str());
			}
			return true;
		}
	}

	return false;
}

bool SkyUtility::ArmorCheck(UInt32 armorFormId, UInt32* idRef, bool isEquipping, UInt32 itemId, const char* tName = "")
{
	TESForm* wornItem = NativeFunctions::GetWornForm(*g_thePlayer, armorFormId);

	if (wornItem)
	{
		if (isEquipping)
		{
			if (wornItem->formID != *idRef)
			{
				*idRef = wornItem->formID;

				if (NetworkHandler::HasInitialized)
				{
					string netData;
					netData = "equip,";
					netData += to_string(*idRef) + "," + (strcmp(tName, "") == 0 ? to_string(*idRef) : tName);
					NetworkHandler::SendData((char*)netData.c_str());
				}

				return true;
			}
		}

		else
		{
			if (wornItem->formID == *idRef && itemId == *idRef)
			{
				*idRef = 0;

				if (NetworkHandler::HasInitialized)
				{
					string netData;
					netData = "equip,";
					netData += to_string(*idRef) + "," + (strcmp(tName, "") == 0 ? to_string(*idRef) : tName);
					NetworkHandler::SendData((char*)netData.c_str());
				}
				return true;
			}
		}
	}

	return false;
}

void SkyUtility::CheckEquipEvent(TESEquipEvent e)
{
	if (e.actor->formID == (*g_thePlayer)->formID)
	{
		string netData = "";
		TESForm* tEquipment = LookupFormByID(e.equippedFormID);

		//SPELLS
		if (tEquipment->GetFormType() == kFormType_Spell)
		{
			if (e.isEquipping)
			{
				SpellItem* rightSpell = GetEquippedSpell(GameState::skyrimVMRegistry, 0, *g_thePlayer, 1);
				SpellItem* leftSpell = GetEquippedSpell(GameState::skyrimVMRegistry, 0, *g_thePlayer, 0);
				UInt32 tSpell = e.equippedFormID;

				if (rightSpell)
				{
					if (tSpell != GameState::plState.fRightSpellName)
					{
						GameState::plState.fRightSpellName = tSpell;
						netData = "equipspell,";
						netData += to_string(GameState::plState.fRightSpellName) + ",right";
						NetworkHandler::SendData((char*)netData.c_str());
						return;
					}
				}

				if (leftSpell)
				{
					if (tSpell != GameState::plState.fLeftSpellName)
					{
						GameState::plState.fLeftSpellName = tSpell;
						netData = "equipspell,";
						netData += to_string(GameState::plState.fLeftSpellName) + ",left";
						NetworkHandler::SendData((char*)netData.c_str());
					}
				}
			}

			else
			{
				TESForm* removingSpellForm = LookupFormByID(e.equippedFormID);
				SpellItem* removingSpell = DYNAMIC_CAST(removingSpellForm, TESForm, SpellItem);

				TESForm* leftObject = (*g_thePlayer)->GetEquippedObject(true);
				TESForm* rightObject = (*g_thePlayer)->GetEquippedObject(false);

				if (!leftObject)
				{
					GameState::plState.fLeftSpellName = -1;
					netData = "removeSpellLeft";
					NetworkHandler::SendData((char*)netData.c_str());
				}

				if (!rightObject)
				{
					GameState::plState.fRightSpellName = -1;
					netData = "removeSpellRight";
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}
		}

		//WEAPONS
		else if (tEquipment->GetFormType() == kFormType_Weapon)
		{
			if (GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, false))
			{
				UInt32 tWeaponId = GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, false)->formID;

				if (GameState::plState.fRightWeaponId != tWeaponId)
				{
					GameState::plState.fRightWeaponId = tWeaponId;

					netData.clear();
					netData = "equip,";
					netData += to_string(GameState::plState.fRightWeaponId) + ",right";
					NetworkHandler::SendData((char*)netData.c_str());
					return;
				}
			}

			else
			{
				if (GameState::plState.fRightWeaponId != 0)
				{
					GameState::plState.fRightWeaponId = 0;
					netData.clear();
					netData = "equip,";
					netData += to_string(GameState::plState.fRightWeaponId) + ",right";
					NetworkHandler::SendData((char*)netData.c_str());
					return;
				}
			}

			if (GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, true))
			{
				UInt32 tWeaponId = GetEquippedWeapon(GameState::skyrimVMRegistry, 0, *g_thePlayer, true)->formID;

				if (GameState::plState.fLeftWeaponId != tWeaponId)
				{
					GameState::plState.fLeftWeaponId = tWeaponId;

					netData.clear();
					netData = "equip,";
					netData += to_string(GameState::plState.fLeftWeaponId) + ",left";
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}

			else
			{
				if (GameState::plState.fLeftWeaponId != 0)
				{
					GameState::plState.fLeftWeaponId = 0;
					netData.clear();
					netData = "equip,";
					netData += to_string(GameState::plState.fLeftWeaponId) + ",left";
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}
		}

		//ARMOR
		else if (tEquipment->GetFormType() == kFormType_Armor)
		{
			if (ArmorCheck(0x00000001, &GameState::plState.fHeadArmorId, e.isEquipping, e.equippedFormID, "head") || ArmorCheck(0x00000004, &GameState::plState.fBodyArmorId, e.isEquipping, e.equippedFormID, "body")
				|| ArmorCheck(0x00000008, &GameState::plState.fHandsArmorId, e.isEquipping, e.equippedFormID, "hands") || ArmorCheck(0x00000010, &GameState::plState.fForeArmArmorId, e.isEquipping, e.equippedFormID, "forearm")
				|| ArmorCheck(0x00000020, &GameState::plState.fAmuletArmorId, e.isEquipping, e.equippedFormID, "amulet") || ArmorCheck(0x00000040, &GameState::plState.fRingArmorId, e.isEquipping, e.equippedFormID, "ring")
				|| ArmorCheck(0x00000080, &GameState::plState.fFeetArmorId, e.isEquipping, e.equippedFormID, "feet") || ArmorCheck(0x00000100, &GameState::plState.fCalvesArmorId, e.isEquipping, e.equippedFormID, "calves")
				|| ArmorCheck(0x00000200, &GameState::plState.fShieldArmorId, e.isEquipping, e.equippedFormID, "right") //Shield will attach the keyword "right" as if it was a weapon, so that it is equipped like a weapon when the message is received over the network.load().
				|| ArmorCheck(0x00001000, &GameState::plState.fCircletArmorId, e.isEquipping, e.equippedFormID, "circlet"))
			{
			}
		}

		//SHOUTS
		else if (tEquipment->GetFormType() == kFormType_Shout)
		{
			if (GetEquippedShout(GameState::skyrimVMRegistry, 0, *g_thePlayer))
			{
				UInt32 tShout = GetEquippedShout(GameState::skyrimVMRegistry, 0, *g_thePlayer)->formID;

				if (tShout != GameState::plState.fShoutId)
				{
					GameState::plState.fShoutId = tShout;
					netData.clear();
					netData = "equipShout,";
					netData += to_string(GameState::plState.fShoutId);
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}

			else
			{
				if (0 != GameState::plState.fShoutId)
				{
					GameState::plState.fShoutId = 0;
					netData.clear();
					netData = "equipShout,";
					netData += to_string(GameState::plState.fShoutId);
					NetworkHandler::SendData((char*)netData.c_str());
				}
			}
		}

		//AMMO
		else if (tEquipment->GetFormType() == kFormType_Ammo)
		{
			if (GameState::plState.fAmmoId != tEquipment->formID)
			{
				GameState::plState.fAmmoId = tEquipment->formID;

				netData.clear();
				netData = "equip,";
				netData += to_string(GameState::plState.fAmmoId) + "," + to_string(GameState::plState.fAmmoId);
				NetworkHandler::SendData((char*)netData.c_str());
			}
		}
	}
}

void SkyUtility::CheckCombatAction(SKSEActionEvent e)
{
	if (e.actor->formID == (*g_thePlayer)->formID)
	{
		if (e.type == e.kType_BeginDraw || e.type == e.kType_EndDraw)
			NetworkHandler::SendData("StartDraw");
		else if (e.type == e.kType_BeginSheathe || e.type == e.kType_EndSheathe)
			NetworkHandler::SendData("StartSheath");
		else if (e.type == e.kType_WeaponSwing)
		{
			//If we have both keys pressed, ignore the individual attacks
			if (IsKeyPressed("Right Attack/Block") && IsKeyPressed("Left Attack/Block"))
				NetworkHandler::SendData("AttackDual");
			else if (e.slot == e.kSlot_Left)
				NetworkHandler::SendData("AttackLeft");
			else if (e.slot == e.kSlot_Right)
				NetworkHandler::SendData("AttackRight");
		}
		else if (e.type == e.kType_SpellCast)
		{
			if (e.slot == e.kSlot_Left)
			{
				if (selfCasted)
				{
					NetworkHandler::SendData("CastLeft,0");
					selfCasted = false;
				}
				else
					NetworkHandler::SendData("CastLeft,");

				GameState::plState.bLeftCasting = true;
			}
			else if (e.slot == e.kSlot_Right)
			{
				if (selfCasted)
				{
					NetworkHandler::SendData("CastRight,0");
					selfCasted = false;
				}
				else
					NetworkHandler::SendData("CastRight,");

				GameState::plState.bRightCasting = true;
			}
		}
		else if (e.type == e.kType_BowRelease)
			NetworkHandler::SendData("FireBow");
	}

	else { } //We can possibly use this for npcs

	if (GameState::plState.bRightCasting)
	{
		if (!NativeFunctions::GetAnimationVariableBool(*g_thePlayer, "IsCastingRight") && !NativeFunctions::GetAnimationVariableBool(*g_thePlayer, "bWantCastRight"))
		{
			NetworkHandler::SendData("InterruptCastRight");
			GameState::plState.bRightCasting = false;
		}
	}

	if (GameState::plState.bLeftCasting)
	{
		if (!NativeFunctions::GetAnimationVariableBool(*g_thePlayer, "IsCastingLeft") && !NativeFunctions::GetAnimationVariableBool(*g_thePlayer, "bWantCastLeft"))
		{
			NetworkHandler::SendData("InterruptCastLeft");
			GameState::plState.bLeftCasting = false;
		}
	}
}

void SkyUtility::UpdateCheck()
{
	newX = (*g_thePlayer)->pos.x;
	newY = (*g_thePlayer)->pos.y;
	newZ = (*g_thePlayer)->pos.z;
	rNewX = GetAngleX(GameState::skyrimVMRegistry, 23000, *g_thePlayer);
	rNewY = GetAngleY(GameState::skyrimVMRegistry, 23000, *g_thePlayer);
	rNewZ = GetAngleZ(GameState::skyrimVMRegistry, 23000, *g_thePlayer);

	if (GameState::plState.transform.pos.x != newX || GameState::plState.transform.pos.y != newY || GameState::plState.transform.pos.z != newZ ||
		GameState::plState.transform.rot.x != rNewX || GameState::plState.transform.rot.y != rNewY || GameState::plState.transform.rot.z != rNewZ || inactivityTimer.HasMillisecondsPassed(5000))
	{
		GameState::plState.transform.pos.x = newX;
		GameState::plState.transform.pos.y = newY;
		GameState::plState.transform.pos.z = newZ;

		GameState::plState.transform.rot.x = rNewX;
		GameState::plState.transform.rot.y = rNewY;
		GameState::plState.transform.rot.z = rNewZ;

		if (!GameState::IsLoading)
		{
			NetworkHandler::SendModifiedPosition(GameState::plState.transform.pos.x, GameState::plState.transform.pos.y, GameState::plState.transform.pos.z,
				                                GameState::plState.transform.rot.x, GameState::plState.transform.rot.y, GameState::plState.transform.rot.z);
		}

		InitialPosition = false;
		inactivityTimer.StartTimer();
	}

	string netData;

	//Are we looking at a horse.
	if (!GameState::plState.bIsRidingHorse && IsOnMount(GameState::skyrimVMRegistry, 0, *g_thePlayer))
	{
		if ((*g_thePlayer)->lastRiddenHorseHandle != NULL)
		{
			refHolder = nullptr;
			GameState::plState.fHorseHandleId = (*g_thePlayer)->lastRiddenHorseHandle;
			LookupREFRByHandle(&GameState::plState.fHorseHandleId, &refHolder);

			if (refHolder)
			{
				//Is the horse being ridden?
				if (IsBeingRidden(GameState::skyrimVMRegistry, 0, DYNAMIC_CAST(refHolder, TESObjectREFR, Actor)))
				{
					NetworkHandler::SendData((char*)("ridehorse," + to_string(refHolder->formID)).c_str());
					GameState::plState.bIsRidingHorse = true;
				}
			}
		}
	}

	//Are we riding a horse, if so, dismount the horse.
	else if (GameState::plState.bIsRidingHorse && !IsOnMount(GameState::skyrimVMRegistry, 0, *g_thePlayer))
	{
		NetworkHandler::SendData("dismounthorse");
		GameState::plState.bIsRidingHorse = false;
	}

	//Send anim event whenever we start a new idle animation, or it's been 3 seconds since we updated the other players with our current animation.
	//We resend the animation every 3 seconds for people who have move near us, everyone else will ignore the update.
	if (!GameState::plState.bIsAnimationPlaying && (*g_thePlayer)->processManager->middleProcess->currentIdle)
	{
		GameState::plState.bIsAnimationPlaying = true;
		NetworkHandler::SendAnimation(NativeFunctions::GetSitAnimation(*g_thePlayer));
	}

	else if (GameState::plState.bIsAnimationPlaying && !(*g_thePlayer)->processManager->middleProcess->currentIdle)
	{
		GameState::plState.bIsAnimationPlaying = false;
		NetworkHandler::SendAnimation("");
	}

	if (periodicTimeCheckUtilities.HasMillisecondsPassed(2500))
	{
		if (GameState::plState.av.stMeleeDamage != NativeFunctions::GetAV(*g_thePlayer, "MeleeDamage"))
		{
			netData = "meleeDamage,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "MeleeDamage"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.stMeleeDamage = NativeFunctions::GetAV(*g_thePlayer, "MeleeDamage");
		}

		if (GameState::plState.av.stUnarmedDamage != NativeFunctions::GetAV(*g_thePlayer, "UnarmedDamage"))
		{
			netData = "unarmedDamage,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "UnarmedDamage"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.stUnarmedDamage = NativeFunctions::GetAV(*g_thePlayer, "UnarmedDamage");
		}

		if (GameState::plState.av.ssParalysis != NativeFunctions::GetAV(*g_thePlayer, "Paralysis"))
		{
			netData = "paralysis,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "Paralysis"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.ssParalysis = NativeFunctions::GetAV(*g_thePlayer, "Paralysis");
		}

		if (GameState::plState.av.ssInvisibility != NativeFunctions::GetAV(*g_thePlayer, "Invisibility"))
		{
			netData = "invisibility,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "Invisibility"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.ssInvisibility = NativeFunctions::GetAV(*g_thePlayer, "Invisibility");
		}

		if (GameState::plState.av.ssWaterWalking != NativeFunctions::GetAV(*g_thePlayer, "WaterWalking"))
		{
			netData = "waterWalking,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "WaterWalking"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.ssWaterWalking = NativeFunctions::GetAV(*g_thePlayer, "WaterWalking");
		}

		if (GameState::plState.av.mtAttackDamage != NativeFunctions::GetAV(*g_thePlayer, "AttackDamageMult"))
		{
			netData = "attackDamageMult,";
			netData += to_string(NativeFunctions::GetAV(*g_thePlayer, "AttackDamageMult"));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.mtAttackDamage = NativeFunctions::GetAV(*g_thePlayer, "AttackDamageMult");
		}

		if (GameState::plState.av.mtSpeed != (GetActorValuePercentage(GameState::skyrimVMRegistry, 0, *g_thePlayer, &BSFixedString("SpeedMult")) * 100))
		{
			netData = "speedMult,";
			netData += to_string(GetActorValuePercentage(GameState::skyrimVMRegistry, 0, *g_thePlayer, &BSFixedString("SpeedMult")));
			NetworkHandler::SendData((char*)netData.c_str());
			GameState::plState.av.mtSpeed = NativeFunctions::GetAV(*g_thePlayer, "SpeedMult");
		}

		ArmorCheck(0x00000002, &GameState::plState.fHairTypeId);
		ArmorCheck(0x00000800, &GameState::plState.fHairLongId);

		periodicTimeCheckUtilities.StartTimer();
	}
}

void SkyUtility::Connect()
{
	if (!NetworkState::bIsConnected)
	{
		NetworkHandler::PrintNote("Starting Connection...");
		Networking::instance->connect();
	}

	else
	{
		NetworkHandler::PrintNote("Disconnecting...");
		NetworkHandler::Disconnect();
		Networking::instance->disconnect();
	}
}

void SkyUtility::PlayerKOFA(StaticFunctionTag* base, Actor* target, UInt32 positionSelection)
{
	((ActorEx*)target)->KeepOffsetFromActor(NetworkHandler::RemotePlayerMap[positionSelection].positionControllerActor, 0, 0, 0, 0, 0, 0, 128, 20);
}

/* SKSE FUNCTIONS */
void SkyUtility::SetINIFloatEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection, BSFixedString iniVariable, float value)
{
	_MESSAGE(iniName.data);
	string strNew = iniVariable.data + (string)"=" + to_string(value);
	string sIniSection = iniSection.data;
	string sIniVariable = iniVariable.data + (string)"=";

	ifstream t(iniName.data);
	string str;

	t.seekg(0, ios::end);
	str.reserve(t.tellg());
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
	           istreambuf_iterator<char>());

	size_t foundSection = str.find(sIniSection);

	if (foundSection != string::npos)
	{
		size_t foundVariable = str.find(sIniVariable);

		if (foundVariable != string::npos && foundVariable > foundSection)
		{
			size_t foundNewline = str.find("\n", foundVariable + 1);

			if (foundNewline != string::npos)
				str.replace(foundVariable, foundNewline - foundVariable, strNew);
		}
	}

	t.close();

	ofstream fileout(iniName.data);

	if (!fileout.is_open())
		return;

	fileout << str;

	fileout.close();
}

void SkyUtility::SetINIBoolEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection, BSFixedString iniVariable, bool value)
{
	_MESSAGE(iniName.data);
	string strNew = iniVariable.data + (string)"=" + to_string(value);
	string sIniSection = iniSection.data;
	string sIniVariable = iniVariable.data + (string)"=";

	ifstream t(iniName.data);
	string str;

	t.seekg(0, ios::end);
	str.reserve(t.tellg());
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
	           istreambuf_iterator<char>());

	size_t foundSection = str.find(sIniSection);

	if (foundSection != string::npos)
	{
		size_t foundVariable = str.find(sIniVariable);

		if (foundVariable != string::npos && foundVariable > foundSection)
		{
			size_t foundNewline = str.find("\n", foundVariable + 1);

			if (foundNewline != string::npos)
				str.replace(foundVariable, foundNewline - foundVariable, strNew);
		}
	}

	t.close();

	ofstream fileout(iniName.data);

	if (!fileout.is_open())
		return;

	fileout << str;

	fileout.close();
}

void SkyUtility::SetINIIntEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection, BSFixedString iniVariable, UInt32 value)
{
	_MESSAGE(iniName.data);
	string strNew = iniVariable.data + (string)"=" + to_string(value);
	string sIniSection = iniSection.data;
	string sIniVariable = iniVariable.data + (string)"=";

	ifstream t(iniName.data);
	string str;

	t.seekg(0, ios::end);
	str.reserve(t.tellg());
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
	           istreambuf_iterator<char>());

	size_t foundSection = str.find(sIniSection);

	if (foundSection != string::npos)
	{
		size_t foundVariable = str.find(sIniVariable);

		if (foundVariable != string::npos && foundVariable > foundSection)
		{
			size_t foundNewline = str.find("\n", foundVariable + 1);

			if (foundNewline != string::npos)
				str.replace(foundVariable, foundNewline - foundVariable, strNew);
		}
	}

	t.close();

	ofstream fileout(iniName.data);

	if (!fileout.is_open())
		return;

	fileout << str;

	fileout.close();
}

void SkyUtility::SetINIStringEx(StaticFunctionTag* base, BSFixedString iniName, BSFixedString iniSection, BSFixedString iniVariable, BSFixedString value)
{
	_MESSAGE(iniName.data);
	string strNew = iniVariable.data + (string)"=" + value.data;
	string sIniSection = iniSection.data;
	string sIniVariable = iniVariable.data + (string)"=";

	ifstream t(iniName.data);
	string str;

	t.seekg(0, ios::end);
	str.reserve(t.tellg());
	t.seekg(0, ios::beg);

	str.assign((istreambuf_iterator<char>(t)),
	           istreambuf_iterator<char>());

	size_t foundSection = str.find(sIniSection);

	if (foundSection != string::npos)
	{
		size_t foundVariable = str.find(sIniVariable);

		if (foundVariable != string::npos && foundVariable > foundSection)
		{
			size_t foundNewline = str.find("\n", foundVariable + 1);

			if (foundNewline != string::npos)
				str.replace(foundVariable, foundNewline - foundVariable, strNew);
		}
	}

	t.close();

	ofstream fileout(iniName.data);

	if (!fileout.is_open())
		return;

	fileout << str;

	fileout.close();
}

void SkyUtility::EquipItem(StaticFunctionTag* base, Actor* ref, TESForm* item, UInt32 slot)
{
	NativeFunctions::ExecuteCommand(((string)"equipitem " + Utilities::GetFormIDString(item->formID) + " 1 " + (slot == 0 ? "left" : "right")).c_str(), ref);
}

void SkyUtility::SetTransform(StaticFunctionTag* base, BSFixedString ref, UInt32 x, UInt32 y, UInt32 z, UInt32 xRot, UInt32 yRot, UInt32 zRot)
{
	TESObjectREFR* target = (TESObjectREFR*)LookupFormByID(strtoul(ref.data, nullptr, 0));

	NativeFunctions::ExecuteCommand(((string)"SetPos X " + to_string(x)).c_str(), target);
	NativeFunctions::ExecuteCommand(((string)"SetPos Y " + to_string(y)).c_str(), target);
	NativeFunctions::ExecuteCommand(((string)"SetPos Z " + to_string(z)).c_str(), target);

	NativeFunctions::ExecuteCommand(((string)"SetAngle X " + to_string(xRot)).c_str(), target);
	NativeFunctions::ExecuteCommand(((string)"SetAngle Y " + to_string(yRot)).c_str(), target);
	NativeFunctions::ExecuteCommand(((string)"SetAngle Z " + to_string(zRot)).c_str(), target);
}

//Calls enable net after ensuring that the actor cannot be disabled by npc checks.
void SkyUtility::EnableNetPlayer(StaticFunctionTag* base, Actor* target)
{
	if (std::find(NetworkHandler::playerLookup.begin(), NetworkHandler::playerLookup.end(), target->formID) == NetworkHandler::playerLookup.end())
		NetworkHandler::playerLookup.push_back(target->formID);
	NetworkHandler::EnableNet(target);
}

void SkyUtility::DisableNet(StaticFunctionTag* base, Actor* target)
{
	NetworkHandler::DisableNet(target);
}

bool SkyUtility::RegisterFuncs(VMClassRegistry* registry)
{
	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, BSFixedString>("ReadMessageName", "SkyUtilitiesScript", NetworkHandler::ReadMessageName, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, TESForm*>("ReadMessageRef", "SkyUtilitiesScript", NetworkHandler::ReadMessageRef, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, VMResultArray<BSFixedString>>("ReadMessageValues", "SkyUtilitiesScript", NetworkHandler::ReadMessageValues, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, BSFixedString,
		                    Actor*>("GetSitAnimation", "SkyUtilitiesScript", NetworkHandler::GetSitAnimation, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void,
		                    Actor*, TESObjectREFR*>("MountActor", "SkyUtilitiesScript", NetworkHandler::MountActor, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void,
		                    Actor*>("ResurrectExtended", "SkyUtilitiesScript", NetworkHandler::ResurrectExtended, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void,
		                    Actor*, BSFixedString>("SetRace", "SkyUtilitiesScript", NetworkHandler::SetRace, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void,
		                    Actor*, BSFixedString>("APS", "SkyUtilitiesScript", NetworkHandler::APS, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void,
		                    Actor*, BSFixedString>("SetDisplayName", "SkyUtilitiesScript", NetworkHandler::SetDisplayName, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void,
		                    BSFixedString>("StartTimer", "SkyUtilitiesScript", NetworkHandler::StartTimer, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, bool,
		                    UInt32, BSFixedString>("GetRemotePlayerDataBool", "SkyUtilitiesScript", NetworkHandler::GetRemotePlayerDataBool, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, float,
		                    UInt32, BSFixedString>("GetRemotePlayerDataFloat", "SkyUtilitiesScript", NetworkHandler::GetRemotePlayerDataFloat, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, UInt32,
		                    UInt32, BSFixedString>("GetRemotePlayerDataInt", "SkyUtilitiesScript", NetworkHandler::GetRemotePlayerDataInt, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, BSFixedString,
		                    UInt32, BSFixedString>("GetRemotePlayerDataString", "SkyUtilitiesScript", NetworkHandler::GetRemotePlayerDataString, registry));

	registry->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void,
		                    UInt32, BSFixedString, bool>("SetRemotePlayerDataBool", "SkyUtilitiesScript", NetworkHandler::SetRemotePlayerDataBool, registry));

	registry->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void,
		                    UInt32, BSFixedString, float>("SetRemotePlayerDataFloat", "SkyUtilitiesScript", NetworkHandler::SetRemotePlayerDataFloat, registry));

	registry->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void,
		                    UInt32, BSFixedString, UInt32>("SetRemotePlayerDataInt", "SkyUtilitiesScript", NetworkHandler::SetRemotePlayerDataInt, registry));

	registry->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void,
		                    UInt32, BSFixedString, BSFixedString>("SetRemotePlayerDataString", "SkyUtilitiesScript", NetworkHandler::SetRemotePlayerDataString, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, bool, 
							BSFixedString, float>("HasSecondsPassed", "SkyUtilitiesScript", NetworkHandler::HasSecondsPassed, registry));

	registry->RegisterFunction(new NativeFunction2<StaticFunctionTag, bool, BSFixedString, float>("HasMillisecondsPassed", "SkyUtilitiesScript", 
		NetworkHandler::HasMillisecondsPassed, registry));

	registry->RegisterFunction(
		new NativeFunction4<StaticFunctionTag, void, 
							UInt32, UInt32, UInt32, UInt32>("InitializeNewPlayer", "SkyUtilitiesScript", NetworkHandler::InitializeNewPlayer, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, BSFixedString, 
							Actor*>("GetSitAnimation", "SkyUtilitiesScript", NetworkHandler::GetSitAnimation, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, UInt32>("GetMessageCount", "SkyUtilitiesScript", NetworkHandler::GetMessageCount, registry));

	registry->RegisterFunction(
		new NativeFunction0<StaticFunctionTag, UInt32>("ReadMessageGuid", "SkyUtilitiesScript", NetworkHandler::ReadMessageGuid, registry));

	registry->RegisterFunction(
		new NativeFunction3<StaticFunctionTag, void, 
							Actor*, TESForm*, UInt32>("EquipItem", "SkyUtilitiesScript", EquipItem, registry));

	registry->RegisterFunction(
		new NativeFunction7<StaticFunctionTag, void, 
							BSFixedString, UInt32, UInt32, UInt32, UInt32, UInt32, UInt32>("SetTransform", "SkyUtilitiesScript", SetTransform, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void, 
							Actor*>("EnableNet", "SkyUtilitiesScript", EnableNetPlayer, registry));

	registry->RegisterFunction(
		new NativeFunction1<StaticFunctionTag, void, 
							Actor*>("DisableNet", "SkyUtilitiesScript", DisableNet, registry));

	registry->RegisterFunction(
		new NativeFunction2<StaticFunctionTag, void, 
							Actor*, UInt32>("PlayerKOFA", "SkyUtilitiesScript", PlayerKOFA, registry));

	registry->SetFunctionFlags("SkyUtilitiesScript", "GetMessageCount", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "ReadMessageGuid", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "ReadMessageName", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "ReadMessageRef", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "ReadMessageValues", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "MountActor", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "ResurrectExtended", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetRace", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "APS", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetDisplayName", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "StartTimer", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "GetRemotePlayerDataBool", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "GetRemotePlayerDataFloat", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "GetRemotePlayerDataInt", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "GetRemotePlayerDataString", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetRemotePlayerDataBool", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetRemotePlayerDataFloat", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetRemotePlayerDataInt", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetRemotePlayerDataString", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "HasSecondsPassed", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "HasMillisecondsPassed", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "InitializeNewPlayer", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "GetSitAnimation", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "EquipItem", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "SetTransform", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "EnableNet", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "DisableNet", VMClassRegistry::kFunctionFlag_NoWait);
	registry->SetFunctionFlags("SkyUtilitiesScript", "PlayerKOFA", VMClassRegistry::kFunctionFlag_NoWait);

	return true;
}
