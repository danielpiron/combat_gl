/*
#pragma once

#include "VertexArray.h"

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

// Experimental Mesh structure
struct SubMesh
{
    std::shared_ptr<applesauce::VertexArray> array;
    std::vector<std::shared_ptr<applesauce::Buffer>> buffers;
    std::shared_ptr<applesauce::Buffer> indexBuffer;
    int indexBufferByteOffset;
    int elementCount;
};

struct Mesh
{
    std::list<SubMesh> submeshes;
};

std::unordered_map<std::string, Mesh> loadMeshes(const char *filename);
*/