#ifndef SHADER_HPP__
#define SHADER_HPP__

#include "global.hpp"
#include <vector>
#include <map>

namespace GL
{
   class Shader : public ContextListener, public ContextResource
   {
      public:
         Shader() { ContextListener::init(); }
         ~Shader() { deinit(); }

         enum AttribLocations
         {
            VertexLocation = 0,
            TexCoordLocation = 1,
            NormalLocation = 2,
            ModelInstancedCol0 = 3,
            ModelInstancedCol1 = 4,
            ModelInstancedCol2 = 5,
            ModelInstancedCol3 = 6,
         };

         enum UniformLocation
         {
            GlobalVertexData = 0,
            GlobalFragmentData = 1,

            ModelTransform = 2,
            Material = 3
         };

         struct Sampler
         {
            std::string name;
            unsigned unit;
         };

         struct UniformBuffer
         {
            std::string name;
            unsigned index;
         };

         void init(const std::string& path_vs, const std::string& path_fs, const std::string& path_gs = "");
         void init_compute(const std::string& path_compute);

         void use();
         void unbind();

         void reset() override;
         void destroyed() override;

         void reserve_define(const std::string& name, unsigned bits);
         void set_define(const std::string& name, unsigned value);

         static void reserve_global_define(const std::string& name, unsigned bits);
         void set_global_define(const std::string& name, unsigned value);

      private:
         std::map<unsigned, GLuint> progs;
         unsigned current_permutation = 0;

         unsigned total_bits = 0;
         struct Define
         {
            unsigned start_bit;
            unsigned bits;
            unsigned value;
            std::string name;
         };
         std::vector<Define> defines;

         static unsigned total_global_bits;
         static std::vector<Define> global_defines;

         std::string source_vs, source_fs, source_gs, source_compute;
         bool alive = false;

         unsigned compile_shaders();
         void compile_shader(GLuint obj, const std::string& source,
               const std::vector<std::string>& defines);
         void log_shader(GLuint obj, const std::vector<const GLchar*>& source);
         void log_program(GLuint obj);

         std::vector<std::string> current_defines() const;
         unsigned compute_permutation() const;

         bool active = false;
   };
}

#endif

