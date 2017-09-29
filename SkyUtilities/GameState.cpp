#include "GameState.h"

GameState::ActorState GameState::plState = GameState::ActorState();
string GameState::loadOrder = "";
TESFaction* GameState::npcFaction;
VMClassRegistry* GameState::skyrimVMRegistry;
bool GameState::IsLoading, GameState::IsMenuOpen, GameState::IsRefreshing;
TESQuest* GameState::mainQuest;