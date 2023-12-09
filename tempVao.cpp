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

// Block Vao
unsigned int blockVao;
glGenVertexArrays(1, &blockVao);
glBindVertexArray(blockVao);

unsigned int blockVbo;
glGenBuffers(1, &blockVbo);
glBindBuffer(GL_ARRAY_BUFFER, blockVbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(blockVertex), blockVertex, GL_STATIC_DRAW);

unsigned int blockEbo;
glGenBuffers(1, &blockEbo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, blockEbo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(blockIndices), blockIndices, GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

glBindVertexArray(0);

unsigned int ballVao;
glGenVertexArrays(1, &ballVao);
glBindVertexArray(ballVao);

unsigned int ballVbo;
glGenBuffers(1, &ballVbo);
glBindBuffer(GL_ARRAY_BUFFER, ballVbo);
glBufferData(GL_ARRAY_BUFFER, sizeof(ballVertices), ballVertices, GL_STATIC_DRAW);

unsigned int ballEbo;
glGenBuffers(1, &ballEbo);
glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ballEbo);
glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ballIndices), ballIndices, GL_STATIC_DRAW);

glEnableVertexAttribArray(0);
glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

glBindVertexArray(0);