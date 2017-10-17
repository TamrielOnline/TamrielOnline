#pragma once

#include "LoadBalancing-cpp/inc/Client.h"
#include <string>

enum State
{
	STATE_INITIALIZED = 0,
	STATE_CONNECTING,
	STATE_CONNECTED,
	STATE_JOINING,
	STATE_JOINED,
	STATE_LEAVING,
	STATE_LEFT,
	STATE_DISCONNECTING,
	STATE_DISCONNECTED
};

enum Input
{
	INPUT_NON = 0,
	INPUT_1,
	INPUT_2,
	INPUT_EXIT
};

class NetworkLogicListener : public ExitGames::Common::ToString
{
public:
	using ToString::toString;
	virtual void stateUpdate(State newState) = 0;
	ExitGames::Common::JString& toString(ExitGames::Common::JString& retStr, bool withTypes = false) const override;
};

class StateAccessor
{
public:
	State getState(void) const;
	void setState(State newState);
	void registerForStateUpdates(NetworkLogicListener* listener);

private:
	State mState;
	ExitGames::Common::JVector<NetworkLogicListener*> mStateUpdateListeners;
};


class Networking : private ExitGames::LoadBalancing::Listener
{
public:
	Networking();

	static std::shared_ptr<Networking> instance;
	static ExitGames::Common::JString appId; // set your app id here

	void registerForStateUpdates(NetworkLogicListener* listener);
	void run(void);
	void runBasic(void);
	void sendKeepAlive(void);
	void reconnect(void);
	void connect(void);
	void changeRoom(std::string roomName);
	bool leaveRoom(void);
	void opCreateRoom(void);
	void opJoinRandomRoom(void);
	void opJoinOrCreateRoom(void);
	void disconnect(void);
	int getPeerId(void);
	int getLocalId(void);
	int getServerTime(void);
	int getPlayerCount(void);
	bool getIsHost(void);

	//Check if the local player is the master client
	bool IsHost()
	{
		if (mLoadBalancingClient.getIsInRoom())
			return mLoadBalancingClient.getCurrentlyJoinedRoom().getMasterClientID() == getLocalId();
		else
			return false;
	}

	template <typename T>
	void sendEvent(bool reliable, T parameters, nByte eventCode, nByte channelId = 0)
	{
		ExitGames::LoadBalancing::RaiseEventOptions evOptions = ExitGames::LoadBalancing::RaiseEventOptions(channelId, ExitGames::Lite::EventCache::DO_NOT_CACHE);
		mLoadBalancingClient.opRaiseEvent(reliable, parameters, eventCode, evOptions);
	}

	template <typename T>
	void sendEventCached(bool reliable, T parameters, nByte eventCode, nByte channelId = 0)
	{
		ExitGames::LoadBalancing::RaiseEventOptions evOptions = ExitGames::LoadBalancing::RaiseEventOptions(channelId, ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE);
		mLoadBalancingClient.opRaiseEvent(reliable, parameters, eventCode, evOptions);
	}

	template <typename T>
	void sendEventCachedOverwrite(bool reliable, T parameters, nByte eventCode, nByte channelId = 0) //Overwrite any existing event of this type before caching.
	{
		ExitGames::LoadBalancing::RaiseEventOptions evOptions = ExitGames::LoadBalancing::RaiseEventOptions(channelId, ExitGames::Lite::EventCache::REMOVE_FROM_ROOM_CACHE);
		ExitGames::LoadBalancing::RaiseEventOptions _evOptions = ExitGames::LoadBalancing::RaiseEventOptions(channelId, ExitGames::Lite::EventCache::ADD_TO_ROOM_CACHE, 0, 0,
			ExitGames::Lite::ReceiverGroup::OTHERS);
		
		//Send the event to erase any existing entries from the cache.
		mLoadBalancingClient.opRaiseEvent(reliable, NULL, eventCode, evOptions);
		//Send the actual event, and add it to the cache.
		mLoadBalancingClient.opRaiseEvent(reliable, parameters, eventCode, evOptions);
	}

	Input getLastInput(void) const;
	void setLastInput(Input newInput);
	State getState(void) const;

	void (*OnHostGame)();
	void (*OnNpcUpdate)();
	void (*OnPositionUpdate)();
	void (*OnSkyrimMessage)();
	void (*OnSetUserVariable)();
	void (*OnSetUserVariables)();
	void (*OnSetRoomVariable)();
	void (*OnSetRoomVariables)();
	void (*OnMakeMaster)();
	void (*OnSpawnPlayerResponse)();
	void (*OnMasterSet)();
	void (*OnLocationUpdate)();
	void (*ReceivedList)();
	void (*OnConnectionReplication)();
	void (*OnConnected)();
	void (*OnDisconnected)();
	void (*OnExit)(int playerNr);
	void (*OnRoomEnter)();
	void (*OnReceiveEvent)(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent);
	void (*_MESSAGE)(const char* message);

	bool isConnected;
private:
	// receive and print out debug out here
	void debugReturn(int debugLevel, const ExitGames::Common::JString& string) override;

	// implement your error-handling here
	void connectionErrorReturn(int errorCode) override;
	void clientErrorReturn(int errorCode) override;
	void warningReturn(int warningCode) override;
	void serverErrorReturn(int errorCode) override;

	// events, triggered by certain operations of all players in the same room
	void joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& playernrs, const ExitGames::LoadBalancing::Player& player) override;
	void leaveRoomEventAction(int playerNr, bool isInactive) override;
	void customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent) override;

	void onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats) override;
	void onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats) override;
	void onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers) override;

	// callbacks for operations on PhotonLoadBalancing server
	void connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& cluster) override;
	void disconnectReturn(void) override;
	void createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
	void joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
	void joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
	void joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString) override;
	void leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString) override;
	void joinLobbyReturn(void) override;
	void leaveLobbyReturn(void) override;

	ExitGames::LoadBalancing::Client mLoadBalancingClient;
	ExitGames::Common::JString mLastJoinedRoom;
	int mLastPlayerNr;
	ExitGames::Common::Logger mLogger;
	StateAccessor mStateAccessor;
	Input mLastInput;
	bool mAutoJoinRoom;
};
