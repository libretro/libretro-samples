#include "scene.hpp"

using namespace std;
using namespace glm;

namespace GL
{
   void RenderQueue::set_frustum(const Frustum& frustum)
   {
      this->frustum = frustum;
   }

   void RenderQueue::begin()
   {
      draw_list.clear();
   }

   void RenderQueue::end()
   {
      // Frustum culling.
      draw_list.erase(remove_if(std::begin(draw_list), std::end(draw_list), [this](Renderable* draw) -> bool {
               auto aabb = draw->get_aabb();

               BoundingSphere sphere{aabb};
               draw->set_cache_depth(dot(frustum.planes[0], vec4(sphere.pos_radius.xyz, 1.0f)));

               return !frustum.intersects_with_sphere(sphere);
            }),
            std::end(draw_list));

      // Sort based on various criteria.
      sort(std::begin(draw_list), std::end(draw_list),
            [](Renderable* a, Renderable* b) -> bool {
            return a->compare_less(*b);
         });

      //Log::log("%zu draw calls.", draw_list.size());
   }

   const RenderQueue::DrawList& RenderQueue::get_draw_list() const
   {
      return draw_list;
   }

   void RenderQueue::push(Renderable* elem)
   {
      draw_list.push_back(elem);
   }

   void RenderQueue::render()
   {
      for (auto& elem : draw_list)
         elem->render();
   }
}

