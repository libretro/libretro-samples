#ifndef MESH_HPP__
#define MESH_HPP__

#include "global.hpp"
#include "vertex_array.hpp"
#include "aabb.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace GL
{
   struct Material
   {
      glm::vec3 ambient = glm::vec3(0.2f);
      glm::vec3 diffuse = glm::vec3(0.8f);
      glm::vec3 specular = glm::vec3(0.0f);
      float specular_power = 0.0f;

      std::string diffuse_map;
   };

   struct MaterialBuffer
   {
      glm::vec4 ambient;
      glm::vec4 diffuse;
      glm::vec4 specular;
      float specular_power;

      MaterialBuffer& operator=(const Material& mat)
      {
         ambient = glm::vec4(mat.ambient.x, mat.ambient.y, mat.ambient.z, 0.0f);
         diffuse = glm::vec4(mat.diffuse.x, mat.diffuse.y, mat.diffuse.z, 0.0f);
         specular = glm::vec4(mat.specular.x, mat.specular.y, mat.specular.z, 0.0f);
         specular_power = mat.specular_power;
         return *this;
      }

      explicit MaterialBuffer(const Material& mat)
      {
         *this = mat;
      }
   };

   struct Mesh
   {
      std::vector<float> vbo;
      std::vector<GLuint> ibo;
      std::vector<VertexArray::Array> arrays;
      AABB aabb;
      bool has_vertex = false;
      bool has_normal = false;
      bool has_texcoord = false;

      Material material;

      void finalize();
   };

   std::vector<Mesh> load_meshes_obj(const std::string& path);
   Mesh create_mesh_box();
}

#endif

