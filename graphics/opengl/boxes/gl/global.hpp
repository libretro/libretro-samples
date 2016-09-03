#ifndef GLOBAL_HPP__
#define GLOBAL_HPP__

#include <vector>
#include <memory>
#include <utility>
#include <stdexcept>
#include "../libretro/libretro.h"
#include "../glsym/glsym.h"

#include "util.hpp"

#define GLM_SWIZZLE
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

namespace GL
{
   class LibretroGLApplication
   {
      public:
         struct Resolution
         {
            unsigned width;
            unsigned height;
         };

         struct InputState
         {
            struct Analog
            {
               float x, y, rx, ry;
            };

            Analog analog;

            struct Buttons
            {
               bool left, right, up, down;
               bool a, b, x, y;
               bool l, r;
            };

            Buttons pressed;
            Buttons triggered;
         };

         virtual void get_context_version(unsigned& major, unsigned& minor) const = 0;
         virtual void get_system_info(retro_system_info& info) const = 0;
         virtual void get_system_av_info(retro_system_av_info& info) const = 0;
         virtual std::string get_application_name() const = 0;
         virtual std::string get_application_name_short() const = 0;
         virtual std::vector<Resolution> get_resolutions() const = 0;

         virtual void load() {}
         virtual void unload() {}
         virtual void viewport_changed(const Resolution& res) = 0;
         virtual void run(float delta, const InputState& input) = 0;
   };

   class ContextListener
   {
      public:
         virtual ~ContextListener() {}

         virtual void reset() = 0;
         virtual void destroyed() = 0;

         bool operator==(const ContextListener& other) const { return this == &other; }
         bool operator!=(const ContextListener& other) const { return this != &other; }

         void set_dead_manager() { dead_manager = true; }

      protected:
         void register_dependency(ContextListener *listener);
         void unregister_dependency(ContextListener *listener);
         void init();
         void deinit();

      private:
         bool dead_manager = false;
   };

   class ContextManager
   {
      public:
         static ContextManager& get();
         ~ContextManager();

         void register_listener(ContextListener *listener);
         void unregister_listener(const ContextListener *listener);
         void notify_reset();
         void notify_destroyed();

         void register_dependency(ContextListener *master, ContextListener *slave);
         void unregister_dependency(ContextListener *master, ContextListener *slave);

         void set_dir(const std::string& dir) { libretro_dir = dir; }
         inline std::string path(const std::string& p)
         {
            if (libretro_dir.empty())
               throw std::logic_error("Loading assets too early!\n");

            return Path::join(libretro_dir, p);
         }

      private:
         ContextManager() {}

         struct ListenerState
         {
            ContextListener *listener = nullptr;
            bool signaled = false;
            std::vector<std::weak_ptr<ListenerState>> dependencies;
            std::vector<std::weak_ptr<ListenerState>> dependers;
            uint64_t id = 0;

            void reset_chain();
            void destroy_chain();

            bool operator==(const ListenerState& other) const { return id == other.id; }
            bool operator!=(const ListenerState& other) const { return id != other.id; }
            bool operator==(const ContextListener& other) const { return listener == &other; }
            bool operator!=(const ContextListener& other) const { return listener != &other; }
         };

         std::vector<std::shared_ptr<ListenerState>> listeners;
         bool alive = false;

         uint64_t context_id = 0;

         std::string libretro_dir;
   };

   inline std::string asset_path(const std::string& path)
   {
      return ContextManager::get().path(path);
   }

   // Non-copyable, non-movable stubs.
   class ContextResource
   {
      public:
         ContextResource() {}
         ContextResource(const ContextResource&) = delete;
         ContextResource& operator=(const ContextResource&) = delete;
         ContextResource(ContextResource&&) = delete;
         ContextResource& operator=(ContextResource&&) = delete;
   };
}

std::unique_ptr<GL::LibretroGLApplication> libretro_gl_application_create(); // Entry point for core.

#endif

