#pragma once

#include "skse/GameTypes.h"
#include "skse/GameEvents.h"
#include "skse/GameCamera.h"
#include "skse/GameReferences.h"

#include "skse/ScaleformCallbacks.h"
#include "skse/ScaleformMovie.h"

#include "skse/Utilities.h"
#include "skse/NiNodes.h"

class TESObjectREFR;
class TESFullName;

class InventoryEntryData;

class UIDelegate;
class UIDelegate_v1;

//// menu implementations

// 1C+
class IMenu : public FxDelegateHandler
{
	struct BSUIScaleformData
	{
		virtual ~BSUIScaleformData() {}

		//	void	** _vtbl;		// 00
		UInt32				unk04; // 04
		void*				unk08; // 08
	};

	struct UnkData1
	{
		BSFixedString		name;	// 00
		UInt32				unk04;	// 04
		BSUIScaleformData*	data;	// 08 - BSUIScaleformData
	};

public:
	IMenu();
	virtual ~IMenu() { CALL_MEMBER_FN(this, dtor)(); } // TODO

	enum {
		kType_PauseGame = 1,
		kType_ShowCursor = 2
	};

	virtual void	Accept(CallbackProcessor * processor) {}
	virtual void	Unk_02(void) {}
	virtual void	Unk_03(void) {}
	virtual UInt32	ProcessUnkData1(UnkData1* data);
	virtual void	NextFrame(UInt32 arg0, UInt32 arg1)	{ CALL_MEMBER_FN(this, NextFrame_internal)(arg0, arg1); }
	virtual void	Render(void);
	virtual void	Unk_07(void) {}
	virtual void	InitMovie(void)		{ CALL_MEMBER_FN(this,InitMovie_internal)(view); }

	GFxMovieView	* view;	// 08 - init'd to 0, a class, virtual fn 0x114 called in dtor
	UInt8			unk0C;		// 0C - init'd to 3
	UInt8			pad0D[3];	// 0D
	UInt32			flags;		// 10 - init'd to 0
	UInt32			unk14;		// 14 - init'd to 0x12
	GRefCountBase	* unk18;	// 18 - holds a reference

	MEMBER_FN_PREFIX(IMenu);
	DEFINE_MEMBER_FN(InitMovie_internal, void, 0xA64A50, GFxMovieView* view);
	DEFINE_MEMBER_FN(NextFrame_internal, void, 0xA64980, UInt32 arg0, UInt32 arg1);
	DEFINE_MEMBER_FN(dtor, void, 0x00A64A10);
};

// 34
class Console : public IMenu
{
public:
	// unk0C - 0x0C
	// Flags - 0x807
	// unk14 - 2

	// C+
	struct Unk20
	{
		struct Unk0
		{
			UInt32	unk0;
			UInt32	unk4;
		};

		struct Unk8
		{
			UInt32	unk0;
		};

		Unk0	unk0;	// 0
		Unk8	unk8;	// 8
	};

	void	* opcodeInfo;	// 1C - constructor checks that opcodes match
	Unk20	unk20;			// 20 - init'd to 0, probably history linked list?
	UInt32	unk2C;			// 2C - init'd to 0
	UInt8	unk30;			// 30 - init'd to 0
};

// 68
class BarterMenu : public IMenu
{
	// unk0C - 0
	// Flags - 0xA489
	// unk14 - 3
	GFxValue	* root;		// 1C
	// ...
	UInt8		unk34;		// 34
};

class BGSHeadPart;
class TESRace;

class RaceMenuSlider
{
public:
	RaceMenuSlider::RaceMenuSlider() {};
	RaceMenuSlider::RaceMenuSlider(UInt32 filterFlag, const char * sliderName, const char * callbackName, UInt32 sliderId, UInt32 index, UInt32 type, UInt8 unk8, float min, float max, float value, float interval, UInt32 unk13);

	enum {
		kTypeHeadPart = 0,
		kTypeUnk1,
		kTypeDoubleMorph,
		kTypePreset,
		kTypeTintingMask,
		kTypeHairColorPreset,
		kTypeUnk6,
		kTypeUnused7,
		kTypeUnk8,
		kTypeUnk9,
		kTypeUnk10
	};

	float	min;	// 00
	float	max;	// 04
	float	value;	// 08
	float	interval;	// 0C
	UInt32	filterFlag;	// 10
	UInt32	type;	// 14
	const char	* name;	// 18
	char	callback[MAX_PATH];	// 1C
	UInt32	index;	// 120
	UInt32	id;	// 124
	UInt32	unk128;	// 128
	UInt32	unk12C;	// 12C - 0x7F7FFFFF
	UInt8	unk130;	// 130
	UInt8	pad131[3]; // 131

	MEMBER_FN_PREFIX(RaceMenuSlider);
	DEFINE_MEMBER_FN(Construct, RaceMenuSlider *, 0x0087D840, UInt32 filterFlag, const char * sliderName, const char * callbackName, UInt32 sliderId, UInt32 index, UInt32 type, UInt8 unk8, float min, float max, float value, float interval, UInt32 unk13);
};

class RaceSexMenu : public IMenu
{
public:
	// unk0C - 3
	// Flags - 0x709
	// unk14 - 3
	void					* menuHandler;	// 1C
	UInt32					unk20;			// 20
	UInt32					unk24;			// 24
	enum {
		kHeadPartsHairLine = 0,
		kHeadPartsHead,
		kHeadPartsEyes,
		kHeadPartsHair,
		kHeadPartsBeard,
		kHeadPartsScars,
		kHeadPartsBrows,
		kNumHeadPartLists
	};
	tArray<BGSHeadPart*>	headParts[kNumHeadPartLists];	// 28 - 70
	/*tArray<BGSHeadPart*>	hairline;		// 28
	tArray<BGSHeadPart*>	head;			// 34
	tArray<BGSHeadPart*>	eyes;			// 40
	tArray<BGSHeadPart*>	hair;			// 4C
	tArray<BGSHeadPart*>	beard;			// 58
	tArray<BGSHeadPart*>	scars;			// 64
	tArray<BGSHeadPart*>	brows;			// 70*/
	RaceSexCamera			camera;			// 7C

	float					unkA4[0x07];	// A4

	struct RaceComponent
	{
		TESRace				* race;			// 00
		tArray<RaceMenuSlider>	sliders;	// 04
		UInt32				unk10;			// 10
	};

	tArray<RaceComponent>	sliderData[2];	// C0
	UInt32					unkD8;			// D8
	UInt32					unkDC;			// DC
	UInt32					unkE0;			// E0
	UInt32					raceIndex;		// E4

	MEMBER_FN_PREFIX(RaceSexMenu);
	DEFINE_MEMBER_FN(LoadSliders, void *, 0x00882290, UInt32 unk1, UInt8 unk2);
};

STATIC_ASSERT(offsetof(RaceSexMenu, sliderData) == 0xC0);
STATIC_ASSERT(offsetof(RaceSexMenu, raceIndex) == 0xE4);

class MapMenu : public IMenu
{
public:
	// unk0C - 3
	// Flags - 0x9005
	// unk14 - 7
	enum
	{
		kMarkerType_Location = 0
	};

	// 20
	struct MarkerData
	{
		TESFullName * name;			// 00
		UInt32		refHandle;		// 04
		void		* unk08;		// 08
		UInt32		type;			// 0C
		UInt32		unk10;			// 10
		UInt32		unk14;			// 14
		UInt32		unk18;			// 18
		UInt32		unk1C;			// 1C
	};

	struct LocalMap
	{
		UInt32					unk00;							// 00
		UInt32					unk04;							// 04
		UInt32					unk08;							// 08
		UInt32					unk0C;							// 0C
		GFxValue				markerData;						// 10
		UInt32					unk20;							// 20
		UInt32					unk24;							// 24
		UInt32					unk28;							// 28
		UInt32					unk2C;							// 2C
		LocalMapCullingProcess	cullingProcess;					// 30
		NiRenderedTexture		* renderedLocalMapTexture;		// 26C
		UInt32					unk270;							// 270
		UInt32					unk274;							// 274
		GFxValue				localMapRoot;					// 278
		GFxValue				mapRoot;						// 288
		GFxMovieView			* view;							// 298
		void					* localMapInputHandler;			// 29C
		UInt32					unk2A0;							// 2A0
		UInt8					unk2A4[4];						// 2A4
	};

	void				* eventSinkMenuOpenCloseEvent; 			// 1C
	void				* mapCameraCallback;					// 20
	UInt32				mapMoveHandler; 						// 24
	UInt32				mapLookHandler; 						// 28
	UInt32				mapZoomHandler; 						// 2C
	UInt32				unk30; 									// 30
	UInt32				unk34; 									// 34
	LocalMap			localMap;								// 38
	UInt32				unk2E0[4];								// 2E0
	tArray<MarkerData>	markers;								// 2F0
	// ..
};

STATIC_ASSERT(offsetof(MapMenu, localMap) == 0x38);
STATIC_ASSERT(offsetof(MapMenu::LocalMap, cullingProcess) == 0x30);
STATIC_ASSERT(offsetof(MapMenu::LocalMap, renderedLocalMapTexture) == 0x26C);
STATIC_ASSERT(offsetof(MapMenu, markers) == 0x2F0);

// 18
class HUDObject
{
public:
	HUDObject::HUDObject(GFxMovieView* movie)
	{
		if(movie)
			InterlockedIncrement(&movie->refCount);
		view = movie;
	}
	virtual ~HUDObject(void)
	{
		object.CleanManaged();

		GFxMovieView * thisView = view;
		if(thisView)
			thisView->ForceCollectGarbage();
	}

	virtual void Update(void) = 0;	// Called per-frame
	virtual UInt8 Unk_02(void * unk1) { return 0; };
	virtual void * Unk_03(void * unk1) { return CALL_MEMBER_FN(this, Impl_Fn03)(unk1); };
	virtual void Unk_04(void) { }; // No implementation?

	UInt32			unk04;		// 04
	GFxMovieView	* view;		// 08
	UInt32			unk0C;		// 0C
	GFxValue		object;		// 10
	
	MEMBER_FN_PREFIX(HUDObject);
	DEFINE_MEMBER_FN(dtor, void, 0x0085FF10);
	DEFINE_MEMBER_FN(Impl_Fn03, void *, 0x0085F030, void * unk1);

	// redirect to formheap
	static void * operator new(std::size_t size)
	{
		return FormHeap_Allocate(size);
	}

	static void * operator new(std::size_t size, const std::nothrow_t &)
	{
		return FormHeap_Allocate(size);
	}

	// placement new
	static void * operator new(std::size_t size, void * ptr)
	{
		return ptr;
	}

	static void operator delete(void * ptr)
	{
		FormHeap_Free(ptr);
	}

	static void operator delete(void * ptr, const std::nothrow_t &)
	{
		FormHeap_Free(ptr);
	}

	static void operator delete(void *, void *)
	{
		// placement delete
	}
};
STATIC_ASSERT(sizeof(HUDObject) == 0x20);

// 30
class Compass : public HUDObject
{
public:
	UInt32	unk20;	// 20
	UInt32	unk24;	// 24
	UInt32	unk28;	// 28
	UInt32	unk2C;	// 2C
};
STATIC_ASSERT(sizeof(Compass) == 0x30);

// A0
class FloatingQuestMarker : public HUDObject
{
public:
	
};

// 58
class HUDNotifications : public HUDObject
{
public:

};

// 68
class EnemyHealth : public HUDObject
{
public:
	UInt32			handle;			// 20
	UInt32			unk24;			// 24
	UInt32			unk28;			// 28
	UInt32			unk2C;			// 2C
	GFxValue		unk30;			// 30
	GFxValue		unk40;			// 40
	GFxValue		text;			// 50
	UInt32			unk5C;			// 5C
	UInt32			unk60;			// 60
	UInt32			unk64;			// 64

	TESObjectREFR	* GetTarget() const;
};
STATIC_ASSERT(offsetof(EnemyHealth, handle) == 0x20);

// 70
class StealthMeter : public HUDObject
{
public:

};

// 28
class HUDChargeMeter : public HUDObject
{
public:

};

// 38?
class HUDMeter : public HUDObject
{
public:
	virtual double GetMaxValue(void);
	
	char	* setMeterPercent;	// 20
	char	* startBlinking;	// 24
	char	* fadeOut;			// 28
	float	unk28;				// 2C
	UInt32	unk2C;				// 30
	UInt32	unk34;				// 34
};
STATIC_ASSERT(sizeof(HUDMeter) == 0x38);

// 38
class ActorValueMeter : public HUDMeter
{
public:
	
};

// 38
class ShoutMeter : public HUDMeter
{
public:
	
};

// 58
class HUDMenu : public IMenu
{
public:
	BSTEventSink<void>	unk1C;	// UserEventEnabledEvent
	tArray<HUDObject*>	hudComponents;	// 20
	UInt32	unk2C;
	UInt32	unk30;
	UInt32	unk34;
	UInt32	unk38;
	UInt32	unk3C;
	UInt32	unk40;
	UInt32	unk44;
	UInt32	unk48;
	UInt32	unk4C;
	UInt32	unk50;
	UInt32	unk54;
};
STATIC_ASSERT(sizeof(HUDMenu) == 0x58);

class CraftingMenu : public IMenu
{
public:
};

// ???
class CraftingSubMenu : public FxDelegateHandler
{
public:
	virtual ~CraftingSubMenu();

	UInt32			unk08;		// 008
	GFxMovieView*	view;		// 00C
	// ...
};

STATIC_ASSERT(offsetof(CraftingSubMenu, view) == 0x00C);

// 158
class EnchantConstructMenu : public CraftingSubMenu
{
public:
	enum
	{
		kFilterFlag_EnchantWeapon	 = 0x1,
		kFilterFlag_DisenchantWeapon = 0x2,
		kFilterFlag_EnchanteArmor	 = 0x4,
		kFilterFlag_DisenchantArmor  = 0x8,
		kFilterFlag_EffectWeapon     = 0x10,
		kFilterFlag_EffectArmor      = 0x20,
		kFilterFlag_SoulGem          = 0x40
	};

	class CategoryListEntry
	{
	public:
		virtual ~CategoryListEntry();

		virtual void Unk1();
		virtual void Unk2();
		virtual void Unk3(); // pure
		virtual void SetData(GFxValue* dataContainer);

	//	void		** _vtbl;	// 00
		UInt32		unk04;		// 04
		UInt32		filterFlag;	// 08
		UInt8		bEquipped;	// 0C
		UInt8		bEnabled;	// 0D
		UInt16		pad0E;		// 0E

		MEMBER_FN_PREFIX(CategoryListEntry);
		DEFINE_MEMBER_FN(SetData, void, 0x0084CF60, GFxValue* target);

		// Implemented in Hooks_Scaleform - note the extra parameter
		void SetData_Extended(EnchantConstructMenu*	subMenu, GFxValue* target);
	};

	// 014
	class ItemChangeEntry : public CategoryListEntry
	{
	public:
		InventoryEntryData*	data;	// 10
		UInt32				unk14;	// 14
		UInt32				unk18;	// 18
	};

	// 01C
	class EnchantmentEntry : public CategoryListEntry
	{
	public:
		EnchantmentItem*	data;	// 10
	};

	// ...
};

// 0E8
class SmithingMenu : public CraftingSubMenu
{
public:


	// ...
};

// 0E0
class ConstructibleObjectMenu : public CraftingSubMenu
{
public:
	// 08
	struct EntryData
	{
		BGSConstructibleObject*	object;			// 00
		UInt32					filterFlag;		// 04
	};

	// ...
};

// Declared outside of AlchemyMenu for forward decls
// 08
struct AlchemyEffectCategory
{
	UInt32 formId;
	UInt32 unk1;
};

// 100
class AlchemyMenu : public CraftingSubMenu
{
public:
	// 0C
	struct EntryData
	{
		InventoryEntryData*	data;		// 00
		UInt32				filterFlag;	// 04
		UInt8				bEquipped;	// 08
		UInt8				bEnabled;	// 09
		UInt16				pad0E;		// 0A
	};

	// ...
};



// HUDMenu
// unk0C - 2
// Flags - 0x18902
// unk14 - 0x12

// DialogueMenu
// unk0C - 3
// Flags - 0x4400
// unk14 - 1

// MainMenu
// unk0C - 9
// Flags - 0x581
// unk14 - 1

// MagicMenu
// unk0C - 0
// Flags - 0xA489
// unk14 - 3

// InventoryMenu
// unk0C - 0
// Flags - 0x4400
// unk14 - 0x12

//// menu management

// 08
class IUIMessageData
{
public:
	virtual ~IUIMessageData();

//	void	** _vtbl;	// 00
	UInt8	unk04;		// 04
	UInt8	unk05;		// 05
	UInt8	pad06[2];	// 06
};

// 14
class BSUIMessageData : public IUIMessageData
{
public:
	BSString			* unk08;	// 08
	StringCache::Ref	unk0C;		// 0C
	UInt32				unk10;		// 10
};

// 0C
class RefHandleUIData : public IUIMessageData
{
public:
	UInt32	refHandle;	// 08
};

typedef void * (* _CreateUIMessageData)(BSFixedString * name);
extern const _CreateUIMessageData CreateUIMessageData;

// 10
// ### pool added in 1.3 (or maybe 1.2)
class UIMessage
{
public:
	enum
	{
		kMessage_Refresh = 0,	// used after ShowAllMapMarkers
		kMessage_Open,
		kMessage_PreviouslyKnownAsClose,
		kMessage_Close
	};

	StringCache::Ref	strData;	// 00
	UInt32				message;	// 04
	IUIMessageData		* objData;	// 08 - something with a virtual destructor
	UInt8				isPooled;	// 0C
	UInt8				pad0D[3];	// 0D
};

// 04
template <typename T>
class BSTMessageQueue
{
public:
	BSTMessageQueue();
	virtual ~BSTMessageQueue();

	virtual bool	Push(T * obj);		// pure, add (loop until lock taken, call Fn05)
	virtual bool	TryPush(T * obj);	// pure, try add (try to take lock, return false if already taken, else call Fn05)
	virtual bool	Pop(T * obj);		// pure, remove (loop until lock taken, call Fn06)
	virtual bool	TryPop(T * obj);	// pure, try remove (try to take lock, return false if already taken, else call Fn06)

//	void	** _vtbl;	// 00
};

// 08
template <typename T>
class BSTCommonMessageQueue : public BSTMessageQueue <T>
{
protected:
	virtual bool	PushInternal(T * obj);	// pure
	virtual bool	PopInternal(T * obj);	// pure

public:
	volatile UInt32	lock;	// 04
};

template <typename T>
class BSTCommonScrapHeapMessageQueue : public BSTCommonMessageQueue<T>
{
public:
	BSTCommonScrapHeapMessageQueue();
	virtual ~BSTCommonScrapHeapMessageQueue();

	UInt32 unk08;	// 08
	UInt32 unk0C;	// 0C
	UInt32 unk10;	// 10
};

// 08 + sizeof(T) * T_len + 0C
template <typename T, UInt32 T_len>
class BSTCommonStaticMessageQueue : public BSTCommonMessageQueue <T>
{
public:
	T		data[T_len];	// 008
	UInt32	numEntries;		// 198 - offsets are for <UIMessage *, 100>
	UInt32	writePtr;		// 19C
	UInt32	readPtr;		// 1A0
};

// 1C8
// 5CC - pool added in 1.2 or 1.3
class UIManager
{
public:
	enum
	{
		kPoolSize = 0x40,
	};

	typedef BSTCommonStaticMessageQueue <UIMessage *, 100>	MessageQueue;

	UInt32			unk000;		// 000
	MessageQueue	messages;	// 004
	UInt32			pad1A8[(0x1C8 - 0x1A8) / 4];	// 1A8
	UInt32			poolUsed;	// 1C8
	UIMessage		messagePool[kPoolSize];	// 1CC

	MEMBER_FN_PREFIX(UIManager);
	// this takes ownership of the message ptr
//	DEFINE_MEMBER_FN(AddMessage, void, 0x004503E0, UIMessage * msg);	// old 1.1 implementation
	// 1.3 uses a little non-thread-safe pool of UIMessages to wrap around the nicely thread-safe BSTMessageQueue it gets added to
	DEFINE_MEMBER_FN(AddMessage, void, 0x00431B00, StringCache::Ref * strData, UInt32 msgID, void * objData);

	static UIManager *	GetSingleton(void)
	{
		return *((UIManager **)0x012E35E4);
	}

	// Used by Hooks_UI
	void ProcessCommands(void);
	void QueueCommand(UIDelegate* cmd);
	void QueueCommand(UIDelegate_v1* cmd);

	DEFINE_MEMBER_FN(ProcessEventQueue_HookTarget, void, 0x00A5C270);
};

// 11C
class UIStringHolder
{
public:
	void			* unk00;					// 000
	BSFixedString	faderData;					// 004 "FaderData"
	BSFixedString	hudData;					// 008 "HUDData"
	BSFixedString	hudCamData;					// 00C "HUDCamData"
	BSFixedString	floatingQuestMarkers;		// 010 "FloatingQuestMarkers"
	BSFixedString	consoleData;				// 014 "ConsoleData"
	BSFixedString	quantityData;				// 018 "QuantityData"
	BSFixedString	messageBoxData;				// 01C "MessageBoxData"
	BSFixedString	bsUIScaleformData;			// 020 "BSUIScaleformData"
	BSFixedString	bsUIMessageData;			// 024 "BSUIMessageData"
	BSFixedString	bsUIAnalogData;				// 028 "BSUIAnalogData"
	BSFixedString	inventoryUpdateData;		// 02C "InventoryUpdateData"
	BSFixedString	refHandleUIData;			// 030 "RefHandleUIData"
	BSFixedString	tesFormUIData;				// 034 "TESFormUIData"
	BSFixedString	loadingMenuData;			// 038 "LoadingMenuData"
	BSFixedString	kinectStateData;			// 03C "KinectStateChangeData"
	BSFixedString	kinectUserEventData;		// 040 "KinectUserEventData"
	BSFixedString	inventoryMenu;				// 044 "InventoryMenu"
	BSFixedString	console;					// 048 "Console"
	BSFixedString	dialogueMenu;				// 04C "Dialogue Menu"
	BSFixedString	hudMenu;					// 050 "HUD Menu"
	BSFixedString	mainMenu;					// 054 "Main Menu"
	BSFixedString	messageBoxMenu;				// 058 "MessageBoxMenu"
	BSFixedString	cursorMenu;					// 05C "Cursor Menu"
	BSFixedString	faderMenu;					// 060 "Fader Menu"
	BSFixedString	magicMenu;					// 064 "MagicMenu"
	BSFixedString	topMenu;					// 068 "Top Menu"
	BSFixedString	overlayMenu;				// 06C "Overlay Menu"
	BSFixedString	overlayInteractionMenu;		// 070 "Overlay Interaction Menu"
	BSFixedString	loadingMenu;				// 074 "Loading Menu"
	BSFixedString	tweenMenu;					// 078 "TweenMenu"
	BSFixedString	barterMenu;					// 07C "BarterMenu"
	BSFixedString	giftMenu;					// 080 "GiftMenu"
	BSFixedString	debugTextMenu;				// 084 "Debug Text Menu"
	BSFixedString	mapMenu;					// 088 "MapMenu"
	BSFixedString	lockpickingMenu;			// 08C "Lockpicking Menu"
	BSFixedString	quantityMenu;				// 090 "Quantity Menu"
	BSFixedString	statsMenu;					// 094 "StatsMenu"
	BSFixedString	containerMenu;				// 098 "ContainerMenu"
	BSFixedString	sleepWaitMenu;				// 09C "Sleep/Wait Menu"
	BSFixedString	levelUpMenu;				// 0A0 "LevelUp Menu"
	BSFixedString	journalMenu;				// 0A4 "Journal Menu"
	BSFixedString	bookMenu;					// 0A8 "Book Menu"
	BSFixedString	favoritesMenu;				// 0AC "FavoritesMenu"
	BSFixedString	raceSexMenu;				// 0B0 "RaceSex Menu"
	BSFixedString	craftingMenu;				// 0B4 "Crafting Menu"
	BSFixedString	trainingMenu;				// 0B8 "Training Menu"
	BSFixedString	mistMenu;					// 0BC "Mist Menu"
	BSFixedString	tutorialMenu;				// 0C0 "Tutorial Menu"
	BSFixedString	creditsMenu;				// 0C4 "Credits Menu"
	BSFixedString	titleSequenceMenu;			// 0C8 "TitleSequence Menu"
	BSFixedString	consoleNativeUIMenu;		// 0CC "Console Native UI Menu"
	BSFixedString	kinectMenu;					// 0D0 "Kinect Menu"
	BSFixedString	textWidget;					// 0D4 "TextWidget"
	BSFixedString	buttonBarWidget;			// 0D8 "ButtonBarWidget"
	BSFixedString	graphWidget;				// 0DC "GraphWidget"
	BSFixedString	textureWidget;				// 0E0 "TextureWidget"
	BSFixedString	uiMenuOK;					// 0E4 "UIMenuOK"
	BSFixedString	uiMenuCancel;				// 0E8 "UIMenuCancel"
	BSFixedString	showText;					// 0EC "Show Text"
	BSFixedString	hideText;					// 0F0 "Hide Text"
	BSFixedString	showList;					// 0F4 "Show List"
	BSFixedString	voiceReady;					// 0F8 "Voice Ready"
	BSFixedString	dmfoStr;					// 0FC "DMFOStr"
	BSFixedString	showJournal;				// 100 "Show Journal"
	BSFixedString	journalSettingsSaved;		// 104 "Journal Settings Saved"
	BSFixedString	closeMenu;					// 108 "CloseMenu"
	BSFixedString	closingAllMenus;			// 10C "Closing All Menus"
	BSFixedString	refreshMenu;				// 110 "RefreshMenu"
	BSFixedString	menuTextureDegradeEvent;	// 114 "Menu Texture Degrade Event"
	BSFixedString	diamondMarker;				// 118 "<img src='DiamondMarker' width='10' height='15' align='baseline' vspace='5'>"

	static UIStringHolder *	GetSingleton(void)
	{
		return *((UIStringHolder **)0x012E35E0);
	}
};

// E4
class Inventory3DManager
{
public:
	virtual ~Inventory3DManager();

	static Inventory3DManager * GetSingleton(void)
	{
		return *((Inventory3DManager **)0x01B2E99C);
	}

//	void			** _vtbl;	// 00
	UInt32			unk04;
	UInt32			unk08; // This appears to be 1 when a menu is open
	UInt32			unk0C;
	float			unk10[(0x30 - 0x10) / 4];
	UInt32			unk30;
	TESObjectREFR	* object;	// 34
	UInt32			unk38;
	UInt32			unk3C;
	UInt32			unk40;

	struct ItemData
	{
		TESForm	* unk04;
		TESForm	* unk08;
		void	* unk0C;
		void	* unk10;
		float	unk14;
	};

	ItemData		unk44[7];
	UInt32			unkD0; // Number of ItemDatas?
	UInt32			unkD4;
	UInt32			unkD8;
	UInt32			unkDC;
	UInt8			unkE0;
	UInt8			unkE1; // Somekind of mode (0 for MagicMenu)
	UInt8			unkE2;
	UInt8			padE3;

	MEMBER_FN_PREFIX(Inventory3DManager);
	DEFINE_MEMBER_FN(UpdateItem3D, void, 0x00867C00, InventoryEntryData * objDesc);
	DEFINE_MEMBER_FN(UpdateMagic3D, void, 0x00867930, TESForm * form, UInt32 unk1);
	DEFINE_MEMBER_FN(Clear3D, void, 0x008668C0);
	DEFINE_MEMBER_FN(Render, UInt32, 0x00867730);

	/*DEFINE_MEMBER_FN(Unk1, void, 0x008667E0, UInt32 unk1);
	DEFINE_MEMBER_FN(Unk2, void, 0x00867110);
	DEFINE_MEMBER_FN(Unk3, bool, 0x008664C0);
	DEFINE_MEMBER_FN(Unk4, double, 0x008663E0);
	DEFINE_MEMBER_FN(Unk5, bool, 0x008418D0);
	DEFINE_MEMBER_FN(Unk6, int, 0x00867730);*/
};

STATIC_ASSERT(offsetof(Inventory3DManager, unk10) == 0x10);
STATIC_ASSERT(offsetof(Inventory3DManager, object) == 0x34);
STATIC_ASSERT(offsetof(Inventory3DManager, unkE0) == 0xE0);

// 00C
class MenuTableItem
{
public:
	BSFixedString	name;				// 000
	IMenu			* menuInstance;		// 004	0 if the menu is not currently open
	void			* menuConstructor;	// 008

	bool operator==(const MenuTableItem & rhs) const	{ return name == rhs.name; }
	bool operator==(const BSFixedString a_name) const	{ return name == a_name; }
	operator UInt32() const								{ return (UInt32)name.data; }

	static inline UInt32 GetHash(BSFixedString * key)
	{
		UInt32 hash;
		CRC32_Calc4(&hash, (UInt32)key->data);
		return hash;
	}

	void Dump(void)
	{
		_MESSAGE("\t\tname: %s", name);
		_MESSAGE("\t\tinstance: %08X", menuInstance);
	}
};

// 11C
class MenuManager
{
	typedef tHashSet<MenuTableItem,BSFixedString> MenuTable;

	// 030
	struct Unknown3
	{
		UInt32		freqLow;	// 000 (= Frequency.LowPart)
		UInt32		freqHigh;	// 004 (= Frequency.HighPart)

		UInt32		unk_008;	// 008 (= 0)
		UInt32		unk_00C;	// 00C (= 0)
		UInt32		unk_010;	// 010 (= 0)
		UInt32		unk_014;	// 014 (= 0)
		UInt32		unk_018;	// 018 (= 0)
		UInt32		unk_01C;	// 018 (= frequency related)
		
		UInt32		unk_020;	// 020
		UInt32		unk_024;	// 024

		UInt32		unk_028;	// 028 (= 0)
		UInt32		unk_02C;	// 02C (= 0)
	};
	STATIC_ASSERT(sizeof(Unknown3) == 0x30);

private:
	UInt32					unk_000;	// 000

	EventDispatcher<MenuOpenCloseEvent>		menuOpenCloseEventDispatcher;	// 004
	EventDispatcher<MenuModeChangeEvent>	menuModeChangeEventDispatcher;	// 034
	EventDispatcher<void*>					unk_064;						// 064 - New in 1.6.87.0 - Kinect related?

	UnkArray				menuStack;	// 094
	UInt32					unk_0A0;	// 0A0
	MenuTable				menuTable;	// 0A4
	UInt32					unk_0C0;	// 0C0 (= 0)
	UInt32					unk_0C4;	// 0C4 (= 0)
	UInt32					unk_0C8;	// 0C8 (= 0)
	UInt32					unk_0CC;	// 0CC (= 0)
	UInt32					unk_0D0;	// 0D0 (= 0)
	UInt32					unk_0D4;	// 0D4 (= 0)
	UInt32					unk_0D8;	// 0D8 (= 0)
	UInt32					unk_0DC;	// 0DC (= 0)
	UInt32					unk_0E0;	// 0E0 (= 0)
	UInt32					unk_0E4;	// 0E4
	Unknown3				unk_0E8;
	bool					unk_118;	// 118 (= 0)
	bool					unk_119;	// 119 (= 0)
	char					pad[2];

public:
	typedef IMenu*	(*CreatorFunc)(void);

private:
	MEMBER_FN_PREFIX(MenuManager);
	DEFINE_MEMBER_FN(IsMenuOpen, bool, 0x00A5CE90, BSFixedString * menuName);
	DEFINE_MEMBER_FN(Register_internal, void, 0x00A5D2A0, const char * name, CreatorFunc creator);

public:

	static MenuManager * GetSingleton(void)
	{
		return *((MenuManager **)0x012E3548);
	}

	EventDispatcher<MenuOpenCloseEvent> * MenuOpenCloseEventDispatcher()
	{
		return &menuOpenCloseEventDispatcher;
	}

	bool				IsMenuOpen(BSFixedString * menuName);
	IMenu *				GetMenu(BSFixedString * menuName);
	GFxMovieView *		GetMovieView(BSFixedString * menuName);

	typedef IMenu* (*CreatorFunc)(void);

	void Register(const char* name, CreatorFunc creator)
	{
		CALL_MEMBER_FN(this, Register_internal)(name, creator);
	}
};
STATIC_ASSERT(sizeof(MenuManager) == 0x11C);