#pragma once

#include <glad/glad.h>

class GLResource
{
public:
    explicit GLResource(const GLuint resourceId) : id(resourceId) {}
    GLuint glId() const { return id; }

protected:
    GLuint id;
};