#pragma once

#define SCR_WIDTH 1440
#define SCR_HEIGHT 900

#define LARGE_FONT   "arial.ttf?size=64"
#define DEFAULT_FONT "arial.ttf?size=18"
#define GAME_FONT    "neuropolmod.ttf?size=18"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

#ifdef RESOURCES_DIR
#define RESOURCES std::filesystem::path(TOSTRING(RESOURCES_DIR))
#else
#define RESOURCES std::filesystem::path("resources")
#endif

#define SHADERS_DIR  (RESOURCES / "shaders")
#define MODELS_DIR   (RESOURCES / "models")
#define FONTS_DIR    (RESOURCES / "fonts")
#define TEXTURES_DIR (RESOURCES / "textures")
