#include <gl/global.hpp>
#include <gl/buffer.hpp>
#include <gl/shader.hpp>
#include <gl/vertex_array.hpp>
#include <gl/texture.hpp>
#include <gl/mesh.hpp>
#include <gl/aabb.hpp>
#include <gl/framebuffer.hpp>
#include <gl/scene.hpp>
#include <memory>
#include <cstdint>

using namespace std;
using namespace glm;
using namespace GL;
using namespace Util;

class Scene
{
   public:
      void init()
      {
         int base = 48;
         int scale = 8;
         for (int z = -base; z < base; z++)
            for (int y = -base; y < base; y++)
               for (int x = -base; x < base; x++)
               {
                  blocks.push_back(vec4(vec3(x, y, z) * vec3(scale), 1.4143f));
                  blocks.push_back(vec4(0.0f));
               }
         size = 2 * base / 4;

         model.init(GL_ARRAY_BUFFER, blocks.size() * sizeof(vec4), Buffer::None, blocks.data());

         auto mesh = create_mesh_box();
         auto mesh_fine = load_meshes_obj("app/mesh.obj");

         mesh_fine[0].arrays.push_back({3, 4, GL_FLOAT, GL_FALSE, 0, 1, 1, 0});
         render_array[0].setup(mesh_fine[0].arrays, { &vert_fine, &culled_buffer[0] }, &elem_fine);

         mesh.arrays.push_back({3, 4, GL_FLOAT, GL_FALSE, 0, 1, 1, 0});
         render_array[1].setup(mesh.arrays, { &vert, &culled_buffer[1] }, &elem);

         // Point sprites here.
         VertexArray::Array point_array = { Shader::VertexLocation, 4, GL_FLOAT, GL_FALSE };
         render_array[2].setup({point_array}, { &culled_buffer[2] }, nullptr);

         vert_fine.init(GL_ARRAY_BUFFER, mesh_fine[0].vbo, Buffer::None);
         elem_fine.init(GL_ELEMENT_ARRAY_BUFFER, mesh_fine[0].ibo, Buffer::None);
         vert.init(GL_ARRAY_BUFFER, mesh.vbo, Buffer::None);
         elem.init(GL_ELEMENT_ARRAY_BUFFER, mesh.ibo, Buffer::None);

         indices_fine = mesh_fine[0].ibo.size();
         indices = mesh.ibo.size();

         MaterialBuffer material_buf(mesh_fine[0].material);
         material[0].init(GL_UNIFORM_BUFFER, sizeof(material),
               Buffer::None, &material_buf, Shader::Material);

         mesh.material.diffuse = vec3(0.5f, 0.8f, 0.5f);
         mesh.material.ambient = vec3(0.5f, 0.8f, 0.5f);
         material_buf = mesh.material;
         material[1].init(GL_UNIFORM_BUFFER, sizeof(material),
               Buffer::None, &material_buf, Shader::Material);

         material_buf = mesh.material;
         material[2].init(GL_UNIFORM_BUFFER, sizeof(material),
               Buffer::None, &material_buf, Shader::Material);

         if (!mesh.material.diffuse_map.empty())
         {
            use_diffuse = true;
            tex.load_texture({Texture::Texture2D,
                  { mesh.material.diffuse_map },
                  true });
         }
         else
            use_diffuse = false;

         cull_shader.init_compute("app/shaders/boxcull.cs");
         render_shader.reserve_define("DIFFUSE_MAP", 1);
         render_shader.reserve_define("LOD", 1);
         render_shader.init("app/shaders/boxrender.vs", "app/shaders/boxrender.fs");
         render_shader_point.init("app/shaders/boxrender_point.vs", "app/shaders/boxrender_point.fs");

         for (auto& buffer : culled_buffer)
            buffer.init(GL_ARRAY_BUFFER, 16 * 1024 * 1024, Buffer::Copy);
      }

      void render(const mat4& view_proj)
      {
         // Reset indirect draw buffer.
         struct IndirectCommand
         {
            GLuint count;
            GLuint primCount;
            GLuint firstIndex;
            GLuint baseVertex;
            GLuint baseInstance;
         };
         IndirectCommand command[] = {
            { GLuint(indices_fine) }, // primCount is incremented by shaders.
            { GLuint(indices) },
            { 0, 1 }, // Draw point sprites here, so we're using glDrawArraysIndirect.
         };
         indirect.init(GL_DRAW_INDIRECT_BUFFER, sizeof(command), Buffer::Copy, command);

         // Frustum cull instanced cubes (points) and update indirect draw buffer.
         // Compute shader! :D
         cull_shader.use();
         model.bind_indexed(GL_SHADER_STORAGE_BUFFER, 0);
         for (unsigned i = 0; i < 3; i++)
            culled_buffer[i].bind_indexed(GL_SHADER_STORAGE_BUFFER, i + 1);
         indirect.bind_indexed(GL_ATOMIC_COUNTER_BUFFER, 0); // Instance count is written here.
         glDispatchCompute(size, size, size);
         indirect.unbind_indexed(GL_ATOMIC_COUNTER_BUFFER, 0);
         model.unbind_indexed(GL_SHADER_STORAGE_BUFFER, 0);
         for (unsigned i = 0; i < 3; i++)
            culled_buffer[i].unbind_indexed(GL_SHADER_STORAGE_BUFFER, i + 1);

         // GL must wait until previous shader has made updated data visible.
         // We use updated shader storage buffer in next frame, so just barrier it here.
         glMemoryBarrier(GL_COMMAND_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

         // Render instanced data.
         Sampler::bind(0, Sampler::TrilinearClamp);

         render_shader.use();
         if (use_diffuse)
         {
            tex.bind(0);
            render_shader.set_define("DIFFUSE_MAP", 1);
         }
         else
            render_shader.set_define("DIFFUSE_MAP", 0);

         indirect.bind();
         for (unsigned i = 0; i < 2; i++)
         {
            render_shader.set_define("LOD", i);
            render_array[i].bind();
            material[i].bind();

            // glMultiDrawElementsIndirect is possible, but I had issues getting it to work.
            // Only possible if all LOD levels use same shader though ...
            glDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT,
                  reinterpret_cast<void*>(i * uintptr_t(sizeof(IndirectCommand))));
         }

         // Draw farthest blocks as point sprites.
         render_shader_point.use();
         render_array[2].bind();
         material[2].bind();
         glDrawArraysIndirect(GL_POINTS, reinterpret_cast<void*>(2 * uintptr_t(sizeof(IndirectCommand))));

         indirect.unbind();

         if (use_diffuse)
            tex.unbind(0);

         Sampler::unbind(0, Sampler::TrilinearClamp);
         render_shader_point.unbind();
         render_array[2].unbind();
         material[2].unbind();
      }

      Shader cull_shader;
      Shader render_shader;
      Shader render_shader_point;

      Buffer vert, vert_fine;
      Buffer elem, elem_fine;
      Buffer culled_buffer[3];
      VertexArray render_array[3];

      size_t indices_fine;
      size_t indices;

      Buffer model;
      Buffer material[3];
      Buffer indirect;

      Texture tex;
      bool use_diffuse;
      float cache_depth;
      unsigned size;

      vector<vec4> blocks;
};

class BoxesApp : public LibretroGLApplication
{
   public:
      void get_system_info(retro_system_info& info) const override
      {
         info.library_name = "Boxes";
         info.library_version = "v1";
         info.valid_extensions = nullptr;
         info.need_fullpath = false;
         info.block_extract = false;
      }

      void get_system_av_info(retro_system_av_info& info) const override
      {
         info.timing.fps = 60.0;
         info.timing.sample_rate = 0.0;
         info.geometry.base_width = 320;
         info.geometry.base_height = 180;
         info.geometry.max_width = 1920;
         info.geometry.max_height = 1080;
         info.geometry.aspect_ratio = 16.0f / 9.0f;
      }

      string get_application_name() const override
      {
         return "Boxes";
      }

      string get_application_name_short() const override
      {
         return "boxes";
      }

      vector<Resolution> get_resolutions() const override
      {
         vector<Resolution> res;
         res.push_back({320, 180});
         res.push_back({640, 360});
         res.push_back({1280, 720});
         res.push_back({1920, 1080});
         return res;
      }

      void update_global_data()
      {
         float zn = 1.0f;
         float zf = 2000.0f;
         global.proj = perspective(45.0f, float(width) / float(height), zn, zf);
         global.inv_proj = inverse(global.proj);
         global.view = lookAt(player_pos, player_pos + player_look_dir, vec3(0, 1, 0));
         global.view_nt = lookAt(vec3(0.0f), player_look_dir, vec3(0, 1, 0));
         global.inv_view = inverse(global.view);
         global.inv_view_nt = inverse(global.view_nt);

         global.vp = global.proj * global.view;
         global.inv_vp = inverse(global.vp);

         global.camera_pos = vec4(player_pos, 0.0f);
         global.frustum = Frustum(global.vp);

         global_fragment.camera_pos = global.camera_pos;
         global_fragment.light_pos = vec4(500, 2500, -1000, 1);
         global_fragment.light_color = vec4(1.0);
         global_fragment.light_ambient = vec4(0.2);

         // Compute a point size factor for point sprites.
         // Point size can be determined as size * Delta / clip.w where clip.w is depth.
         vec4 clip_delta_x = global.proj * vec4(1, 0, -zf, 1); // Check dCx/dx for far plane.
         vec4 clip_delta_y = global.proj * vec4(0, 1, -zf, 1); // Check dCy/dy for far plane.
         float delta_x = 0.5f * fabs(clip_delta_x.x / clip_delta_x.w) * width * zf;
         float delta_y = 0.5f * fabs(clip_delta_y.y / clip_delta_y.w) * height * zf;

         global.resolution = vec4(width, height, delta_x, delta_y);
         global_fragment.resolution = vec2(width, height);

         GlobalTransforms *buf;
         if (global_buffer.map(buf))
         {
            *buf = global;
            global_buffer.unmap();
         }

         GlobalFragmentData *frag_buf;
         if (global_fragment_buffer.map(frag_buf))
         {
            *frag_buf = global_fragment;
            global_fragment_buffer.unmap();
         }
      }

      void viewport_changed(const Resolution& res) override
      {
         width = res.width;
         height = res.height;

         update_global_data();
      }

      void update_input(float delta, const InputState::Analog& analog, const InputState::Buttons& buttons)
      {
         player_view_deg_y += analog.rx * -120.0f * delta;
         player_view_deg_x += analog.ry * -90.0f * delta;
         player_view_deg_x = clamp(player_view_deg_x, -80.0f, 80.0f);

         mat4 rotate_x = rotate(mat4(1.0), player_view_deg_x, vec3(1, 0, 0));
         mat4 rotate_y = rotate(mat4(1.0), player_view_deg_y, vec3(0, 1, 0));
         mat4 rotate_y_right = rotate(mat4(1.0), player_view_deg_y - 90.0f, vec3(0, 1, 0));

         player_look_dir = vec3(rotate_y * rotate_x * vec4(0, 0, -1, 1));
         vec3 right_walk_dir = vec3(rotate_y_right * vec4(0, 0, -1, 1));

         vec3 mod_speed = buttons.r ? vec3(500.0f) : vec3(250.0f);
         vec3 velocity = player_look_dir * vec3(analog.y * -0.25f) +
            right_walk_dir * vec3(analog.x * 0.25f);

         player_pos += velocity * mod_speed * delta;
         global.delta_time = delta;
         global.camera_vel = vec4(velocity * mod_speed, 0.0f);
         global_fragment.camera_vel = vec4(velocity * mod_speed, 0.0f);
         update_global_data();
      }

      void run(float delta, const InputState& input) override
      {
         auto analog = input.analog;
         if (fabsf(analog.x) < 0.3f)
            analog.x = 0.0f;
         if (fabsf(analog.y) < 0.3f)
            analog.y = 0.0f;
         if (fabsf(analog.rx) < 0.3f)
            analog.rx = 0.0f;
         if (fabsf(analog.ry) < 0.3f)
            analog.ry = 0.0f;
         update_input(delta, analog, input.pressed);

         glViewport(0, 0, width, height);
         glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
         glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

         glEnable(GL_CULL_FACE);
         glEnable(GL_DEPTH_TEST);
         glDepthFunc(GL_LEQUAL);

         global_buffer.bind();
         global_fragment_buffer.bind();

         scene.render(global.vp);

         skybox.tex.bind(0);
         Sampler::bind(0, Sampler::TrilinearClamp);
         skybox.shader.use();
         skybox.arrays.bind();
         glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
         skybox.arrays.unbind();
         skybox.shader.unbind();

         global_buffer.unbind();
         global_fragment_buffer.unbind();
         skybox.tex.unbind(0);
         Sampler::unbind(0, Sampler::TrilinearClamp);
      }

      void get_context_version(unsigned& major, unsigned& minor) const override
      {
         major = 4;
         minor = 3;
      }

      void load() override
      {
         global_buffer.init(GL_UNIFORM_BUFFER, sizeof(global), Buffer::WriteOnly, nullptr, Shader::GlobalVertexData);
         global_fragment_buffer.init(GL_UNIFORM_BUFFER,
               sizeof(global_fragment), Buffer::WriteOnly, nullptr, Shader::GlobalFragmentData);

         player_pos = vec3(0, 0, 500);
         player_look_dir = vec3(0, 0, -1);
         player_view_deg_x = 0.0f;
         player_view_deg_y = 0.0f;

         scene.init();

         skybox.tex.load_texture({Texture::TextureCube, {
                  "app/xpos.png",
                  "app/xneg.png",
                  "app/ypos.png",
                  "app/yneg.png",
                  "app/zpos.png",
                  "app/zneg.png",
               }, true});
         skybox.shader.init("app/shaders/skybox.vs", "app/shaders/skybox.fs");
         vector<int8_t> vertices = { -1, -1, 1, -1, -1, 1, 1, 1 };
         skybox.vertex.init(GL_ARRAY_BUFFER, 8, Buffer::None, vertices.data());
         skybox.arrays.setup({{Shader::VertexLocation, 2, GL_BYTE, GL_FALSE}}, { &skybox.vertex }, nullptr);
      }

   private:
      unsigned width = 0;
      unsigned height = 0;

      float player_view_deg_x = 0.0f;
      float player_view_deg_y = 0.0f;
      vec3 player_pos;
      vec3 player_look_dir{0, 0, -1};

      struct GlobalTransforms
      {
         mat4 vp;
         mat4 view;
         mat4 view_nt;
         mat4 proj;
         mat4 inv_vp;
         mat4 inv_view;
         mat4 inv_view_nt;
         mat4 inv_proj;
         vec4 camera_pos;
         vec4 camera_vel;
         vec4 resolution; // Padded.
         Frustum frustum;
         float delta_time;
      };

      struct GlobalFragmentData
      {
         vec4 camera_pos;
         vec4 camera_vel;
         vec4 light_pos;
         vec4 light_color;
         vec4 light_ambient;
         vec2 resolution;
      };

      GlobalTransforms global;
      GlobalFragmentData global_fragment;
      Buffer global_buffer;
      Buffer global_fragment_buffer;

      Scene scene;

      struct
      {
         Texture tex;
         Shader shader;
         VertexArray arrays;
         Buffer vertex;
      } skybox;
};

unique_ptr<LibretroGLApplication> libretro_gl_application_create()
{
   return Util::make_unique<BoxesApp>();
}

