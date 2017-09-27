#include "GameMenus.h"

const _CreateUIMessageData CreateUIMessageData = (_CreateUIMessageData)0x00547A00;

IMenu::IMenu() :
	view(NULL),
	unk0C(3),
	flags(0),
	unk14(0x12),
	unk18(NULL)
{
}

UInt32 IMenu::ProcessUnkData1(UnkData1* data)
{
	if (data->unk04 == 6)
	{
		if (view && data->data)
		{
			view->HandleEvent(data->data->unk08);
			return 0;
		}
	}
	return 2;
}

void IMenu::Render(void)
{
	if (view)
		view->Render();
}

bool MenuManager::IsMenuOpen(BSFixedString * menuName)
{
	return CALL_MEMBER_FN(this, IsMenuOpen)(menuName);
}

GFxMovieView * MenuManager::GetMovieView(BSFixedString * menuName)
{
	IMenu * menu = GetMenu(menuName);
	if (!menu)
		return NULL;

	return menu->view;
}

// Added this function here because otherwise we need to include a lot of CPP files that aren't really needed.
// Also I changed a bit to increase ref count while we use console.
IMenu * MenuManager::GetMenu(BSFixedString * menuName)
{
	if (!menuName->data)
		return NULL;

	MenuTableItem * item = menuTable.Find(menuName);

	if (!item)
		return NULL;

	IMenu * menu = item->menuInstance;
	if (!menu)
		return NULL;

	const int incRef = 0x93F050;
	_asm
	{
		pushad
		pushfd
			mov ecx, menu
			call incRef
			popfd
			popad
	}

	return menu;
}

RaceMenuSlider::RaceMenuSlider(UInt32 filterFlag, const char * sliderName, const char * callbackName, UInt32 sliderId, UInt32 index, UInt32 type, UInt8 unk8, float min, float max, float value, float interval, UInt32 unk13)
{
	CALL_MEMBER_FN(this, Construct)(filterFlag, sliderName, callbackName, sliderId, index, type, unk8, min, max, value, interval, unk13);
}

TESObjectREFR * EnemyHealth::GetTarget() const
{
	TESObjectREFR * refr = NULL;
	UInt32 refHandle = (*g_thePlayer)->targetHandle;
	LookupREFRByHandle(&refHandle, &refr);
	if(!refr) {
		refHandle = handle;
		LookupREFRByHandle(&refHandle, &refr);
	}

	return refr;
}