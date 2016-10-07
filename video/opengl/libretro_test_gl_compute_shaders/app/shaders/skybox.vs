layout(binding = 0) uniform GlobalVertexData
{
   mat4 vp;
   mat4 view;
   mat4 view_nt;
   mat4 proj;
   mat4 inv_vp;
   mat4 inv_view;
   mat4 inv_view_nt;
   mat4 inv_proj;
};

layout(location = VERTEX) in vec2 aVertex;
out vec3 vDirection;

void main()
{
   gl_Position = vec4(aVertex, 1.0, 1.0);

   vec4 world_space = inv_view_nt * inv_proj * vec4(aVertex, 1.0, 1.0);
   vDirection = world_space.xyz / world_space.w;
}

