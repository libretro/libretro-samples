#ifndef AABB_HPP__
#define AABB_HPP__

#include "global.hpp"
#include <cstdint>
#include <vector>
#include <string>

namespace GL
{
   struct AABB
   {
      AABB() = default;
      AABB(const glm::vec3& minimum, const glm::vec3& maximum);
      glm::vec3 base;
      glm::vec3 offset;

      glm::vec3 center() const;
      AABB transform(const glm::mat4& mat) const;
      glm::vec3 corner(unsigned corner) const;
   };

   struct BoundingSphere
   {
      BoundingSphere() = default;
      explicit BoundingSphere(const AABB& aabb);

      glm::vec4 pos_radius;
   };

   struct Frustum
   {
      Frustum() = default;
      Frustum(const glm::mat4& view_proj);
      glm::vec4 planes[6];

      // Used for frustum culling.
      bool intersects_with_sphere(const BoundingSphere& sphere) const;
   };
}

#endif

