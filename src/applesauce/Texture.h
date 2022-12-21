#pragma once

#include "GLResource.h"

namespace applesauce
{
    class Texture2D : public GLResource
    {
    public:
        enum class CompareMode
        {
            none,
            compareRefToTexture
        };

        enum class CompareFunc
        {
            lessThanOrEqual,
            greaterThanOrEqual,
            less,
            greater,
            equal,
            notEqual,
            always,
            never
        };

        enum class Format
        {
            depthComponent,
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

        enum class Wrap
        {
            clampToEdge,
            clampToBorder,
            mirroredRepeat,
            repeat,
            mirrorClampToEdge
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

        static GLenum glWrap(const Wrap wrap)
        {
            switch (wrap)
            {
            case Wrap::clampToBorder:
                return GL_CLAMP_TO_BORDER;
            case Wrap::clampToEdge:
                return GL_CLAMP_TO_EDGE;
            case Wrap::mirrorClampToEdge:
                return GL_MIRROR_CLAMP_TO_EDGE;
            case Wrap::mirroredRepeat:
                return GL_MIRRORED_REPEAT;
            case Wrap::repeat:
                return GL_REPEAT;
            default:
                return 0;
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
            case Format::depthComponent:
                return GL_DEPTH_COMPONENT;
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

        void bind() const
        {
            glBindTexture(target, glId());
        }

        void unbind() const
        {
            glBindTexture(target, 0);
        }

        void setCompareFunc(const CompareFunc func)
        {
            GLenum glFunc = 0;
            switch (func)
            {
            case CompareFunc::lessThanOrEqual:
                glFunc = GL_LEQUAL;
                break;
            case CompareFunc::greaterThanOrEqual:
                glFunc = GL_GEQUAL;
                break;
            case CompareFunc::less:
                glFunc = GL_LESS;
                break;
            case CompareFunc::greater:
                glFunc = GL_GREATER;
                break;
            case CompareFunc::equal:
                glFunc = GL_EQUAL;
                break;
            case CompareFunc::notEqual:
                glFunc = GL_NOTEQUAL;
                break;
            case CompareFunc::always:
                glFunc = GL_ALWAYS;
                break;
            case CompareFunc::never:
                glFunc = GL_NEVER;
                break;
            }
            bind();
            glTexParameteri(target, GL_TEXTURE_COMPARE_FUNC, glFunc);
            unbind();
        }

        void setCompareMode(const CompareMode mode)
        {
            GLenum glMode = GL_NONE;
            if (mode == CompareMode::compareRefToTexture)
            {
                glMode = GL_COMPARE_REF_TO_TEXTURE;
            }
            bind();
            glTexParameteri(target, GL_TEXTURE_COMPARE_MODE, glMode);
            unbind();
        }

        void setImage(int width, int height, Format format, void *data) const
        {
            bind();
            glTexImage2D(target,
                         0, // mipmap level
                         glInternalFormat(internalFormat),
                         static_cast<GLsizei>(width),
                         static_cast<GLsizei>(height),
                         0, // border always 0
                         glInternalFormat(format),
                         GL_UNSIGNED_BYTE,
                         data);
            unbind();
        }

        void setMinFilter(const Filter filter)
        {
            bind();
            glTexParameteri(target, GL_TEXTURE_MIN_FILTER, glFilter(filter));
            unbind();
        }

        void setMagFilter(const Filter filter)
        {
            if (minFilterOnly(filter))
            {
                return;
            }
            bind();
            glTexParameteri(target, GL_TEXTURE_MAG_FILTER, glFilter(filter));
            unbind();
        }

        void setWrapping(const Wrap wrapS, const Wrap wrapT)
        {
            bind();
            glTexParameteri(target, GL_TEXTURE_WRAP_S, glWrap(wrapS));
            glTexParameteri(target, GL_TEXTURE_WRAP_T, glWrap(wrapT));
            unbind();
        }

        void generateMipmaps()
        {
            bind();
            glGenerateMipmap(target);
            unbind();
        }

    private:
        Format internalFormat;
    };

    class DepthTexture2D : public Texture2D
    {
    public:
        DepthTexture2D(int width, int height) : Texture2D(Texture2D::Format::depthComponent)
        {
            setImage(width, height, Texture2D::Format::depthComponent, nullptr);
            setCompareMode(Texture2D::CompareMode::compareRefToTexture);
            setCompareFunc(Texture2D::CompareFunc::lessThanOrEqual);
        }
    };

    using Texture = Texture2D;

}
