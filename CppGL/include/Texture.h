#pragma once

#include "Loader.h"
#include "Logger.h"

#include <filesystem>

struct Texture final
{
	unsigned int id;
};

unsigned int textureFromFile(const std::filesystem::path &filename, bool gamma = false);

std::optional<Texture*> textureFromResource(const Resource& resource, const Console& console);
