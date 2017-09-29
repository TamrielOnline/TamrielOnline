#include "Networking.h"

#include <string>
#include <cstdlib>
#include "NetworkState.h"

#define GETTICKMS() static_cast<int>(GetTickCount())

std::shared_ptr<Networking> Networking::instance;
ExitGames::Common::JString Networking::appId; // set your app id here

static const ExitGames::Common::JString appVersion = L"1.0";
static ExitGames::Common::JString USER_NAME;

static const bool autoLobbyStats = true;
#if defined EG_PLATFORM_SUPPORTS_CPP11 && defined EG_PLATFORM_SUPPORTS_MULTITHREADING
// pinging takes a moment, so for a real game it makes sense to retrive the best region with getRegionWithBestPing(), store it in a file, use RegionSelectionMode::SELECT and pass that region to selectRegion() and to only use RegionSelectionMode::BEST, if no best region has been determined yet or if the player explicitly requests to repeat pinging
static const nByte regionSelectionMode = ExitGames::LoadBalancing::RegionSelectionMode::BEST;
#else
	static const nByte regionSelectionMode = ExitGames::LoadBalancing::RegionSelectionMode::SELECT;
#endif

ExitGames::Common::JString& NetworkLogicListener::toString(ExitGames::Common::JString& retStr, bool /*withTypes*/) const
{
	return retStr;
}

State StateAccessor::getState(void) const
{
	return mState;
}

void StateAccessor::setState(State newState)
{
	mState = newState;
	for (unsigned int i = 0; i < mStateUpdateListeners.getSize(); i++)
		mStateUpdateListeners[i]->stateUpdate(newState);
}

void StateAccessor::registerForStateUpdates(NetworkLogicListener* listener)
{
	mStateUpdateListeners.addElement(listener);
}

Input Networking::getLastInput(void) const
{
	return mLastInput;
}

void Networking::setLastInput(Input newInput)
{
	mLastInput = newInput;
}

State Networking::getState(void) const
{
	return mStateAccessor.getState();
}

// functions
Networking::Networking()
#ifdef _EG_MS_COMPILER
#	pragma warning(push)
#	pragma warning(disable:4355)
#endif
	: mLoadBalancingClient(*this, appId, appVersion, ExitGames::Photon::ConnectionProtocol::UDP, autoLobbyStats, regionSelectionMode)
	  , mLastPlayerNr(-1)
	  , mLastInput(INPUT_NON)

#ifdef _EG_MS_COMPILER
#	pragma warning(pop)
#endif
{
	mStateAccessor.setState(STATE_INITIALIZED);
	mLoadBalancingClient.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // that instance of LoadBalancingClient and its implementation details
	mLoadBalancingClient.setDisconnectTimeout(300000); //30 minutes
	mLogger.setListener(*this);
	mLogger.setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // this class
	ExitGames::Common::Base::setListener(this);
	ExitGames::Common::Base::setDebugOutputLevel(DEBUG_RELEASE(ExitGames::Common::DebugLevel::INFO, ExitGames::Common::DebugLevel::WARNINGS)); // all classes that inherit from Base
}

void Networking::registerForStateUpdates(NetworkLogicListener* listener)
{
	mStateAccessor.registerForStateUpdates(listener);
}

void Networking::connect()
{
	//Connecting to Photon
	mLoadBalancingClient.connect();
	mStateAccessor.setState(STATE_CONNECTING);
}

void Networking::disconnect(void)
{
	mLoadBalancingClient.disconnect();
}

void Networking::opCreateRoom(void)
{
	//Creating room
	ExitGames::Common::JString name(L"Tamriel");
	mLoadBalancingClient.opCreateRoom(name, ExitGames::LoadBalancing::RoomOptions().setMaxPlayers(255).setPlayerTtl(INT_MAX / 2).setEmptyRoomTtl(10000));
	mStateAccessor.setState(STATE_JOINING);
}

void Networking::opJoinRandomRoom(void)
{
	mLoadBalancingClient.opJoinRandomRoom();
}

void Networking::opJoinOrCreateRoom(void)
{
	ExitGames::LoadBalancing::RoomOptions roomOptions(true, true, 255);
	roomOptions.setPlayerTtl(INT_MAX / 2);
	roomOptions.setEmptyRoomTtl(10000);

	//Joining or creating room
	ExitGames::Common::JString name(L"Tamriel");
	mLoadBalancingClient.opJoinOrCreateRoom(name, roomOptions, 0);
	mStateAccessor.setState(STATE_JOINING);
}

void Networking::run(void)
{
	mLoadBalancingClient.service();
}

void Networking::runBasic(void)
{
	mLoadBalancingClient.serviceBasic();
}

void Networking::sendKeepAlive(void)
{
	mLoadBalancingClient.sendAcksOnly();
}

// protocol implementations

void Networking::debugReturn(int debugLevel, const ExitGames::Common::JString& string)
{
	//Debug output
}

bool Networking::leaveRoom()
{
	return mLoadBalancingClient.opLeaveRoom(false, false);
}

void Networking::reconnect()
{
	mLoadBalancingClient.reconnectAndRejoin();
}

void Networking::changeRoom(std::string roomName)
{
	ExitGames::LoadBalancing::RoomOptions roomOptions(true, true, 255);
	roomOptions.setPlayerTtl(INT_MAX / 2);
	roomOptions.setEmptyRoomTtl(10000);

	//Joining or creating room
	ExitGames::Common::JString name(roomName.c_str());
	mLoadBalancingClient.opJoinOrCreateRoom(name, roomOptions, 0);
	mStateAccessor.setState(STATE_JOINING);
}

void Networking::connectionErrorReturn(int errorCode)
{
	if (mLoadBalancingClient.reconnectAndRejoin())
		return;

	//Received connection error
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	mStateAccessor.setState(STATE_DISCONNECTED);
	OnDisconnected();
	_MESSAGE("Disconnected due to an connection error.");
}

void Networking::clientErrorReturn(int errorCode)
{
	//Received error from client
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	_MESSAGE("Client error.");
}

void Networking::warningReturn(int warningCode)
{
	//Received warning from client
	EGLOG(ExitGames::Common::DebugLevel::WARNINGS, L"code: %d", warningCode);
}

void Networking::serverErrorReturn(int errorCode)
{
	//Received error from server
	EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"code: %d", errorCode);
	_MESSAGE("Server error.");
}

void Networking::joinRoomEventAction(int playerNr, const ExitGames::Common::JVector<int>& /*playernrs*/, const ExitGames::LoadBalancing::Player& player)
{
	//A player has joined the game
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls joined the game", player.getName().cstr());
}

void Networking::leaveRoomEventAction(int playerNr, bool isInactive)
{
	//A player has left the game
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	OnExit(playerNr);
}

void Networking::customEventAction(int playerNr, nByte eventCode, const ExitGames::Common::Object& eventContent)
{
	// you do not receive your own events, unless you specify yourself as one of the receivers explicitly, so you must start 2 clients, to receive the events, which you have sent, as sendEvent() uses the default receivers of opRaiseEvent() (all players in same room like the sender, except the sender itself)
	EGLOG(ExitGames::Common::DebugLevel::ALL, L"");
	OnReceiveEvent(playerNr, eventCode, eventContent);
}

int Networking::getPeerId(void)
{
	return mLoadBalancingClient.getPeerId();
}

int Networking::getLocalId(void)
{
	return mLastPlayerNr;
}

int Networking::getServerTime(void)
{
	return mLoadBalancingClient.getServerTime();
}

int Networking::getPlayerCount(void)
{
	return mLoadBalancingClient.getCurrentlyJoinedRoom().getPlayerCount();
}

void Networking::connectReturn(int errorCode, const ExitGames::Common::JString& errorString, const ExitGames::Common::JString& cluster)
{
	//Connected to cluster
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"connected to cluster " + cluster);
	if (errorCode)
	{
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_DISCONNECTING);
		OnDisconnected();
		return;
	}

	mStateAccessor.setState(STATE_CONNECTED);
	OnConnected();
}

void Networking::disconnectReturn(void)
{
	//Disconnected
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	_MESSAGE("Disconnected");
	mStateAccessor.setState(STATE_DISCONNECTED);
	OnDisconnected();
	mLastPlayerNr = -1;
}

void Networking::createRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if (errorCode)
	{
		//opCreateRoom() failed
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}
	mLastJoinedRoom = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	mLastPlayerNr = localPlayerNr;

	//Room has been created, regularly sending dummy events now.
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	mStateAccessor.setState(STATE_JOINED);
	OnRoomEnter();
}

void Networking::joinOrCreateRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if (errorCode)
	{
		//opJoinOrCreateRoom() failed
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}

	mLastJoinedRoom = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	mLastPlayerNr = localPlayerNr;

	//Room has been entered, regularly sending dummy events now
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	mStateAccessor.setState(STATE_JOINED);
	_MESSAGE("Connected to cluster.");
	OnRoomEnter();
}

void Networking::joinRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& gameProperties, const ExitGames::Common::Hashtable& playerProperties, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if (errorCode)
	{
		//opJoinRoom() failed
		mLastJoinedRoom = L"";
		mLastPlayerNr = 0;
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}

	//Room has been successfully joined, regularly sending dummy events now
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	mStateAccessor.setState(STATE_JOINED);
	//_MESSAGE("We did it, we did it!");
	OnRoomEnter();
}

void Networking::joinRandomRoomReturn(int localPlayerNr, const ExitGames::Common::Hashtable& /*gameProperties*/, const ExitGames::Common::Hashtable& /*playerProperties*/, int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if (errorCode)
	{
		//opJoinRandomRoom() failed
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_CONNECTED);
		return;
	}

	mLastJoinedRoom = mLoadBalancingClient.getCurrentlyJoinedRoom().getName();
	mLastPlayerNr = localPlayerNr;

	//Room has been successfully joined, regularly sending dummy events now
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"localPlayerNr: %d", localPlayerNr);
	mStateAccessor.setState(STATE_JOINED);
	OnRoomEnter();
}

void Networking::leaveRoomReturn(int errorCode, const ExitGames::Common::JString& errorString)
{
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	if (errorCode)
	{
		//opLeaveRoom() failed
		EGLOG(ExitGames::Common::DebugLevel::ERRORS, L"%ls", errorString.cstr());
		mStateAccessor.setState(STATE_DISCONNECTING);
		OnDisconnected();
		return;
	}

	//Room has been successfully left
	mStateAccessor.setState(STATE_LEFT);
	changeRoom(to_string(NetworkState::locationId));
}

void Networking::joinLobbyReturn(void)
{
	//Joined lobby
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
	_MESSAGE("We joined a lobby!");
}

void Networking::leaveLobbyReturn(void)
{
	//Left lobby
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"");
}

void Networking::onLobbyStatsResponse(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
	//The LobbyStats are as follows
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls", lobbyStats.toString().cstr());
}

void Networking::onLobbyStatsUpdate(const ExitGames::Common::JVector<ExitGames::LoadBalancing::LobbyStatsResponse>& lobbyStats)
{
	//The LobbyStats are as follows
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls", lobbyStats.toString().cstr());
}

void Networking::onAvailableRegions(const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegions, const ExitGames::Common::JVector<ExitGames::Common::JString>& availableRegionServers)
{
	//onAvailableRegions, selecting region
	EGLOG(ExitGames::Common::DebugLevel::INFO, L"%ls / %ls", availableRegions.toString().cstr(), availableRegionServers.toString().cstr());
	// select first region from list
	mLoadBalancingClient.selectRegion(availableRegions[0]);
	_MESSAGE("Region selected.");
}

bool Networking::getIsHost()
{
	if (!mLoadBalancingClient.getIsInRoom())
		return false;

	return mLoadBalancingClient.getCurrentlyJoinedRoom().getPlayerForNumber(mLastPlayerNr)->getIsMasterClient();
}
