#include "GameEvents.h"

int GameEvents::lastNpcListSize;
bool GameEvents::hasLocationsLoaded, GameEvents::loadingMenuOpen, GameEvents::mistMenuOpen, GameEvents::firstNpcRequest = true;
BGSLocation* GameEvents::npcLocation;
Actor* GameEvents::loadedActor;
Timer GameEvents::npcListTimer, GameEvents::regularTimer, GameEvents::animationTimer; //npcListTimer - Determines how long we'll initially be grabbing npcs.