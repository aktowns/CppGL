# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("CppGL")

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

# Force static linking
set(CompilerFlags
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
)

foreach(CompilerFlag ${CompilerFlags})
  string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
endforeach()

find_package(OpenGL REQUIRED)

# Compile external dependencies 
add_subdirectory (third)

set(THIRD "${CMAKE_SOURCE_DIR}/third/")

set(ALL_LIBS
	${OPENGL_LIBRARY}
	glfw
	GLAD
	assimp
	freetype
	fmt-header-only
	PhysX
	PhysXExtensions
	PhysXCharacterKinematic
	PhysXCooking
	PhysXFoundation
	PhysXVehicle
	PhysXPvdSDK
	lua
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(ALL_LIBS ${ALL_LIBS} stdc++fs)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

# Include sub-projects.
add_subdirectory ("CppGL")
