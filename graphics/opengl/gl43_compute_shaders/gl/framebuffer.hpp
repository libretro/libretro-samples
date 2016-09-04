#ifndef FRAMEBUFFER_HPP__
#define FRAMEBUFFER_HPP__

#include "global.hpp"
#include "texture.hpp"
#include <vector>
#include <stack>

namespace GL
{
   class Framebuffer;
   class Renderbuffer : public ContextListener, public ContextResource
   {
      public:
         friend class Framebuffer;
         Renderbuffer() { ContextListener::init(); }
         ~Renderbuffer() { deinit(); }

         void init(GLenum internal_format, unsigned width, unsigned height, unsigned samples = 0);

         void reset() override;
         void destroyed() override;

      private:
         GLuint id = 0;
         GLenum internal_format = 0;
         unsigned width = 0;
         unsigned height = 0;
         unsigned samples = 0;

         void allocate();
   };

   class Framebuffer : public ContextListener, public ContextResource
   {
      public:
         Framebuffer() { ContextListener::init(); }
         ~Framebuffer() { deinit(); }

         void reset() override;
         void destroyed() override;

         struct RenderbufferAttachment
         {
            Renderbuffer* buffer;
            unsigned color_index;
         };

         struct TextureAttachment
         {
            Texture* tex;
            unsigned color_index;
            unsigned layer;
            unsigned level;
         };

         void set_attachments(const std::vector<TextureAttachment>& textures,
               const std::vector<RenderbufferAttachment>& renderbuffers);

         void blit(Framebuffer& fb, unsigned width, unsigned height,
               unsigned mask);
         void blit(GLuint fb, unsigned width, unsigned height,
               unsigned mask);

         void bind();

         static void set_back_buffer(GLuint fbo) { back_buffer = fbo; }
         static void set_back_buffer(Framebuffer& fb) { back_buffer = fb.id; }

         static void unbind();

         void invalidate();

         static void push();
         static void pop();

      private:
         GLuint id = 0;
         static GLuint back_buffer;
         static std::stack<Framebuffer*> bind_stack;
         static Framebuffer* bound;

         std::vector<TextureAttachment> textures;
         std::vector<RenderbufferAttachment> renderbuffers;
         std::vector<GLenum> color_attachments;
         std::vector<GLenum> attachments;

         void bind_all();

         static GLenum format_to_attachment(GLenum format, unsigned color_index);
         static bool format_is_color(GLenum format);
   };
}

#endif

