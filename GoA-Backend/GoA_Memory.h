#ifndef __GOA_MEMORY_H__
#define __GOA_MEMORY_H__

#pragma warning(push)
#pragma warning(disable : 4244)

extern "C"
{
#include <lua.h>
}

#include "GoA_Globals.h"
#include "GoA_Defines.h"

template<typename T>
T GoA_Read(u8* address)
{
	return *(T*)address;
}

template<typename T>
void GoA_Write(u8* address, T value)
{
	*(T*)((void*)address) = value;
}

void GoA_WriteString(u8* address, const char *value, size_t length = 0);

template<typename T>
int Lua_Read(lua_State* L)
{
	u64 offset = (u64)lua_tonumber(L, 1);
	if (KH2_BASE_ADDRESS + offset < KH2_BASE_ADDRESS)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	T value = GoA_Read<T>(KH2_BASE_ADDRESS + offset);
	lua_pushnumber(L, value);
	
	return 1;
}

template<typename T>
int Lua_ReadA(lua_State* L)
{
	u64 address = (u64)lua_tonumber(L, 1);
	if (address < (u64)KH2_BASE_ADDRESS)
	{
		lua_pushnumber(L, 0);
		return 1;
	}

	T value = GoA_Read<T>((u8*)((void*)address));
	lua_pushnumber(L, value);

	return 1;
}

template<typename T>
int Lua_Write(lua_State* L)
{
	u64 offset = (u64)lua_tonumber(L, 1);
	T value = static_cast<T>(lua_tonumber(L, 2));
	if (KH2_BASE_ADDRESS + offset < KH2_BASE_ADDRESS)
	{
		return 0;
	}

	GoA_Write<T>(KH2_BASE_ADDRESS + offset, value);

	return 0;
}

template<typename T>
int Lua_WriteA(lua_State* L)
{
	u64 address = (u64)lua_tonumber(L, 1);
	T value = static_cast<T>(lua_tonumber(L, 2));
	if (address < (u64)KH2_BASE_ADDRESS)
	{
		return 0;
	}

	GoA_Write<T>((u8*)((void*)address), value);

	return 0;
}

int Lua_WriteString(lua_State* L);
int Lua_WriteStringA(lua_State* L);

#pragma warning(pop)

#endif // __GOA_MEMORY_H__
