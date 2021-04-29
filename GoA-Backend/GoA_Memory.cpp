#include "GoA_Memory.h"

#include <string.h>

void GoA_WriteString(u8* address, const char* value, size_t length)
{
	if (length == 0)
		length = strlen(value);

	for (size_t i = 0; i < length; i++)
		GoA_Write<char>(address + i, value[i]);
}

int Lua_WriteString(lua_State* L)
{
	if (lua_isstring(L, 2))
	{
		u64 offset = (u64)lua_tonumber(L, 1);
		const char* value = lua_tostring(L, 2);
		size_t length = lua_rawlen(L, 2);

		GoA_WriteString(KH2_BASE_ADDRESS + offset, value, length);
	}

	return 0;
}

int Lua_WriteStringA(lua_State* L)
{
	if (lua_isstring(L, 2))
	{
		u64 address = (u64)lua_tonumber(L, 1);
		const char* value = lua_tostring(L, 2);
		size_t length = lua_rawlen(L, 2);

		GoA_WriteString((u8*)((void*)address), value, length);
	}

	return 0;
}

int Lua_WriteByteArray(lua_State* L)
{
	if (lua_istable(L, 2))
	{
		u64 offset = (u64)lua_tonumber(L, 1);
		size_t length = lua_rawlen(L, 2);
		
		lua_settop(L, 2);
		lua_pushnil(L);
		for (int i = 0; i < length; i++)
		{
			lua_next(L, -2);
			u8 byte = (u8)lua_tonumber(L, -1);
			GoA_Write<u8>(KH2_BASE_ADDRESS + offset + i, byte);
			lua_pop(L, 1);
		}
	}

	return 0;
}

int Lua_WriteByteArrayA(lua_State* L)
{
	if (lua_istable(L, 2))
	{
		u64 address = (u64)lua_tonumber(L, 1);
		size_t length = lua_rawlen(L, 2);

		lua_settop(L, 2);
		lua_pushnil(L);
		for (int i = 0; i < length; i++)
		{
			lua_next(L, -2);
			u8 byte = (u8)lua_tonumber(L, -1);
			GoA_Write<u8>((u8*)((void*)(address + i)), byte);
			lua_pop(L, 1);
		}
	}

	return 0;
}
