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
	lua_register(L, "ReadByteA", Lua_ReadA<u8>);
	lua_register(L, "ReadShortA", Lua_ReadA<u16>);
	lua_register(L, "ReadIntA", Lua_ReadA<u32>);
	lua_register(L, "ReadLongA", Lua_ReadA<u64>);
	lua_register(L, "ReadFloatA", Lua_ReadA<f32>);

	lua_register(L, "WriteByte", Lua_Write<u8>);
	lua_register(L, "WriteShort", Lua_Write<u16>);
	lua_register(L, "WriteInt", Lua_Write<u32>);
	lua_register(L, "WriteLong", Lua_Write<u64>);
	lua_register(L, "WriteFloat", Lua_Write<f32>);
	lua_register(L, "WriteByteA", Lua_WriteA<u8>);
	lua_register(L, "WriteShortA", Lua_WriteA<u16>);
	lua_register(L, "WriteIntA", Lua_WriteA<u32>);
	lua_register(L, "WriteLongA", Lua_WriteA<u64>);
	lua_register(L, "WriteFloatA", Lua_WriteA<f32>);

	lua_register(L, "WriteString", Lua_WriteString);
	lua_register(L, "WriteStringA", Lua_WriteStringA);

	luaL_dofile(L, path);

	return L;
}

inline void GoA_CallLuaFunction(lua_State* L, const char* functionName)
{
	lua_getglobal(L, functionName);
	lua_call(L, 0, 0);
	lua_pop(L, 0);
}

struct BackendConfig
{
	const char* scriptsDirectory;
	const char* processExecutablePath;
	bool keybindsEnabled;
	bool running;
};

extern "C"
{
	void _declspec(dllexport) GoA_Run(BackendConfig* config)
	{
		KH2_BASE_ADDRESS = (u8*)GetModuleHandle(NULL);
		KH2_BASE_ADDRESS += 0x56450E; //NOTE(skettios): this is just to maintain compatibility with LuaBackend scripts

		std::vector<lua_State*> scripts;
		for (auto& entry : std::filesystem::directory_iterator(config->scriptsDirectory))
		{
			if (entry.path().extension() == ".lua")
			{
				lua_State* L = GoA_InitializeScript(entry.path().u8string().c_str());
				scripts.push_back(L);

				GoA_CallLuaFunction(L, "_OnInit");
			}
		}

		while (config->running)
		{
			if (config->keybindsEnabled)
			{
				//TODO(skettios): add keybinds
			}

			for (auto L : scripts)
				GoA_CallLuaFunction(L, "_OnFrame");

			Sleep(1);
		}

		for (auto L : scripts)
			lua_close(L);
	}
}
