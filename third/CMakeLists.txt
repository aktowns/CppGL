add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
	-DGLEW_STATIC
	-D_CRT_SECURE_NO_WARNINGS
)

### GLFW ###
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL " " FORCE)
set(GLFW_BUILD_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_INSTALL OFF CACHE BOOL " " FORCE)
set(GLFW_USE_CHDIR OFF CACHE BOOL " " FORCE)

add_subdirectory (glfw)

include_directories(
	glfw/include/GLFW/
)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	-lGL -lGLU -lXrandr -lXext -lX11 -lrt
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
set(OPENGL_LIBRARY
	${OPENGL_LIBRARY}
	${CMAKE_DL_LIBS}
	${GLFW_LIBRARIES}
)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

set_target_properties(glfw PROPERTIES EXCLUDE_FROM_ALL 1 EXCLUDE_FROM_DEFAULT_BUILD 1)

### GLAD ###
set(GLAD_SRC "glad/src/glad.c")
set(GLAD_INCLUDE "glad/include/glad/glad.h")

add_library( GLAD STATIC
	${GLAD_SRC}
	${GLAD_INCLUDE}
)

target_link_libraries(GLAD
	${OPENGL_LIBRARY}
	${EXTRA_LIBS}
)

target_include_directories(GLAD PUBLIC "glad/include")


### ASSIMP ###
include(CheckIncludeFile)
include(CheckTypeSize)
include(CheckFunctionExists)
set(BUILD_STATIC_LIB ON)
set(ASSIMP_BUILD_SAMPLES OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE BOOL "" FORCE)
add_subdirectory(assimp)

### FreeType ### 
add_subdirectory(freetype2)

### GLM ###
set(GLM_TEST_ENABLE  OFF CACHE BOOL "" FORCE)
add_subdirectory(glm)

### Bullet ###
set(BUILD_UNIT_TESTS  OFF CACHE BOOL "" FORCE)
set(BUILD_BULLET2_DEMOS  OFF CACHE BOOL "" FORCE)
set(BUILD_EXTRAS  OFF CACHE BOOL "" FORCE)
set(BUILD_OPENGL3_DEMOS  OFF CACHE BOOL "" FORCE)
add_subdirectory(bullet3)


### fmt ###
add_subdirectory(fmt)