#include "vertex_array.hpp"

using namespace std;

namespace GL
{
   void VertexArray::setup()
   {
      bind();

      if (elem_buffer)
         elem_buffer->bind();

      for (auto& array : arrays)
      {
         vertex_buffers[array.buffer_index]->bind();
         glEnableVertexAttribArray(array.location);
         glVertexAttribPointer(array.location, array.size, array.type,
               array.normalized, array.stride, reinterpret_cast<const void*>(array.offset));
         glVertexAttribDivisor(array.location, array.divisor);
         vertex_buffers[array.buffer_index]->unbind();
      }

      unbind();

      if (elem_buffer)
         elem_buffer->unbind();
   }

   void VertexArray::bind()
   {
      glBindVertexArray(vao);
   }

   void VertexArray::unbind()
   {
      glBindVertexArray(0);
   }

   void VertexArray::setup(const vector<Array>& arrays, std::vector<Buffer*> vertex_buffers, Buffer *elem_buffer)
   {
      this->arrays = arrays;
      for (auto vert : this->vertex_buffers)
         unregister_dependency(vert);
      if (this->elem_buffer)
         unregister_dependency(this->elem_buffer);

      this->vertex_buffers = move(vertex_buffers);
      this->elem_buffer = elem_buffer;

      for (auto vert : this->vertex_buffers)
         register_dependency(vert);
      if (this->elem_buffer)
         register_dependency(this->elem_buffer);

      if (alive)
      {
         destroyed();
         reset();
         if (!arrays.empty())
            setup();
      }
   }

   void VertexArray::reset()
   {
      alive = true;
      glGenVertexArrays(1, &vao);

      if (!arrays.empty())
         setup();
   }

   void VertexArray::destroyed()
   {
      glDeleteVertexArrays(1, &vao);
      vao = 0;
   }
}

