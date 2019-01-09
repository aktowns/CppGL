# CMakeList.txt : CMake project for CppGL, include source and define
# project specific logic here.
#
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
	#${THIRD}/bullet3/src/
	${THIRD}/fmt/include/
	${THIRD}/PhysX/physx/include/
	${THIRD}/PhysX/pxshared/include/
	include/
)

set(HEADERS 
	"include/Shader.h" 
	"include/Camera.h"
	"include/Mesh.h"
	"include/Model.h"
	"include/Font.h"
	"include/Texture.h"
	"include/Game.h"
	"include/Config.h"
	"include/Loader.h"
	"include/utils/Resource.h"
	"include/utils/FmtExtensions.h"
	#"include/utils/DebugDrawer.h"
)

set(SRCS
	"src/Main.cpp"
	"src/Shader.cpp"
	"src/Camera.cpp"
	"src/Mesh.cpp"
	"src/Model.cpp"
	"src/Font.cpp"
	"src/Texture.cpp"
	"src/Game.cpp"
	"src/utils/Resource.cpp"
	"src_boilerplate/stb_image.cpp"
	"src/PhysxSetup.cpp"
)

# Add source to this project's executable.
add_executable (CppGL ${SRCS} ${HEADERS})
target_link_libraries(CppGL ${ALL_LIBS})
set_property(TARGET CppGL PROPERTY CXX_STANDARD 17)
set_property(TARGET CppGL PROPERTY CXX_STANDARD_REQUIRED ON)

### RESOURCES ###

set(SHADER_DIR "resources/shaders")
set(TEXTURE_DIR "resources/textures")
set(MODEL_DIR "resources/models")
set(FONT_DIR "resources/fonts")

function(copy TYPE FROM TO)
	message("copying " ${TYPE} " from " ${FROM} " to " ${TO})	
	configure_file(${FROM} ${TO} COPYONLY)
endfunction()

function(copy_resource)
	set(options MODEL SHADER TEXTURE FONT)
	set(oneValueArgs SOURCE)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(copy_resource "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})	

	list(APPEND copy_resource_SOURCES "${copy_resource_SOURCE}")
	foreach(resource ${copy_resource_SOURCES})
		if(copy_resource_TEXTURE)
			copy("texture" "${TEXTURE_DIR}/${resource}" "${CMAKE_CURRENT_BINARY_DIR}/${TEXTURE_DIR}/${resource}")
		elseif(copy_resource_SHADER)
			copy("shader" "${SHADER_DIR}/${resource}" "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_DIR}/${resource}")
		elseif(copy_resource_FONT)
			copy("font" "${FONT_DIR}/${resource}" "${CMAKE_CURRENT_BINARY_DIR}/${FONT_DIR}/${resource}")
		elseif(copy_resource_MODEL)
			file(GLOB model_files "${MODEL_DIR}/${resource}/*")
			foreach(file ${model_files})
				set(TYPE "model")
				get_filename_component(filename ${file} NAME)
				copy("model" "${MODEL_DIR}/${resource}/${filename}" 
					"${CMAKE_CURRENT_BINARY_DIR}/${MODEL_DIR}/${resource}/${filename}")
			endforeach()
		else()
			message (FATAL_ERROR "unhandled copy")
		endif()

	endforeach()
endfunction()

copy_resource(SHADER SOURCES "shader.frag" "shader.vert" "model.frag" "model.vert" "text.frag" "text.vert" 
	"multiple_lights.frag" "multiple_lights.vert" "debugline.frag" "debugline.vert" "lamp.frag" "lamp.vert"
	"skybox.frag" "skybox.vert")
copy_resource(TEXTURE SOURCES "awesomeface.png" "container.png" "container2.png" "container2_specular.png")
copy_resource(TEXTURE SOURCES "skybox/back.png" "skybox/bottom.png" "skybox/front.png" 
	"skybox/left.png" "skybox/right.png" "skybox/top.png")
copy_resource(MODEL SOURCES "nanosuit")
copy_resource(MODEL SOURCES "link")
copy_resource(FONT SOURCES "arial.ttf")