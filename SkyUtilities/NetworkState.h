#pragma once
#include <vector>
#include <string>

using namespace std;

//Holds all of our generic network data.
class NetworkState
{
public:
	NetworkState() = default;

	enum EV
	{
		ID_MESSAGE,
		ID_SPAWN_PLAYER,
		ID_POSITION_UPDATE,
		ID_SPAWN_PLAYER_RESPONSE, //Used to prevent an endless loop of player spawnings
		ID_SET_TIME,
		ID_SET_WEATHER,
		ID_SET_ANIMATION,
		ID_NPC_UPDATE,
		ID_LOCK_UPDATE
	};

	enum CHANNEL
	{
		EVENT = 0,
		PLAYER_UPDATE = 1,
		NPC_UPDATE = 2
	};

	static bool bIsConnected;
	static vector<int> vConnectionList;

	static void(*OnConnectedToServer)();
	static void(*OnDisconnectedFromServer)();
	static void(*OnConnectionFailed)();
	static void(*OnReceivedMessage)(void* message);
};