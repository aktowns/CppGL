#pragma once

#include "Logger.hpp"

#include <string>
#include <optional>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class LuaScript : Logger
{
    lua_State* _state;
public:
    LuaScript(const std::string& filename);
    ~LuaScript();

    template<typename T>
    std::optional<T> get(const std::string& var)
    {
        if (!_state)
        {
            console->error("failed to load {}: script not loaded", var);
            return;
        }

        T result;
        return result;
    }

    template<typename T>
    T luaGet(const std::string& var);
};