#include "Texture.h"

#include <png.h>

#include <memory>

namespace applesauce {

    std::shared_ptr<Texture> singleColorTexture(uint32_t color)
    {
        auto tex = std::make_shared<Texture>();
        tex->setMinFilter(Texture::Filter::nearest);
        tex->setMagFilter(Texture::Filter::nearest);

        tex->setImage(1, 1, Texture::Format::rgba, &color);
        return tex;
    };

    std::shared_ptr<Texture> textureFromPNG(const char* filename)
    {
        png_image image;
        /* Only the image structure version number needs to be set. */
        std::memset(&image, 0, sizeof image);
        image.version = PNG_IMAGE_VERSION;

        if (png_image_begin_read_from_file(&image, filename))
        {
            image.format = PNG_FORMAT_RGBA;
            auto buffer = reinterpret_cast<png_bytep>(malloc(PNG_IMAGE_SIZE(image)));

            if (png_image_finish_read(&image, NULL /*background*/, buffer, 0 /*row_stride*/,
                NULL /*colormap for PNG_FORMAT_FLAG_COLORMAP */))
            {
                auto tex = std::make_shared<Texture>();
                tex->setMinFilter(Texture::Filter::linearMipMapLinear);
                tex->setMagFilter(Texture::Filter::linear);

                auto ptr = reinterpret_cast<GLubyte*>(buffer);
                tex->setImage(image.width, image.height, Texture::Format::rgba, ptr);
                tex->generateMipmaps();

                return tex;
            }
        }
        return nullptr;
    }

}