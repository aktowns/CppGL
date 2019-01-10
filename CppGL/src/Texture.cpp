#include "Texture.h"
#include "Loader.h"

#include <glad/glad.h>
#include <stb_image.h>

#include <string>
#include <iostream>
#include <filesystem>

using namespace std;

unsigned int textureFromFile(const filesystem::path &filename, bool gamma) {
	unsigned int textureId;
	glGenTextures(1, &textureId);

	int width, height, nrComponents;
	const auto data = stbi_load(filename.string().c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		cerr << "Texture failed to load at path: " << filename << endl;
		stbi_image_free(data);
	}

	return textureId;
}

optional<Texture*> textureFromResource(const Resource& resource, const Console& console)
{
	const auto fn = filesystem::path("resources") / "textures" / resource.path();

	const auto texture = new Texture{};
	texture->id = textureFromFile(fn);
	
	return texture;
}
