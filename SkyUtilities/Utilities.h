#ifndef __UTILITIES_H
#define __UTILITIES_H

#include <string>
#include <time.h>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <vector>
#include "skse\GameForms.h"
#include "skse\GameBSExtraData.h"
#include "skse\GameInput.h"
#include "skse\GameReferences.h"

#define _USE_MATH_DEFINES
#include <math.h>

#define CONFIG_FILE "TamrielOnline.ini"

class BGSLocationExtended : public TESForm
{
public:
	enum { kTypeID = kFormType_Location };

	// parents
	TESFullName		fullName;	// 14
	BGSKeywordForm	keyword;	// 1C

								// members
	BGSLocation*				parent;		// 28 - init'd to 0
	TESFaction*					faction;	// 2C - init'd to 0
	UInt32						unk30;		// 30 - init'd to 0
	TESObjectREFR*				unk34;		// 34 - init'd to 0
	UInt32						unk38;		// 38 - init'd to 0
	UInt32						unk3C;		// 3C - init'd to 0
	UnkArray					unk40;		// 40
	UnkArray					unk4C;		// 4C
	UInt32						unk58;		// 58 - init'd to 0
	UInt32						unk5C;		// 5C - init'd to 0
	UnkArray					unk60;		// 60
	UInt32						IsLoaded;		// 6C - init'd to 0
	UInt32						unk70;		// 70 - init'd to 0
	UnkArray					unk74;		// 74
	UInt32						unk80;		// 80 - init'd to 0
	UInt8						unk84;		// 84 - init'd to 0
	UInt8						unk85;		// 85 - init'd to 0
	UInt8						pad86[2];	// 86
};

class TESObjectREFREx : public TESObjectREFR
{
public:
	MEMBER_FN_PREFIX(TESObjectREFR);
	DEFINE_MEMBER_FN(GetDistance, float, 0x009026E0, TESObjectREFR* target);
};

class ActorEx : public Actor
{
public:
	MEMBER_FN_PREFIX(Actor);
	DEFINE_MEMBER_FN(KOFA_SetTarget, float*, 0x006BD6C0, float* afCatchUpRadius);
	DEFINE_MEMBER_FN(KOFA_Call, void, 0x006B4500, float afCatchUpRadius, int offsetPosArray, int offsetRotArray, float afCatchUpRadiusImmutable, float afFollowRadius);
	DEFINE_MEMBER_FN(IS_ON_MOUNT_RAW, bool, 0x004A5010);
	DEFINE_MEMBER_FN(CLEAR_KEEP_OFFSET_FROM_ACTOR_RAW, void, 0x006B45D0);

	inline void ClearKeepOffsetFromActor()
	{
		return CALL_MEMBER_FN(this, CLEAR_KEEP_OFFSET_FROM_ACTOR_RAW)();
	}

	inline bool IsOnMount()
	{
		return CALL_MEMBER_FN(this, IS_ON_MOUNT_RAW)();
	}

	inline void KeepOffsetFromActor(Actor* arTarget, float afOffsetX, float afOffsetY, float afOffsetZ, float afOffsetAngleX, float afOffsetAngleY, float afOffsetAngleZ, float afCatchUpRadius, float afFollowRadius)
	{
		if (!loadedState || !arTarget || !arTarget->loadedState)
			return;

		float catchupRadius = afCatchUpRadius;
		float unmutableCatupRadius = afCatchUpRadius;
		float followRadius = afFollowRadius;

		int posOffset[3];
		float angleOffset[3];

		posOffset[0] = afOffsetX;
		posOffset[1] = afOffsetY;
		posOffset[2] = afOffsetZ;

		//This values still need some work, Skyrim multiplies these by a number to get a valid value.
		angleOffset[0] = afOffsetAngleX;
		angleOffset[1] = afOffsetAngleY;
		angleOffset[2] = afOffsetAngleY;

		CALL_MEMBER_FN((ActorEx*)arTarget, KOFA_SetTarget)(&catchupRadius);
		CALL_MEMBER_FN(this, KOFA_Call)(catchupRadius, (int)&posOffset, (int)&angleOffset, unmutableCatupRadius, afFollowRadius);
	}
};

class ExtraLinkedRef : public BSExtraData
{
public:
	enum { kExtraTypeID = (UInt32)kExtraData_LinkedRef };

	ExtraLinkedRef();
	virtual ~ExtraLinkedRef();

	static const UInt32 s_ExtraLinkedRefVtbl = 0x01079AF8;

	static ExtraLinkedRef* Create()
	{
		ExtraLinkedRef* xLinkedRef = (ExtraLinkedRef*)BSExtraData::Create(sizeof(ExtraLinkedRef), s_ExtraLinkedRefVtbl);
		return xLinkedRef;
	}
	
	struct Pair
	{
		BGSKeyword	* keyword;
		UInt32		  ref;
	};

	// @members
	tArray<Pair> pairs;
};

template <typename T> class BSTEventSink;

template <typename EventT, typename EventArgT = EventT>
class TODispatcher
{
	typedef BSTEventSink<EventT> SinkT;

	SimpleLock			lock;				// 000
	tArray<SinkT*>		eventSinks;			// 008
	tArray<SinkT*>		addBuffer;			// 014 - schedule for add
	tArray<SinkT*>		removeBuffer;		// 020 - schedule for remove
	bool				stateFlag;			// 02C - some internal state changed while sending
	char				pad[3];

	MEMBER_FN_PREFIX(TODispatcher);
	DEFINE_MEMBER_FN(AddEventSink_Internal, void, 0x006E3E30, SinkT * eventSink);
	DEFINE_MEMBER_FN(RemoveEventSink_Internal, void, 0x008CE0C0, SinkT * eventSink);
	DEFINE_MEMBER_FN(SendEvent_Internal, void, 0x006EBC10, EventArgT * evn);

public:
	TODispatcher() : stateFlag(false) {}

	void AddEventSink(SinkT * eventSink) { CALL_MEMBER_FN(this, AddEventSink_Internal)(eventSink); }
	void RemoveEventSink(SinkT * eventSink) { CALL_MEMBER_FN(this, RemoveEventSink_Internal)(eventSink); }
	void SendEvent(EventArgT * evn) { CALL_MEMBER_FN(this, SendEvent_Internal)(evn); }

	bool Try_AddEventSink(SinkT* eventSink)
	{
		CALL_MEMBER_FN(this, AddEventSink_Internal)(eventSink);

		for (int i = 0; i < eventSinks.count; i++)
		{
			if (eventSinks[i] == eventSink)
				return true;
		}

		return false;
	}
};

class TOInputEventDispatcher : public TODispatcher<InputEvent, InputEvent*>
{
public:
	UInt32			unk030;		// 030
	BSInputDevice	* keyboard;	// 034
	BSInputDevice	* mouse;	// 038
	BSInputDevice	* gamepad;	// 03C

	bool	IsGamepadEnabled(void);
};

namespace Utilities
{
	class Timer
	{
		private:
			std::clock_t startTime;
			double secondsPassed;

		public:
			Timer();
			virtual ~Timer();

			void StartTimer();
			bool HasMillisecondsPassed(double millisecondsToDelay);
	};

	static UInt32 GetFormID(std::string decimalForm)
	{
		std::stringstream buffer;
		UInt32 tempID;

		buffer.str(std::string());
		buffer << "0x" << std::hex << std::setw(8) << std::setfill('0') << std::atoi(decimalForm.c_str());
		tempID = std::strtoul(buffer.str().c_str(), NULL, 0);
		return tempID;
	}

	static std::string GetFormIDString(std::string partialHexForm)
	{
		std::stringstream buffer;
		buffer.str(std::string());
		buffer << "0x" << std::hex << std::setw(8) << std::setfill('0') << partialHexForm;
		return buffer.str();
	}

	static std::string GetFormIDString(UInt32 decimalForm)
	{
		std::stringstream buffer;
		buffer.str(std::string());
		buffer << std::hex << std::setw(8) << std::setfill('0') << decimalForm;
		return buffer.str();
	}

	static std::string GetLocationString(BGSLocation *location)
	{
		std::stringstream buffer;
		buffer.str(std::string());
		buffer << location;
		return buffer.str();
	}

	static std::string GetCellString(TESObjectCELL *location)
	{
		std::stringstream buffer;
		buffer.str(std::string());
		buffer << location;
		return buffer.str();
	}

	static std::string GetLoadOrderId(int decimalForm)
	{
		std::stringstream buffer;
		buffer.str(std::string());
		buffer << std::hex << std::setw(2) << std::setfill('0') << decimalForm;
		return buffer.str();
	}

	static std::string GetKeyName(BYTE key)
	{
		DWORD sc = MapVirtualKeyA(key, 0);
		// check key for ascii
		BYTE buf[256];
		memset(buf, 0, 256);
		WORD temp;
		DWORD asc = (key <= 32);
		if (!asc && (key != VK_DIVIDE)) asc = ToAscii(key, sc, buf, &temp, 1);
		// set bits
		sc <<= 16;
		sc |= 0x1 << 25; // <- don't care
		if (!asc) sc |= 0x1 << 24; // <- extended bit
								   // convert to ansi string
		if (GetKeyNameTextA(sc, (char *)buf, sizeof(buf)))
			return (char *)buf;
		return "";
	}

	static bool GetKeyPressed(BYTE key)
	{
		return (GetKeyState(key) & 0x80000000) > 0;
	}

	static std::string GetIniVar(std::string iniName, std::string iniSection, std::string iniVariable)
	{
		bool foundSection = false;

		std::string line;
		std::ifstream myfile(iniName);

		if (myfile.is_open())
		{
			while (std::getline(myfile, line))
			{
				std::size_t found = line.find(iniSection);
				if (found != std::string::npos)
					foundSection = true;

				std::size_t foundVar = line.find(iniVariable);
				if (foundVar != std::string::npos && foundSection)
				{
					myfile.close();
					return line.substr(foundVar + iniVariable.length() + 1, std::string::npos);
				}
			}
		}

		myfile.close();
		return "";
	}

	static std::vector<std::string>& split(const std::string& s, char delim, std::vector<std::string>& elems)
	{
		std::stringstream ss(s);
		std::string item;
		while (std::getline(ss, item, delim))
		{
			elems.push_back(item);
		}
		return elems;
	}

	static std::vector<std::string> split(const std::string& s, char delim)
	{
		std::vector<std::string> elems;
		split(s, delim, elems);
		return elems;
	}

	//Strips the first element from a vector
	static std::vector<std::string>* StripFront(std::vector<std::string>* vec)
	{
		vec->erase(vec->begin());
		return vec;
	}

	static float GetAngleOfLineBetweenTwoPoints(POINT p1, POINT p2)
	{
		long xDiff = p2.x - p1.x;
		long yDiff = p2.y - p1.y;
		return atan2(yDiff, xDiff) * (180 / M_PI);
	}
};
#endif