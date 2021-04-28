#include <Windows.h>
#include <vector>
#include <string>
#include <iostream>
#include <filesystem>

extern "C"
{
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include "GoA_Defines.h"
#include "GoA_Memory.h"

struct BackendConfig
{
	const char* scriptsDirectory;
	bool keybindsEnabled;
};

u8* KH2_BASE_ADDRESS;

lua_State* GoA_InitializeScript(const char* path)
{
	lua_State* L = luaL_newstate();

	lua_pushinteger(L, (u64)KH2_BASE_ADDRESS);
	lua_setglobal(L, "BaseAddress");

	luaL_openlibs(L);

	// TODO(skettios): add functions
	lua_register(L, "ReadByte", Lua_Read<u8>);
	lua_register(L, "ReadShort", Lua_Read<u16>);
	lua_register(L, "ReadInt", Lua_Read<u32>);
	lua_register(L, "ReadLong", Lua_Read<u64>);
	lua_register(L, "ReadFloat", Lua_Read<f32>);

	lua_register(L, "WriteByte", Lua_Write<u8>);
	lua_register(L, "WriteShort", Lua_Write<u16>);
	lua_register(L, "WriteInt", Lua_Write<u32>);
	lua_register(L, "WriteLong", Lua_Write<u64>);
	lua_register(L, "WriteFloat", Lua_Write<f32>);

	luaL_dofile(L, path);

	return L;
}

extern "C"
{
	void _declspec(dllexport) GoA_Run(BackendConfig* config)
	{
		KH2_BASE_ADDRESS = (u8*)GetModuleHandle(NULL);

		std::vector<lua_State*> scripts;
		for (auto& entry : std::filesystem::directory_iterator(config->scriptsDirectory))
		{
			if (entry.path().extension() == ".lua")
			{
				lua_State* L = GoA_InitializeScript(entry.path().u8string().c_str());
				scripts.push_back(L);
			}
		}
	}
}
