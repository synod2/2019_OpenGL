

#include "GL/glew.h"
#include "glm/glm.hpp"
#include <iostream>
#include <fstream>
#include "../BaseCodes/Mesh.h"
#include "GrassModel.h"


///////////////////////////////////////////////
// Two-Triangle

static GLuint g_Grass_vao;
static GLuint g_Grass_vbo_pos;
static GLuint g_Grass_vbo_color;
static GLuint g_Grass_vbo_normal;
static GLuint g_Grass_index_buf;

static Mesh g_Grass_mesh;

void InitGrassModel()
{
	g_Grass_mesh.ReadObjFormatFile("../Data/Grass1.obj");

	g_Grass_mesh.TriangulateAndUniformizeBuffers();

	g_Grass_mesh.ScaleUniformlyVertices(0.01f);
	g_Grass_mesh.SetColor(0.0f, 0.6f, 0.2f, 1);

	// Vertex Array Object
	glGenVertexArrays(1, &g_Grass_vao);
	glBindVertexArray(g_Grass_vao);


	// Position VBO
	glGenBuffers(1, &g_Grass_vbo_pos);
	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_pos);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*g_Grass_mesh.num_points(), g_Grass_mesh.points(), GL_STATIC_DRAW);

	// Color VBO
	glGenBuffers(1, &g_Grass_vbo_color);
	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_color);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*4*g_Grass_mesh.num_points(), g_Grass_mesh.colors(), GL_STATIC_DRAW);

	// Normal VBO
	glGenBuffers(1, &g_Grass_vbo_normal);
	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_normal);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat)*3*g_Grass_mesh.num_points(), g_Grass_mesh.normals(), GL_STATIC_DRAW);


	// Index Buffer Object
	glGenBuffers(1, &g_Grass_index_buf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Grass_index_buf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint)*3*g_Grass_mesh.num_triangles(), g_Grass_mesh.triangle_ids(), GL_STATIC_DRAW);


	// Vertex Attribute
	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_pos);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_color);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, g_Grass_vbo_normal);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}


void DrawGrassModel()
{
	glBindVertexArray(g_Grass_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_Grass_index_buf);
	glDrawElements(GL_TRIANGLES, g_Grass_mesh.num_triangles()*3, GL_UNSIGNED_INT, NULL);
}


void DeleteGrassModel()
{
	glDeleteBuffers(1, &g_Grass_vbo_pos);
	glDeleteBuffers(1, &g_Grass_vbo_normal);
	glDeleteBuffers(1, &g_Grass_index_buf);
	glDeleteVertexArrays(1, &g_Grass_vao);
}




