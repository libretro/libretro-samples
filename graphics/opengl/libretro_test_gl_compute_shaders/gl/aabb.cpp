#include "mesh.hpp"

using namespace std;
using namespace glm;

namespace GL
{
   AABB::AABB(const glm::vec3& minimum, const glm::vec3& maximum)
   {
      base = minimum;
      offset = maximum - minimum;
   }

   vec3 AABB::center() const { return base + vec3(0.5f) * offset; }

   // Assumes we're not using projective geometry ... It gets troublesome to handle flipped-sign W.
   // Transform all corners of the AABB then create a new AABB based on the transformed result.
   AABB AABB::transform(const mat4& mat) const
   {
      AABB aabb;
      vec3 corners[8];
      for (unsigned i = 0; i < 8; i++)
      {
         vec3 c = corner(i);
         vec4 c_trans = mat * vec4(c, 1.0f);
         corners[i] = vec3(c_trans.xyz) / vec3(c_trans.w);
      }

      vec3 minimum = corners[0];
      vec3 maximum = minimum;
      for (auto& v : corners)
      {
         minimum = min(minimum, v);
         maximum = max(maximum, v);
      }

      aabb.base = minimum;
      aabb.offset = maximum - minimum;
      return aabb;
   }

   vec3 AABB::corner(unsigned corner) const
   {
      vec3 b = base;
      if (corner & 4)
         b.z += offset.z;
      if (corner & 2)
         b.y += offset.y;
      if (corner & 1)
         b.x += offset.x;

      return b;
   }

   BoundingSphere::BoundingSphere(const AABB& aabb)
   {
      pos_radius = vec4(aabb.center(), length(aabb.offset * vec3(0.5f)));
   }

   Frustum::Frustum(const mat4& view_proj)
   {
      auto inv_view_proj = inverse(view_proj);

      // Get world-space coordinates for clip-space bounds.
      auto lbn = inv_view_proj * vec4(-1, -1, -1, 1);
      auto ltn = inv_view_proj * vec4(-1,  1, -1, 1);
      auto lbf = inv_view_proj * vec4(-1, -1,  1, 1);
      auto rbn = inv_view_proj * vec4( 1, -1, -1, 1);
      auto rtn = inv_view_proj * vec4( 1,  1, -1, 1);
      auto rbf = inv_view_proj * vec4( 1, -1,  1, 1);
      auto rtf = inv_view_proj * vec4( 1,  1,  1, 1);

      auto lbn_pos = lbn.xyz / lbn.www;
      auto ltn_pos = ltn.xyz / ltn.www;
      auto lbf_pos = lbf.xyz / lbf.www;
      auto rbn_pos = rbn.xyz / rbn.www;
      auto rtn_pos = rtn.xyz / rtn.www;
      auto rbf_pos = rbf.xyz / rbf.www;
      auto rtf_pos = rtf.xyz / rtf.www;

      // Get plane equations for all sides of frustum.
      auto left_normal   = normalize(cross(lbf_pos - lbn_pos, ltn_pos - lbn_pos));
      auto right_normal  = normalize(cross(rtn_pos - rbn_pos, rbf_pos - rbn_pos));
      auto top_normal    = normalize(cross(ltn_pos - rtn_pos, rtf_pos - rtn_pos));
      auto bottom_normal = normalize(cross(rbf_pos - rbn_pos, lbn_pos - rbn_pos));
      auto near_normal   = normalize(cross(ltn_pos - lbn_pos, rbn_pos - lbn_pos));
      auto far_normal    = normalize(cross(rtf_pos - rbf_pos, lbf_pos - rbf_pos));

      planes[0] = vec4(near_normal, -dot(near_normal, lbn_pos)); // Near
      planes[1] = vec4(far_normal, -dot(far_normal, lbf_pos)); // Far
      planes[2] = vec4(left_normal, -dot(left_normal, lbn_pos)); // Left
      planes[3] = vec4(right_normal, -dot(right_normal, rbn_pos)); // Right
      planes[4] = vec4(top_normal, -dot(top_normal, ltn_pos)); // Top
      planes[5] = vec4(bottom_normal, -dot(bottom_normal, lbn_pos)); // Top
   }

   bool Frustum::intersects_with_sphere(const BoundingSphere& sphere) const
   {
      vec4 pos = vec4(sphere.pos_radius.xyz, 1.0f);
      for (auto& plane : planes)
         if (dot(pos, plane) < -sphere.pos_radius.w)
            return false;
      return true;
   }
}

