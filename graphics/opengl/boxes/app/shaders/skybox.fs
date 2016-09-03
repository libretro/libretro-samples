layout(binding = GLOBAL_FRAGMENT_DATA) uniform GlobalFragmentData
{
   vec4 camera_pos;
   vec4 camera_vel;
   vec4 light_pos;
   vec4 light_color;
   vec4 light_ambient;
};

layout(binding = 0) uniform samplerCube skybox;

in vec3 vDirection;
out vec4 FragColor;

void main()
{
   vec4 col = texture(skybox, vDirection); 
   FragColor = col;
}
