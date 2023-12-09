
struct objectData
{
	glm::vec2 position;
	glm::vec2 dimension;

	glm::vec4 color;
};

// Create Vao //
// Player Vao
unsigned int Vao;
glGenVertexArrays(1, &Vao);
unsigned int Vbo;
glBindVertexArray(Vao);

glGenBuffers(1, &Vbo);
glBindBuffer(GL_ARRAY_BUFFER, Vbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(playerVertices), playerVertices, GL_STATIC_DRAW);

unsigned int Ebo;
glGenBuffers(1, &Ebo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Ebo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(playerElementIndices), playerElementIndices, GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

glBindVertexArray(0);