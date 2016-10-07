in VertexData
{
   vec3 color;
} fin;

out vec4 FragColor;

void main()
{
   FragColor = vec4(fin.color, 1.0);
}

