#shader vertex
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 position, vec2 texCoords>

out vec2 TexCoords;

uniform mat4 model;
uniform mat4 projection;

void main()
{
    TexCoords = vertex.zw;
    gl_Position = projection * model * vec4(vertex.xy, 0.0, 1.0);
}

#shader fragment
#version 330 core

in vec2 TexCoord;
out vec4 FragColor;

uniform vec4 TileColor;


void main()
{
	//FragColor = texture(ourTexture, TexCoord) * vec4(fragColor, 1.0);
	FragColor = TileColor;
};