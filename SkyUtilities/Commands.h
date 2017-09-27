#ifndef _Plugin_Commands_H
#define _Plugin_Commands_H

#include <sstream>
#include <iomanip>
#include "skse\GameMenus.h"
#include <time.h>
#include <fstream>

#define USE_OUR_MUTEX

#ifdef USE_OUR_MUTEX
#include <mutex>
#define LOCK_MUTEX(a) std::lock_guard<std::recursive_mutex> _gLock(a)
static std::recursive_mutex mtx;
#endif

// Structure to get and keep console pointer until we don't need it anymore.
struct ConsoleHolder
{
	ConsoleHolder()
	{
		static BSFixedString * consoleStr = NULL;
		if (consoleStr == NULL)
			consoleStr = new BSFixedString("Console");

		_cptr = (int)MenuManager::GetSingleton()->GetMenu(consoleStr);
	}

	~ConsoleHolder()
	{
		if (_cptr != 0)
		{
			const int decRef = 0x9241A0;
			int lPtr = _cptr;
			_cptr = 0;
			_asm
			{
				pushad
				pushfd
					mov ecx, lPtr
					call decRef
					popfd
					popad
			}
		}
	}

	int GetPointer() { return _cptr; }

private:
	int _cptr;
};

// Execute command in console.
static void nExecuteCommand(const char * text)
{
	ConsoleHolder con;

	int ptr = con.GetPointer();
	if (ptr == 0)
	{
		_MESSAGE("Failed to get console pointer!");
		return;
	}

	int * unkStruct = (int*)malloc(64);
	memset(unkStruct, 0, 64);

	unkStruct[4] = ptr;
	unkStruct[6] = (int)(&unkStruct[7]); // Lazy, there's actually another structure here
										 //unkStruct[8] = 0; // If this is 1 << 6 then [9] is char ** instead of char *
	unkStruct[9] = (int)text;

	const int runScript = 0x847080;
	_asm
	{
		pushad
		pushfd
			mov eax, unkStruct
			push eax
			call runScript
			add esp, 4
			popfd
			popad
	}

	free(unkStruct);
}

static std::string Replace(std::string str, std::string first, std::string second)
{
	if (first.empty() || str.empty())
		return str;

	size_t pos = 0;
	while ((pos = str.find(first, pos)) != std::string::npos)
	{
		str.replace(pos, first.length(), second);
		pos += second.length();
	}
	return str;
}

static std::string FormIdToString(UInt32 decimalForm)
{
	std::stringstream buffer;
	buffer.str(std::string());
	buffer << std::hex << std::setw(8) << std::setfill('0') << decimalForm;
	return buffer.str();
}

static void i_ExecuteCommand(BSFixedString text, TESObjectREFR* target)
{
	if (text.data && *text.data)
	{
		std::string textData = text.data;

		if (target)
			textData = '"' + FormIdToString(target->formID) + "\"." + text.data;

#ifdef USE_OUR_MUTEX
		LOCK_MUTEX(mtx);
#endif

		nExecuteCommand(textData.c_str());
	}
}

static void i_ExecuteCommand(StaticFunctionTag * base, BSFixedString text)
{
	if (text.data && *text.data)
	{
#ifdef USE_OUR_MUTEX
		LOCK_MUTEX(mtx);
#endif

		nExecuteCommand(text.data);
	}
}

#endif