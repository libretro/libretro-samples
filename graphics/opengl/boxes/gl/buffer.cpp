#include "buffer.hpp"

namespace GL
{
   void Buffer::init(GLenum target, GLsizei size, GLuint flags, const void *initial_data, GLuint index)
   {
      this->target = target;
      this->size = size;
      this->flags = flags;
      this->index = index;

      if (alive)
         init_buffer(initial_data);

      if (initial_data)
      {
         // Keep the data for later.
         temp.resize(size);
         std::memcpy(temp.data(), initial_data, size);
      }
      else
         temp.clear();
   }

   void Buffer::reset()
   {
      alive = true;
      glGenBuffers(1, &id);

      if (size && target)
         init_buffer(temp.empty() ? nullptr : temp.data());
   }

   void Buffer::destroyed()
   {
      alive = false;
      if (id)
         glDeleteBuffers(1, &id);
      id = 0;
   }

   void Buffer::unmap()
   {
      glBindBuffer(target, id);
      glUnmapBuffer(target);
      glBindBuffer(target, 0);
   }

   bool Buffer::is_indexed(GLenum type)
   {
      switch (type)
      {
         case GL_UNIFORM_BUFFER:
         case GL_ATOMIC_COUNTER_BUFFER:
         case GL_SHADER_STORAGE_BUFFER:
            return true;
         default:
            return false;
      }
   }

   void Buffer::bind_indexed(GLenum target, unsigned index)
   {
      glBindBufferBase(target, index, id);
   }

   void Buffer::unbind_indexed(GLenum target, unsigned index)
   {
      glBindBufferBase(target, index, 0);
   }

   void Buffer::bind(GLenum target)
   {
      glBindBuffer(target, id);
   }

   void Buffer::unbind(GLenum target)
   {
      glBindBuffer(target, 0);
   }

   void Buffer::bind()
   {
      if (is_indexed(target))
         glBindBufferBase(target, index, id);
      else
         glBindBuffer(target, id);
   }

   void Buffer::unbind()
   {
      if (is_indexed(target))
         glBindBufferBase(target, index, 0);
      else
         glBindBuffer(target, 0);
   }

   GLenum Buffer::gl_usage_from_flags(GLuint flags)
   {
      switch (flags)
      {
         case WriteOnly: return GL_DYNAMIC_DRAW;
         case ReadOnly: return GL_DYNAMIC_READ;
         case Copy: return GL_DYNAMIC_COPY;
         default: return GL_STATIC_DRAW;
      }
   }

   void Buffer::init_buffer(const void *initial_data)
   {
      glBindBuffer(target, id);
      glBufferData(target, size, initial_data, gl_usage_from_flags(flags));
      glBindBuffer(target, 0);
   }
}

