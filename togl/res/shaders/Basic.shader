#shader vertex
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 inTexCoord;

out vec3 fragColor;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform mat4 transform;

void main()
{
	//gl_Position = transform * vec4(aPos, 1.0);
	gl_Position = projection * view * model * vec4(aPos, 1.0);
	fragColor = inColor;
	TexCoord = inTexCoord;
};

#shader fragment
#version 330 core
in vec3 fragColor;
in vec2 TexCoord;

out vec4 FragColor;

uniform sampler2D ourTexture;

void main()
{
	FragColor = texture(ourTexture, TexCoord) * vec4(fragColor, 1.0);
	//FragColor = vec4(fragColor, 1.0);
};