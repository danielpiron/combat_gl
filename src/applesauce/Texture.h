#pragma once

#include "GLResource.h"

namespace applesauce
{
    class Texture2D : public GLResource
    {
    public:
        enum class Format
        {
            rgba
        };

        enum class Filter
        {
            nearest,
            linear,
            nearestMipMapNearest,
            nearestMipMapLinear,
            linearMipMapNearest,
            linearMipMapLinear,
        };

    private:
        static bool minFilterOnly(const Filter filter)
        {
            switch (filter)
            {
            case Filter::nearestMipMapNearest:
            case Filter::nearestMipMapLinear:
            case Filter::linearMipMapNearest:
            case Filter::linearMipMapLinear:
                return true;
            default:
                return false;
            }
        }

        static GLenum glFilter(const Filter filter)
        {
            switch (filter)
            {
            case Filter::nearest:
                return GL_NEAREST;
            case Filter::linear:
                return GL_LINEAR;
            case Filter::nearestMipMapNearest:
                return GL_NEAREST_MIPMAP_NEAREST;
            case Filter::nearestMipMapLinear:
                return GL_NEAREST_MIPMAP_LINEAR;
            case Filter::linearMipMapNearest:
                return GL_LINEAR_MIPMAP_NEAREST;
            case Filter::linearMipMapLinear:
                return GL_LINEAR_MIPMAP_LINEAR;
            default:
                return 0;
            }
        }

        static GLenum glInternalFormat(const Format format)
        {
            switch (format)
            {
            case Format::rgba:
                return GL_RGBA;
            default:
                return 0;
            }
        }
        static GLuint genGLTexture()
        {
            GLuint result;
            glGenTextures(1, &result);
            return result;
        }

        static constexpr GLenum target = GL_TEXTURE_2D;

    public:
        Texture2D(Format format = Format::rgba) : GLResource(genGLTexture()), internalFormat(format) {}

        ~Texture2D()
        {
            auto id = glId();
            glDeleteTextures(1, &id);
        }

        void setImage(int width, int height, Format, void *data) const
        {
            bind();
            glTexImage2D(target,
                         0, // mipmap level
                         glInternalFormat(internalFormat),
                         static_cast<GLsizei>(width),
                         static_cast<GLsizei>(height),
                         0, // border always 0
                         GL_RGBA,
                         GL_UNSIGNED_BYTE,
                         data);
            unbind();
        }

        void setMinFilter(const Filter filter)
        {
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, glFilter(filter));
            unbind();
        }

        void setMagFilter(const Filter filter)
        {
            if (minFilterOnly(filter))
            {
                return;
            }
            bind();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, glFilter(filter));
            unbind();
        }

        void bind() const
        {
            glBindTexture(target, glId());
        }

        void unbind() const
        {
            glBindTexture(target, 0);
        }

    private:
        Format internalFormat;
    };
}