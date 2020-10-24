
#include "GL/glew.h"
#include "glm/glm.hpp"
#include "../BaseCodes/BasicShapeObjs.h"
#include "../BaseCodes/Mesh.h"
#include <iostream>
#include <string>
#include <stack>
#include <stdlib.h>
#include <time.h>

static float g_d = 0.04f;
static float g_delta_x = glm::radians(30.f);
static float g_delta_y = glm::radians(75.f);
static std::string g_initial_str = "X";

std::string Rule(char in)
{
	std::string out;

	if      ( in == 'X' ) out = "F[+X]F[-X]+X";
	else if ( in == 'F' ) out = "FF";
	else                  out = in;

	return out;
}

std::string Reproduce(std::string input)
{
	std::string output;

	for ( unsigned int i=0; i<input.size(); i++ )
	{
		output = output + Rule( input[i] );
	}

	return output;
}

void CreateLSystemTree(int iteration, Mesh &out_mash)
{
	
	float seed=0;
	srand(time(NULL));
	seed = (float)rand() * iteration;
	// Element 1. ³ª¹«±âµÕ 
	Mesh trunk = glm::scale(glm::vec3(0.008f, g_d, 0.008f)) 
				* glm::translate(glm::vec3(0.f, 0.5f, 0.f)) 
				* g_cylinder_mesh;
	trunk.SetColor(0.60f, 0.45f, 0.12f, 1);
	
	// Element 2, ³ª¹µÀÙ
	Mesh leaf = glm::rotate(glm::pi<float>() / 2.f, glm::vec3(0.f, 1.f, 0.f))
		* glm::rotate(-glm::pi<float>() / 6.f, glm::vec3(1.f, 0.f, 0.f))
		* glm::scale(glm::vec3(0.01f, 0.0004f, 0.03f))
		* glm::translate(glm::vec3(0.f, 0.f, 1.f))
		* g_cylinder_mesh;
	leaf.SetColor(0.f, 0.6f, 0.1f, 1.f);

	// Element 3, °¨¿­¸Å 
	Mesh fruit = glm::scale(glm::vec3(0.02f, 0.015f, 0.015f))
		* g_sphere_mesh;
	fruit.SetColor(0.90f, 0.40f, 0.02f, 1.f);


	// String Reproduction
	std::string str = g_initial_str;

	for ( int i=0; i<iteration; i++ )
		str = Reproduce(str);


	std::stack<glm::mat4> stack_T;
	glm::mat4 T(1);
	glm::mat4 L(1);
	glm::mat4 G(1);
	float thick ;
	//int sp = 50;
	for ( unsigned int i=0; i<str.size(); i++ )
	{
		if ( str[i] == 'F' )
		{
			if (iteration > 3)
			{
				thick = 1.0f + (float)(str.size() - i) / str.size();
				out_mash += T * glm::scale(glm::vec3(thick, 1.01f, thick)) * trunk;
			}
			else
				out_mash += T * trunk;
			T = T * glm::translate(glm::vec3(0, g_d, 0));
			T = T * glm::rotate(g_delta_y, glm::vec3(0, 1, 0));

			if (i > 65 && i%2 == 0) {
				L = T * glm::rotate(2 * g_delta_x, glm::vec3(1, 0, 1)); //³ª¹µÀÙ 
				out_mash += L * leaf;
			}
			else if ( i > 40 && i%5 == 0) {
				L = T * glm::rotate(2 * g_delta_x, glm::vec3(1, 0, 1)); //³ª¹µÀÙ 
				out_mash += L * leaf;
			}
		}
		else if ( str[i] == '+' )
		{
			glRotated(g_delta_x, 1, 0, 0);
			T = T * glm::rotate(g_delta_x, glm::vec3(1, 0, 0));
			T = T * glm::rotate(g_delta_y, glm::vec3(0, 1, 0));

			L = T * glm::rotate(2*g_delta_x, glm::vec3(1, 0, 1)); //³ª¹µÀÙ 
			out_mash += L * leaf;
			
			if (iteration > 3 && i > 40 && i%3==0) { // °¨¿­¸Å
				G = T * glm::rotate(-g_delta_x, glm::vec3(1, 0, 0));
				G = G * glm::rotate(-g_delta_y, glm::vec3(0, 1, 0));
				G = G * glm::translate(glm::vec3(0.f, -0.03f, 0.f));
				out_mash += G * fruit;
			}
		}
		else if ( str[i] == '-' )
		{
			glRotated(-g_delta_x, 1, 0, 0);
			T = T * glm::rotate(-g_delta_x, glm::vec3(1, 0, 0));
			T = T * glm::rotate(-g_delta_y, glm::vec3(0, 1, 0));
			
			L = T * glm::rotate(-2*g_delta_x, glm::vec3(1, 0, 1)); //³ª¹µÀÙ 
			out_mash += L * leaf;

		}
		else if ( str[i] == '[' )
		{
			stack_T.push(T);
		}
		else if ( str[i] == ']' )
		{
			T = stack_T.top();
			stack_T.pop();
		}
	
	}


	out_mash.RotateVertices(glm::radians(seed), glm::vec3(0, 1, 0)); //³ª¹« ÀüÃ¼ È¸Àü
}

