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
} global_vert;

layout(location = VERTEX) in vec3 aVertex;
layout(location = NORMAL) in vec3 aNormal;
layout(location = TEXCOORD) in vec2 aTexCoord;
layout(location = 3) in vec4 aPos;

out VertexData
{
   vec3 normal;
   vec3 world;
   vec2 tex;
} vout;

void main()
{
   vec4 world = vec4(aVertex + aPos.xyz, 1.0);

   gl_Position = global_vert.vp * world;

   vout.normal = aNormal;

   vout.world = world.xyz;
   vout.tex = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

