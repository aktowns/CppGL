# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("CppGL")

# set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")

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
	BulletDynamics
	BulletCollision
	LinearMath
	fmt-header-only
)

# Include sub-projects.
add_subdirectory ("CppGL")
