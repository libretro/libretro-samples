#ifndef SCENE_HPP__
#define SCENE_HPP__

#include "global.hpp"
#include "mesh.hpp"
#include "aabb.hpp"
#include <functional>
#include <cstdint>
#include <vector>
#include <utility>
#include <algorithm>

namespace GL
{
   struct Renderable
   {
      virtual void set_cache_depth(float depth) = 0;
      virtual const AABB& get_aabb() const = 0;
      virtual bool compare_less(const Renderable& other) const = 0;
      virtual void render() = 0;
   };

   class RenderQueue
   {
      public:
         using DrawList = std::vector<Renderable*>;

         void set_frustum(const Frustum& frustum);
         void begin();
         void end();
         void push(Renderable* elem);
         void render();
         const DrawList& get_draw_list() const;

      private:
         DrawList draw_list;
         Frustum frustum;
   };
}

#endif

