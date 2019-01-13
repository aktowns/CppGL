project (Lua C)
cmake_minimum_required (VERSION 3.8)

include_directories(src/include)
# for linux..
#add_definitions(
#	-Wextra -Wshadow -Wsign-compare -Wundef -Wwrite-strings -Wredundant-decls 
#	-Wdisabled-optimization -Wdouble-promotion -std=c99 
#)

set(lua_sources
		src/lapi.c
		src/lcode.c
		src/lctype.c
		src/ldebug.c
		src/ldo.c
		src/ldump.c
		src/lfunc.c
		src/lgc.c
		src/llex.c
		src/lmem.c
		src/lobject.c
		src/lopcodes.c
		src/lparser.c
		src/lstate.c
		src/lstring.c
		src/ltable.c
		src/ltm.c
		src/lundump.c
		src/lvm.c
		src/lzio.c
		src/ltests.c
)
set(lua_lib
		src/lbaselib.c
		src/ldblib.c
		src/liolib.c
		src/lmathlib.c
		src/loslib.c
		src/ltablib.c
		src/lstrlib.c 
		src/lutf8lib.c
		src/lbitlib.c
		src/loadlib.c
		src/lcorolib.c
		src/linit.c
)

add_library(lua ${lua_sources} ${lua_lib} "src/lauxlib.c")
