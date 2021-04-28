#include "GoA_Memory.h"

#include <string.h>

void GoA_WriteString(u8* address, const char* value, size_t length)
{
	if (length == 0)
		length = strlen(value);

	for (size_t i = 0; i < length; i++)
		GoA_Write<u8>(address, value[0]);
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
