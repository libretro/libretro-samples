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

#if INSTANCED
layout(location = MODEL_INSTANCED) in mat4 aModel; // Instanced arrays
#else
layout(binding = MODEL_TRANSFORM) uniform ModelTransform
{
   mat4 transform;
} model;
#endif

out VertexData
{
   vec3 normal;
   vec3 world;
   vec2 tex;
} vout;

void main()
{
#if INSTANCED
   vec4 world = aModel * vec4(aVertex, 1.0);
#else
   vec4 world = model.transform * vec4(aVertex, 1.0);
#endif

   gl_Position = global_vert.vp * world;

#if INSTANCED
   vout.normal = mat3(aModel) * aNormal;
#else
   vout.normal = mat3(model.transform) * aNormal;
#endif

   vout.world = world.xyz;
   vout.tex = vec2(aTexCoord.x, 1.0 - aTexCoord.y);
}

