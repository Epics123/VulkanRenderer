#pragma once

#include "VertexBuffer.h"
#include <glm/glm.hpp>
#include <vector>

class Mesh
{
	std::vector<Vertex> verticies;
	VertexBuffer vertexBuffer;
};