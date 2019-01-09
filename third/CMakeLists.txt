# Temp set vars for subdirs
function(TSET KEY VALUE TYPE)
message("Temporarily setting ${KEY} to ${VALUE} as ${TYPE}")
set(${KEY}_SAVED ${${KEY}} CACHE ${TYPE} "" FORCE)
set(${KEY} ${VALUE} CACHE ${TYPE} "" FORCE)
endfunction(TSET)

function(RSET KEY TYPE)
message("Unsetting ${KEY} as ${TYPE} BACK TO ${${KEY}_SAVED}")
set(${KEY} ${${KEY}_SAVED} CACHE ${TYPE} "" FORCE)
endfunction(RSET)

# Make everything static?
set(BUILD_STATIC_LIB ON)

add_definitions(/wd5045 /wd4464 /wd4365 /wd5038 /wd5039 /wd4571 /wd4668 /wd4626 /wd4625
 /wd5026 /wd5027 /wd4191) # Disable spectre warning

### PhysX ###
if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
tset(TARGET_BUILD_PLATFORM "windows" STRING)
endif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
tset(CMAKE_PREFIX_PATH "${PHYSX_ROOT_DIR}/../externals/CMakeModules;${PHYSX_ROOT_DIR}/../externals/targa" STRING)
#tset(CMAKE_INSTALL_PREFIX "${PHYSX_ROOT_DIR}/install/vc15win64/PhysX" STRING)

set(PX_OUTPUT_ARCH "x86" CACHE STRING "" FORCE)
set(PHYSX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Physx/physx" CACHE STRING "" FORCE)
set(PX_OUTPUT_LIB_DIR "${CMAKE_INSTALL_PREFIX}" CACHE STRING "" FORCE)
set(PX_OUTPUT_BIN_DIR "${CMAKE_INSTALL_PREFIX}" CACHE STRING "" FORCE)
#set(PX_OUTPUT_LIB_DIR "${PHYSX_ROOT_DIR}" CACHE STRING "" FORCE)
#set(PX_OUTPUT_BIN_DIR "${PHYSX_ROOT_DIR}" CACHE STRING "" FORCE)
set(PX_BUILDSNIPPETS OFF CACHE BOOL "" FORCE)
set(PX_BUILDPUBLICSAMPLES OFF CACHE BOOL "" FORCE)
set(PX_GENERATE_STATIC_LIBRARIES OFF CACHE BOOL "" FORCE)
#set(PX_GENERATE_GPU_STATIC_LIBRARIES ON CACHE BOOL "" FORCE)
set(NV_USE_STATIC_WINCRT ON CACHE BOOL "" FORCE)
set(NV_USE_DEBUG_WINCRT ON CACHE BOOL "" FORCE)
set(PX_FLOAT_POINT_PRECISE_MATH OFF CACHE BOOL "" FORCE)

set(ENV{PM_VSWHERE_PATH} "${PHYSX_ROOT_DIR}/../externals/VsWhere" CACHE STRING "" FORCE)
set(ENV{PM_TARGA_PATH} "${PHYSX_ROOT_DIR}/../externals/targa" CACHE STRING "" FORCE)
set(ENV{PHYSX_ROOT_DIR} "${PHYSX_ROOT_DIR}" CACHE STRING "" FORCE)
set(ENV{PM_CMAKEMODULES_PATH} "${PHYSX_ROOT_DIR}/../externals/CMakeModules" CACHE STRING "" FORCE)
set(ENV{PM_PXSHARED_PATH} "${PHYSX_ROOT_DIR}/../pxshared" CACHE STRING "" FORCE)

add_subdirectory(PhysX/physx/compiler/public)

rset(TARGET_BUILD_PLATFORM STRING)
rset(CMAKE_PREFIX_PATH STRING)
#rset(CMAKE_INSTALL_PREFIX STRING)

add_definitions(
	-DTW_STATIC
	-DTW_NO_LIB_PRAGMA
	-DTW_NO_DIRECT3D
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
#set(BUILD_UNIT_TESTS  OFF CACHE BOOL "" FORCE)
#set(BUILD_EXTRAS  OFF CACHE BOOL "" FORCE)
#set(BUILD_BULLET2_DEMOS  OFF CACHE BOOL "" FORCE)
#set(BUILD_OPENGL3_DEMOS  OFF CACHE BOOL "" FORCE)
#add_subdirectory(bullet3)

### fmt ###
add_subdirectory(fmt)


