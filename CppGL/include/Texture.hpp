#pragma once

#include "Loader.hpp"
#include "Logger.hpp"

#include <filesystem>

struct Texture final
{
	unsigned int id;
};

unsigned int textureFromFile(const std::filesystem::path &filename, bool gamma = false);

std::optional<Texture*> textureFromResource(const Resource& resource, const Console& console);
