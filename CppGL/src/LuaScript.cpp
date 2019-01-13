#include "LuaScript.hpp"

#include <string>

using namespace std;

LuaScript::LuaScript(const string& filename) : Logger("lua")
{
	_state = luaL_newstate();
	if (luaL_loadfile(_state, filename.c_str()) || lua_pcall(_state, 0, 0, 0))
	{
		console->error("failed to load lua script: {}", filename);
		_state = nullptr;
	}
}

LuaScript::~LuaScript()
{
	if (_state) lua_close(_state);
}
