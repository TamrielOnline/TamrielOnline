#include "NetworkHandler.h"

queue<PapyrusData> NetworkHandler::papyrusQueue;
TESObjectREFR* NetworkHandler::PlayerRef;
map<BGSLocation*, vector<TESObjectCELL*>> NetworkHandler::LocChildCellLookupTable;
map<BGSLocation*, vector<BGSLocation*>> NetworkHandler::LocChildLocationLookupTable;
map<UInt32, PlayerData> NetworkHandler::RemotePlayerMap;
map<UInt32, PlayerData> NetworkHandler::LocalNpcMap;
map<UInt32, PlayerData> NetworkHandler::RemoteNpcMap;
queue<pair<int, PlayerData>> NetworkHandler::RemoteNpcUpdates;
set<UInt32> NetworkHandler::disabledNpcs;
bool NetworkHandler::fullRefresh;
map<UInt32, bool> NetworkHandler::ForceSpawn;
vector<UInt32> NetworkHandler::playerLookup;
map<UInt32, Timer> NetworkHandler::KOFATimers;
bool NetworkHandler::ignoreSpawns;
bool NetworkHandler::locationMaster = false;
Timer NetworkHandler::dropTimer;
bool NetworkHandler::HasInitialized;
vector<LockData> NetworkHandler::lockList;
map<UInt32, int> NetworkHandler::ignoreLock;
vector<string> NetworkHandler::myLoadOrder = vector<string>();
Timer NetworkHandler::receivedTime, NetworkHandler::dayTimer;
mutex NetworkHandler::papyrus_mtx;