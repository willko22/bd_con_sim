// Fragment shader source code
const char* fragmentShaderSource = R"(
#version 460 core
in vec4 vertexColor;
out vec4 FragColor;

void main()
{
    FragColor = vertexColor;
}
)";
