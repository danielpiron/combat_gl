#pragma once

#include "GLResource.h"

namespace applesauce
{
    class Texture2D : public GLResource
    {
        static GLuint genGLTexture()
        {
            GLuint result;
            glGenTextures(1, &result);
            return result;
        }

        static constexpr GLenum target = GL_TEXTURE_2D;

    public:
        Texture2D() : GLResource(genGLTexture()) {}
        ~Texture2D()
        {
            auto id = glId();
            glDeleteTextures(1, &id);
        }

        void bind() const
        {
            glBindTexture(target, glId());
        }

        void unbind() const
        {
            glBindTexture(target, 0);
        }
    };
}