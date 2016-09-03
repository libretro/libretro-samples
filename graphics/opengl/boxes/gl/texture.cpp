#include "texture.hpp"
#include <rpng/rpng.h>
#include <math.h>
#include <utility>

using namespace std;

namespace GL
{
   StaticSampler Sampler::samplers[Sampler::NumSamplers] = {
      { PointWrap }, { PointClamp }, { BilinearWrap }, { BilinearClamp }, { TrilinearWrap },
      { TrilinearClamp }, { ShadowLinear }, { ShadowPoint },
   };

   Sampler& Sampler::get(Type type)
   {
      return samplers[static_cast<unsigned>(type)];
   }

   void Sampler::bind(unsigned unit, Type type)
   {
      get(type).bind(unit);
   }

   void Sampler::unbind(unsigned unit, Type type)
   {
      get(type).unbind(unit);
   }

   GLenum Sampler::type_to_wrap(Type type)
   {
      switch (type)
      {
         case PointClamp:
         case BilinearClamp:
         case TrilinearClamp:
         case ShadowLinear:
         case ShadowPoint:
            return GL_CLAMP_TO_EDGE;

         case PointWrap:
         case BilinearWrap:
         case TrilinearWrap:
            return GL_CLAMP_TO_EDGE;

         default:
            return 0;
      }
   }

   GLenum Sampler::type_to_compare(Type type)
   {
      switch (type)
      {
         case ShadowLinear:
         case ShadowPoint:
            return GL_COMPARE_REF_TO_TEXTURE;

         default:
            return GL_NONE;
      }
   }

   GLenum Sampler::type_to_mag(Type type)
   {
      switch (type)
      {
         case BilinearWrap:
         case BilinearClamp:
         case TrilinearWrap:
         case TrilinearClamp:
         case ShadowLinear:
            return GL_LINEAR;

         default:
            return GL_NEAREST;
      }
   }

   GLenum Sampler::type_to_min(Type type)
   {
      switch (type)
      {
         case BilinearWrap:
         case BilinearClamp:
         case ShadowLinear:
            return GL_LINEAR;

         case TrilinearWrap:
         case TrilinearClamp:
            return GL_LINEAR_MIPMAP_LINEAR;

         default:
            return GL_NEAREST;
      }
   }

   void Sampler::init(Type type)
   {
      this->type = type;
      if (id)
      {
         destroyed();
         reset();
      }
   }

   void Sampler::bind(unsigned unit)
   {
      glBindSampler(unit, id);
   }

   void Sampler::unbind(unsigned unit)
   {
      glBindSampler(unit, 0);
   }

   void Sampler::reset()
   {
      glGenSamplers(1, &id);
      glSamplerParameteri(id, GL_TEXTURE_WRAP_S, type_to_wrap(type));
      glSamplerParameteri(id, GL_TEXTURE_WRAP_T, type_to_wrap(type));
      glSamplerParameteri(id, GL_TEXTURE_WRAP_R, type_to_wrap(type));
      glSamplerParameteri(id, GL_TEXTURE_COMPARE_MODE, type_to_compare(type));
      glSamplerParameteri(id, GL_TEXTURE_MIN_FILTER, type_to_min(type));
      glSamplerParameteri(id, GL_TEXTURE_MAG_FILTER, type_to_mag(type));
   }

   void Sampler::destroyed()
   {
      if (id)
         glDeleteSamplers(1, &id);
      id = 0;
   }

   void Texture::bind(unsigned unit)
   {
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(texture_type, id);
      glActiveTexture(GL_TEXTURE0);
   }

   void Texture::unbind(unsigned unit)
   {
      glActiveTexture(GL_TEXTURE0 + unit);
      glBindTexture(texture_type, 0);
      glActiveTexture(GL_TEXTURE0);
   }

   void Texture::bind_image(unsigned unit, StorageAccess access, unsigned level, unsigned layer)
   {
      GLenum acc;
      switch (access)
      {
         case WriteOnly: acc = GL_WRITE_ONLY; break;
         case ReadOnly: acc = GL_READ_ONLY; break;
         case ReadWrite: acc = GL_READ_WRITE; break;
         default: acc = 0; break;
      }
      glBindImageTexture(unit, id, level,
            GL_FALSE, layer, acc, desc.internal_format);
   }

   void Texture::unbind_image(unsigned unit)
   {
      glBindImageTexture(unit, 0, 0,
            GL_FALSE, 0, GL_READ_ONLY, GL_R8);
   }

   void Texture::reset()
   {
      glGenTextures(1, &id);
      if (desc.type != TextureNone)
         setup();
   }

   void Texture::destroyed()
   {
      if (id)
         glDeleteTextures(1, &id);
      id = 0;
   }

   void Texture::init(const Desc& desc_tex)
   {
      desc = desc_tex;
      if (!desc.levels)
         desc.levels = size_to_miplevels(desc.width, desc.height);
      texture_type = type_to_gl(desc.type);
      res = {};

      if (id)
      {
         glDeleteTextures(1, &id);
         glGenTextures(1, &id);
         setup();
      }
   }

   static inline void swizzle(uint8_t *out, const uint32_t *in, unsigned pixels)
   {
      for (unsigned i = 0; i < pixels; i++, out += 4, in++)
      {
         uint32_t col = *in;
         out[0] = uint8_t(col >> 16);
         out[1] = uint8_t(col >>  8);
         out[2] = uint8_t(col >>  0);
         out[3] = uint8_t(col >> 24);
      }
   }

   void Texture::load_texture_data()
   {
      data.clear();

      switch (res.type)
      {
         case Texture1D:
         case Texture1DArray:
            throw std::logic_error("Uploading 1D or 1DArray textures not supported!");

         case TextureCube:
            if (res.paths.size() != 6)
               throw std::logic_error("Cube map must have 6 textures!");
            break;

         case Texture2D:
            if (res.paths.size() != 1)
               throw std::logic_error("Texture2D must have 1 entry!");

         case Texture2DArray:
            if (res.paths.empty())
               throw std::logic_error("Texture2DArray must have at least 1 entry!");
            break;

         default:
            throw std::logic_error("Invalid texture format!");
      }

      unsigned index = 0;
      for (auto& path : res.paths)
      {
         uint32_t *raw_data = nullptr;
         unsigned width = 0;
         unsigned height = 0;

         auto apath = asset_path(path);
         if (!rpng_load_image_argb(apath.c_str(), &raw_data, &width, &height))
            throw std::runtime_error(String::cat("Failed to load texture: ", path.c_str()));

         if ((desc.width && width != desc.width) || (desc.height && height != desc.height))
            throw std::logic_error("Textures are not all of same size!");

         desc.width = width;
         desc.height = height;

         std::vector<uint8_t> byte_data;
         byte_data.resize(width * height * sizeof(uint32_t));
         swizzle(byte_data.data(), raw_data, width * height);

         data.push_back({move(byte_data), width, height});
         free(raw_data);
         index++;
      }

      if (res.type == Texture2DArray)
         desc.array_size = res.paths.size();

      if (res.generate_mipmaps)
         desc.levels = size_to_miplevels(desc.width, desc.height);
   }

   void Texture::upload_texture_data()
   {
      switch (desc.type)
      {
         case Texture2D:
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, data[0].width, data[0].height, GL_RGBA, GL_UNSIGNED_BYTE, data[0].data.data());
            break;

         case Texture2DArray:
         {
            unsigned i = 0;
            for (auto& slice : data)
            {
               glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, i, slice.width, slice.height, 1, GL_RGBA, GL_UNSIGNED_BYTE, slice.data.data());
               i++;
            }
            break;
         }

         case TextureCube:
         {
            for (unsigned i = 0; i < 6; i++)
               glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, data[i].width, data[i].height, GL_RGBA, GL_UNSIGNED_BYTE, data[i].data.data());
            break;
         }

         default:
            throw std::logic_error("Trying to upload null texture.");
      }

      data.clear();
      if (res.generate_mipmaps)
         glGenerateMipmap(texture_type);
   }

   void Texture::load_texture(const Resource& res)
   {
      this->res = res;

      desc.type            = res.type;
      desc.levels          = 1;
      desc.internal_format = GL_RGBA8;
      desc.width           = 0; // Deferred
      desc.height          = 0; // Deferred
      desc.array_size      = 0;

      texture_type = type_to_gl(res.type);

      if (id)
      {
         glDeleteTextures(1, &id);
         glGenTextures(1, &id);
         setup();
      }
   }

   unsigned Texture::size_to_miplevels(unsigned width, unsigned height)
   {
      return unsigned(floor(log2(max(width, height))) + 1);
   }

   void Texture::setup()
   {
      bind(0);

      if (!res.paths.empty())
         load_texture_data();

      switch (desc.type)
      {
         case Texture1D:
            glTexStorage1D(texture_type,
                  desc.levels, desc.internal_format, desc.width);
            break;

         case Texture1DArray:
            glTexStorage2D(texture_type,
                  desc.levels, desc.internal_format, desc.width, desc.array_size);
            break;

         case Texture2D:
         case TextureCube:
            glTexStorage2D(texture_type,
                  desc.levels, desc.internal_format, desc.width, desc.height);
            break;

         case Texture2DArray:
            glTexStorage3D(texture_type,
                  desc.levels, desc.internal_format, desc.width, desc.height, desc.array_size);
            break;

         default:
            break;
      }

      if (!res.paths.empty())
         upload_texture_data();

      unbind(0);
   }

   GLenum Texture::type_to_gl(Type type)
   {
      switch (type)
      {
         case Texture1D:
            return GL_TEXTURE_1D;
         case Texture1DArray:
            return GL_TEXTURE_1D_ARRAY;
         case Texture2D:
            return GL_TEXTURE_2D;
         case Texture2DArray:
            return GL_TEXTURE_2D_ARRAY;
         case TextureCube:
            return GL_TEXTURE_CUBE_MAP;
         default:
            return GL_NONE;
      }
   }
}

