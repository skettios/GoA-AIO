#define _CRT_SECURE_NO_WARNINGS

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

#include "CRC.h"

struct BackendConfig
{
	const char* scriptsDirectory;
	const char* processExecutablePath;
	bool keybindsEnabled;
	bool running;
};

u8* KH2_BASE_ADDRESS;
u32 CRC;

const u32 TRESURE_OFFSETS[592] = {
	0x00, // 0
	0x00, 0x2A6E5FA, 0x2A6E612, 0x00, 0x2A6E67E, 0x00, 0x2A6E5BE, 0x2A6E5CA, 0x2A6E636, 0x2A6E642,	//10
	0x2A6E5E2, 0x2A6E41A, 0x00, 0x00, 0x00, 0x2A6E462, 0x2A6E46E, 0x2A6E47A, 0x00, 0x00,		// 20
	0x2A6E1FE, 0x2A6E216, 0x2A6E222, 0x2A6E25E, 0x2A6E276, 0x2A6E28E, 0x2A6E29A, 0x2A6E2D6, 0x2A6E2E2, 0x2A6E2EE,	// 30
	0x2A6E32A, 0x2A6E336, 0x2A6E342, 0x2A6E3F6, 0x2A6E396, 0x2A6E3A2, 0x2A6E3D2, 0x00, 0x2A6E6A2, 0x2A6E6AE,	// 40
	0x2A6E6F6, 0x2A6E77A, 0x2A6E74A, 0x2A6E756, 0x2A6E76E, 0x2A6E6C6, 0x00, 0x00, 0x2A6E7E6, 0x2A6E7F2,	// 50
	0x2A6E82E, 0x00, 0x2A6E846, 0x2A6E882, 0x2A6E8A6, 0x2A6E8B2, 0x2A6E8D6, 0x2A6E8E2, 0x00, 0x00,		// 60
	0x00, 0x00, 0x2A6E6DE, 0x2A6E7AA, 0x2A6E7B6, 0x00, 0x00, 0x00, 0x00, 0x2A6E8EE,	// 70
	0x2A6E912, 0x2A6E91E, 0x2A6E92A, 0x2A6E942, 0x2A6E972, 0x2A6E9A2, 0x2A6E9C6, 0x2A6E9D2, 0x2A6E40E, 0x00,		// 80
	0x2A6E426, 0x2A6E432, 0x2A6E43E, 0x2A6E44A, 0x2A6E456, 0x00, 0x00, 0x00, 0x00, 0x00,		// 90
	0x2A6E4B6, 0x2A6E486, 0x2A6E492, 0x2A6E5A6, 0x00, 0x00, 0x2A6E4CE, 0x2A6E4DA, 0x00, 0x2A6E516,	// 100
};

lua_State* GoA_InitializeScript(const char* path)
{
	lua_State* L = luaL_newstate();

	lua_pushnumber(L, (u64)KH2_BASE_ADDRESS);
	lua_setglobal(L, "BaseAddress");

	lua_pushnumber(L, (u32)CRC);
	lua_setglobal(L, "GAME_ID");

	lua_pushstring(L, "BACKEND");
	lua_setglobal(L, "ENGINE_TYPE");

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

	lua_register(L, "WriteArray", Lua_WriteByteArray);
	lua_register(L, "WriteArrayA", Lua_WriteByteArrayA);

	luaL_dofile(L, path);

	return L;
}

inline void GoA_CallLuaFunction(lua_State* L, const char* functionName)
{
	lua_getglobal(L, functionName);
	lua_call(L, 0, 0);
	lua_pop(L, 0);
}

struct RandomizerConfig
{
	u16 treasures[592];
};

extern "C"
{
	void _declspec(dllexport) GoA_Run(BackendConfig* config)
	{
#if 0
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
#endif

		KH2_BASE_ADDRESS = (u8*)GetModuleHandle(NULL);
		KH2_BASE_ADDRESS += 0x56450E; //NOTE(skettios): this is just to maintain compatibility with LuaBackend scripts

		CRC = CRC::Calculate(config->processExecutablePath, strlen(config->processExecutablePath), CRC::CRC_32());

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

	void _declspec(dllexport) GoA_Randomize(u16* treasures)
	{
		u8* REAL_BASE_ADDRESS = (u8*)GetModuleHandle(NULL);

		for (u32 i = 0; i < 100; i++)
		{
			if (TRESURE_OFFSETS[i] != 0x00)
			{
				std::cout << std::hex << TRESURE_OFFSETS[i] << ":" << treasures[i] << std::endl;
				GoA_Write<u16>(REAL_BASE_ADDRESS + TRESURE_OFFSETS[i], treasures[i]);
			}
		}
	}
}
