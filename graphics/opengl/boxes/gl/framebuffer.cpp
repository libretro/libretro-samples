#include "framebuffer.hpp"

using namespace std;

namespace GL
{
   void Renderbuffer::allocate()
   {
      glBindRenderbuffer(GL_RENDERBUFFER, id);
      glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internal_format, width, height);
      glBindRenderbuffer(GL_RENDERBUFFER, 0);
   }

   void Renderbuffer::init(GLenum internal_format, unsigned width, unsigned height, unsigned samples)
   {
      this->internal_format = internal_format;
      this->width = width;
      this->height = height;
      this->samples = samples;
      if (id)
         allocate();
   }

   void Renderbuffer::reset()
   {
      glGenRenderbuffers(1, &id);
      if (internal_format)
         allocate();
   }

   void Renderbuffer::destroyed()
   {
      glDeleteRenderbuffers(1, &id);
      id = 0;
   }

   GLuint Framebuffer::back_buffer;
   stack<Framebuffer*> Framebuffer::bind_stack;
   Framebuffer* Framebuffer::bound;

   void Framebuffer::reset()
   {
      glGenFramebuffers(1, &id);
      bind_all();
   }

   GLenum Framebuffer::format_to_attachment(GLenum format, unsigned color_index)
   {
      switch (format)
      {
         case GL_DEPTH_COMPONENT16:
         case GL_DEPTH_COMPONENT24:
         case GL_DEPTH_COMPONENT32F:
            return GL_DEPTH_ATTACHMENT;

         case GL_DEPTH24_STENCIL8:
         case GL_DEPTH32F_STENCIL8:
            return GL_DEPTH_STENCIL_ATTACHMENT;

         default:
            return GL_COLOR_ATTACHMENT0 + color_index;
      }
   }

   bool Framebuffer::format_is_color(GLenum format)
   {
      switch (format)
      {
         case GL_DEPTH_COMPONENT16:
         case GL_DEPTH_COMPONENT24:
         case GL_DEPTH_COMPONENT32F:
         case GL_DEPTH24_STENCIL8:
         case GL_DEPTH32F_STENCIL8:
            return false;

         default:
            return true;
      }
   }

   void Framebuffer::push()
   {
      bind_stack.push(bound);
   }

   void Framebuffer::pop()
   {
      auto top = bind_stack.top();
      if (top)
         top->bind();
      else
         unbind();
      bind_stack.pop();
   }

   void Framebuffer::bind_all()
   {
      if (textures.empty() && renderbuffers.empty())
         return;

      push();
      glBindFramebuffer(GL_FRAMEBUFFER, id);
      attachments.clear();

      for (auto& tex : textures)
      {
         auto& desc = tex.tex->get_desc();
         GLenum attachment = format_to_attachment(desc.internal_format, tex.color_index);
         attachments.push_back(attachment);

         switch (desc.type)
         {
            case Texture::Texture1D:
            case Texture::Texture1DArray:
               throw runtime_error("Attaching 1D textures not supported!");

            case Texture::Texture2D:
               glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                     GL_TEXTURE_2D, tex.tex->id, tex.level);
               break;

            case Texture::TextureCube:
               glFramebufferTexture2D(GL_FRAMEBUFFER, attachment,
                     GL_TEXTURE_CUBE_MAP_POSITIVE_X + tex.layer, tex.tex->id, tex.level);
               break;

            case Texture::Texture2DArray:
               glFramebufferTextureLayer(GL_FRAMEBUFFER, attachment,
                     tex.tex->id, tex.level, tex.layer);
               break;

            default:
               throw runtime_error("Attaching invalid texture type!");
         }
      }

      for (auto& buffer : renderbuffers)
      {
         GLenum attachment = format_to_attachment(buffer.buffer->internal_format, buffer.color_index);
         attachments.push_back(attachment);

         glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, buffer.buffer->id);
      }

      glDrawBuffers(color_attachments.size(), color_attachments.data());

      if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
         throw runtime_error("Framebuffer is not complete!");

      pop();
   }

   void Framebuffer::destroyed()
   {
      glDeleteFramebuffers(1, &id);
      id = 0;
   }

   void Framebuffer::set_attachments(const std::vector<TextureAttachment>& textures,
         const std::vector<RenderbufferAttachment>& renderbuffers)
   {
      for (auto& tex : this->textures)
         unregister_dependency(tex.tex);
      for (auto& buffer : this->renderbuffers)
         unregister_dependency(buffer.buffer);

      this->textures = textures;
      this->renderbuffers = renderbuffers;

      color_attachments.clear();
      attachments.clear();

      for (auto& tex : textures)
      {
         if (format_is_color(tex.tex->get_desc().internal_format))
            color_attachments.push_back(GL_COLOR_ATTACHMENT0 + tex.color_index);

         register_dependency(tex.tex);
      }

      for (auto& buffer : renderbuffers)
      {
         if (format_is_color(buffer.buffer->internal_format))
            color_attachments.push_back(GL_COLOR_ATTACHMENT0 + buffer.color_index);

         register_dependency(buffer.buffer);
      }

      sort(begin(color_attachments), end(color_attachments));

      if (id)
      {
         destroyed();
         reset();
         bind_all();
      }
   }

   void Framebuffer::blit(GLuint fb, unsigned width, unsigned height, unsigned mask)
   {
      push();
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fb);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, id);
      glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, mask,
            GL_NEAREST);
      glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
      glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
      pop();
   }

   void Framebuffer::blit(Framebuffer& fb, unsigned width, unsigned height, unsigned mask)
   {
      blit(fb.id, width, height, mask);
   }

   void Framebuffer::unbind()
   {
      glBindFramebuffer(GL_FRAMEBUFFER, back_buffer);
      bound = nullptr;
   }

   void Framebuffer::bind()
   {
      glBindFramebuffer(GL_FRAMEBUFFER, id);
      bound = this;
   }

   void Framebuffer::invalidate()
   {
      push();
      glBindFramebuffer(GL_FRAMEBUFFER, id);
      glInvalidateFramebuffer(GL_FRAMEBUFFER, attachments.size(), attachments.data());
      pop();
   }
}
