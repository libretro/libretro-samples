#ifndef VERTEX_ARRAY_HPP__
#define VERTEX_ARRAY_HPP__

#include "global.hpp"
#include "buffer.hpp"

namespace GL
{
   class VertexArray : public ContextListener
   {
      public:
         VertexArray() { init(); }
         ~VertexArray() { deinit(); }
         struct Array
         {
            GLuint location;
            GLint size;
            GLenum type;
            GLboolean normalized;

            GLuint stride;
            unsigned buffer_index;
            unsigned divisor;
            GLsizei offset;
         };

         void reset() override;
         void destroyed() override;

         void setup(const std::vector<Array>& arrays, std::vector<Buffer*> vertex_buffers, Buffer *elem_buffer);

         void bind();
         void unbind();

      private:
         GLuint vao = 0;
         bool alive = false;
         std::vector<Array> arrays;
         std::vector<Buffer*> vertex_buffers;
         Buffer* elem_buffer = nullptr;
         void setup();
   };
}

#endif

