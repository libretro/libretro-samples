layout(binding = GLOBAL_VERTEX_DATA) uniform GlobalVertexData
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
   vec4 resolution;
} global_vert;

layout(binding = GLOBAL_FRAGMENT_DATA) uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 camera_vel;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
   vec2 resolution;
} global_frag;

layout(binding = MATERIAL) uniform Material
{
   vec4 ambient;
   vec4 diffuse;
   vec4 specular;
   float specular_power;
} material;

layout(location = VERTEX) in vec4 aVertex;

out VertexData
{
   vec3 color;
} vout;

void main()
{
   vec4 world = vec4(aVertex.xyz, 1.0);

   vec4 clip_pos = global_vert.vp * world;
   gl_Position = clip_pos;
   vec2 point_size = 2.2 * global_vert.resolution.zw / clip_pos.w; // Make it slightly bigger than box (2.0 width) to accomodate for rotation.
   gl_PointSize = dot(vec2(0.5), point_size); // Average

   // Vertex coloring.
   vec3 vEye = normalize(global_frag.camera_pos.xyz - world.xyz);
   vec3 vLight = normalize(global_frag.light_pos.xyz - world.xyz);

   float ndotl = max(dot(vLight, vEye), 0.0);
   float light_mod = 0.5; // Could attenuate, but don't bother.
   vec3 ambient = global_frag.light_ambient.rgb * material.ambient.rgb;
   vec3 diffuse = light_mod * ndotl * global_frag.light_color.rgb * material.diffuse.rgb;
   vout.color = sqrt(ambient + diffuse);
}

