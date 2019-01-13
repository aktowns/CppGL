#pragma once

#include <glm/glm.hpp>
#include <vector>

void generateIcosphereMesh(size_t lod, std::vector<uint32_t>& indices, std::vector<glm::vec3>& positions);
