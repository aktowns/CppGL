cmake_minimum_required (VERSION 3.8)

include_directories(
	${THIRD}/glfw/include/
	${THIRD}/glm/
	${THIRD}/glad/include/
	${THIRD}/assimp/include/
	${CMAKE_BINARY_DIR}/third/assimp/include/
	${THIRD}/stb/
	${THIRD}/freetype2/include/
	${CMAKE_BINARY_DIR}/third/freetype2/include/
	${THIRD}/spdlog/include/
	${THIRD}/nuklear/
	${THIRD}/fmt/include/
	${THIRD}/PhysX/physx/include/
	${THIRD}/PhysX/pxshared/include/
	${THIRD}/lua/src
    ${THIRD}/entt/src/
	include/
)

set(HEADERS 
    "include/utils/Conversions.hpp"
	"include/utils/DebugDrawer.hpp"
	"include/utils/FmtExtensions.hpp"
	"include/utils/Resource.hpp"
	"include/Camera.hpp"
	"include/Config.hpp"
	"include/Font.hpp"
	"include/Game.hpp"
    "include/GameUI.hpp"
	"include/Icosphere.hpp"
	"include/Loader.hpp"
    "include/Logger.hpp"
	"include/LuaScript.hpp"
	"include/Mesh.hpp"
	"include/Model.hpp"
    "include/PhysXSetup.hpp"
    "include/Renderable.hpp"
	"include/Shader.hpp" 
    "include/Skybox.hpp"
	"include/Texture.hpp"
)

set(SRCS
	"src/utils/Resource.cpp"
	"src/Camera.cpp"
	"src/Font.cpp"
	"src/Game.cpp"
    "src/GameUI.cpp"
	"src/Icosphere.cpp"
	"src/LuaScript.cpp"
	"src/Main.cpp"
	"src/Mesh.cpp"
	"src/Model.cpp"
	"src/PhysXSetup.cpp"
	"src/Shader.cpp"
    "src/Skybox.cpp"
	"src/Texture.cpp"
	"src_boilerplate/stb_image.cpp"
)


# Add source to this project's executable.
#add_definitions(-D_DEBUG)
string(TOLOWER ${CMAKE_BUILD_TYPE} LOWERCASE_CMAKE_BUILD_TYPE)
if(LOWERCASE_CMAKE_BUILD_TYPE STREQUAL "debug") 
set(RESOURCES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/resources")
add_definitions(-DRESOURCES_DIR=${RESOURCES_DIR})
else()
add_subdirectory(resources)
endif()

add_executable (CppGL ${SRCS} ${HEADERS})
target_link_libraries(CppGL ${ALL_LIBS})
set_property(TARGET CppGL PROPERTY CXX_STANDARD 17)
set_property(TARGET CppGL PROPERTY CXX_STANDARD_REQUIRED ON)

