

//��ǻ�� �������к� 201421314 ������
#include "GL/glew.h"
#include "GL/freeglut.h"
#include "glm/glm.hpp"
#include "glm/ext.hpp"
#include "CarGame0App.h"
#include "../BaseCodes/Camera.h"
#include "../BaseCodes/GroundTexture.h"
#include "../BaseCodes/InitShader.h"
#include "../BaseCodes/BasicShapeObjs.h"
#include "CarModel.h"
#include "GrassModel.h"
#include "TreeModelL.h"
#include <iostream>
#include <math.h>


// Window and User Interface
static bool g_left_button_pushed;
static bool g_right_button_pushed;
static int g_last_mouse_x;
static int g_last_mouse_y;

extern GLuint g_window_w;
extern GLuint g_window_h;

//////////////////////////////////////////////////////////////////////
// Camera 
//////////////////////////////////////////////////////////////////////
static Camera g_camera;
static int g_camera_mode = 0;


//////////////////////////////////////////////////////////////////////
//// Define Shader Programs
//////////////////////////////////////////////////////////////////////
GLuint s_program_id;
bool night_mode = 0;
int lights_num = 25; //���� �� ����� �� ���� ����. �ݵ����� ���� ���� ����.


glm::vec3 rand_pos[100]; //point light ���� ��ġ ����
#define rand_range(range) rand()%(range*2) - range //�������� ��ũ���Լ�


//////////////////////////////////////////////////////////////////////
//// Animation Parameters
//////////////////////////////////////////////////////////////////////
float g_elapsed_time_s = 0.f;	// 
float save_time = 0.f;
int anim_mode = 0; //0 : none , 1 : first -> bird , 2 : bird -> first, 3: free->bird, 4: free->first , 5: first->free , 6: bird -> free

glm::vec3 get_anim(glm::vec3 start_loc, glm::vec3 end_loc, float move_time, float check_time)
{
	glm::vec3 ret_loc;
	ret_loc.x = start_loc.x + move_time * (end_loc.x - start_loc.x / check_time);
	ret_loc.y = start_loc.y + move_time * (end_loc.y - start_loc.y / check_time);
	ret_loc.z = start_loc.z + move_time * (end_loc.z - start_loc.z / check_time);

	return ret_loc;
}; 


int treeL[64] = { 0, };

//////////////////////////////////////////////////////////////////////
//// Car Position, Rotation, Velocity
//// �ڵ��� ���� ������.
//////////////////////////////////////////////////////////////////////
glm::vec3 g_car_position(0.f, 0.f, 0.f); //��ġ
float g_car_speed = 0;			          // �ӵ� (�ʴ� �̵� �Ÿ�)
float g_car_rotation_y = 0;		          // ���� ���� (y�� ȸ��)
float g_car_angular_speed = 0;	          // ȸ�� �ӵ� (���ӵ� - �ʴ� ȸ�� ��)

double Rounding(double x, int digit) //�ݿø� �Լ� 
{
	return (floor((x)*pow(float(10), digit) + 0.5f) / pow(float(10), digit));
}

void make_tree() {
	for (int i = 0; i != 64; i++) {
		treeL[i] = rand();
	}
}

/**
InitOpenGL: ���α׷� �ʱ� �� ������ ���� ���� �� �� ȣ��Ǵ� �Լ�. (main �Լ� ����)
OpenGL�� ������ �ʱ� ���� ���α׷��� �ʿ��� �ٸ� �ʱ� ���� �����Ѵ�.
�������, VAO�� VBO�� ���⼭ ������ �� �ִ�.
*/
void InitOpenGL()
{
	s_program_id = CreateFromFiles("../Shaders/v_shader.glsl", "../Shaders/f_shader.glsl"); //���̴� ���� �о��. 
	glUseProgram(s_program_id);


	glViewport(0, 0, (GLsizei)g_window_w, (GLsizei)g_window_h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// Initial State of Camera
	// ī�޶� �ʱ� ��ġ �����Ѵ�.
	g_camera.lookAt(glm::vec3(3.f, 2.f, 3.f), glm::vec3(0.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));

	// basic meshes
	InitBasicShapeObjs();

	// Tree
	make_tree();
	InitTreeModel();

	// Car
	InitCarModel();

	// Grass
	InitGrassModel();

	// �ٴ� ���� VAO ����
	InitGroundTexture();

	srand(time(NULL));
	int rand_num = 5;
	//���� ������ -5 ���� 5����
	for(int i=0;i!=lights_num;i++)
		rand_pos[i] = glm::vec3(rand_range(rand_num), rand_range(rand_num), rand_range(rand_num));
	
}

/**
ClearOpenGLResource: ���α׷��� ������ �޸� ������ ���� �� �� ȣ��Ǵ� �Լ�. (main �Լ� ����)
���α׷����� ����� �޸𸮸� ���⿡�� ������ �� �ִ�.
�������, VAO�� VBO�� ���⼭ ���� �� �ִ�.
*/
void ClearOpenGLResource()
{
	// Delete (VAO, VBO)
	DeleteBasicShapeObjs();
	DeleteTreeModel();
	DeleteCarModel();
	DeleteGrassModel();
	DeleteGroundTexture();
}

/**
Display: ������ ȭ���� ������Ʈ �� �ʿ䰡 ���� �� ȣ��Ǵ� callback �Լ�.

������ �� ���� ����� ������ �ϴ� �ڵ�� �� �Լ� ���� �����ؾ��Ѵ�.
�����찡 ó�� ���� ��, ������ ũ�Ⱑ �ٲ� ��, �ٸ� �����쿡 ���� ȭ���� �Ϻ�
�Ǵ� ��ü�� �������ٰ� �ٽ� ��Ÿ�� �� �� �ý����� �ش� ������ ���� �׸��� ����
������Ʈ�� �ʿ��ϴٰ� �Ǵ��ϴ� ��� �ڵ����� ȣ��ȴ�.
���� ȣ���� �ʿ��� ��쿡�� glutPostRedisplay() �Լ��� ȣ���ϸ�ȴ�.

�� �Լ��� �ҽÿ� ����ϰ� ȣ��ȴٴ� ���� ����ϰ�, ������ ���� ��ȭ�� ������
1ȸ�� �ڵ�� �������� �� �Լ� �ۿ� �����ؾ��Ѵ�. Ư�� �޸� �Ҵ�, VAO, VBO ����
���� �ϵ���� ������ �õ��ϴ� �ڵ�� Ư���� ������ ���ٸ� ���� �� �Լ��� ���Խ�Ű��
�ȵȴ�. ���� ���, �޽� ���� �����ϰ� VAO, VBO�� �����ϴ� �κ��� ���� 1ȸ��
�����ϸ�ǹǷ� main() �Լ� �� �ܺο� �����ؾ��Ѵ�. ���ǵ� �޽� ���� ������ ���ۿ�
�׸����� �����ϴ� �ڵ常 �� �Լ��� �����ϸ� �ȴ�.

����, �� �Լ� ������ ���� �޸� �Ҵ��� �ؾ��ϴ� ��찡 �ִٸ� �ش� �޸𸮴� �ݵ��
�� �Լ��� ������ ���� ���� �ؾ��Ѵ�.

ref: https://www.opengl.org/resources/libraries/glut/spec3/node46.html#SECTION00081000000000000000
*/
void Display()
{
	// ��ü ȭ���� �����.
	// glClear�� Display �Լ� ���� �� �κп��� �� ���� ȣ��Ǿ���Ѵ�.

	if (night_mode)
		glClearColor(0.3, 0.3, 0.3, 1);
	else
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Vertex shader �� matrix �������� location�� �޾ƿ´�.
	int m_proj_loc = glGetUniformLocation(s_program_id, "proj_matrix");
	int m_view_loc = glGetUniformLocation(s_program_id, "view_matrix");
	int m_model_loc = glGetUniformLocation(s_program_id, "model_matrix");

	//ī�޶� ��ġ, ���� ���� ���� ���� 
	glm::vec3 cam_pos;
	glm::vec3 cam_dir;
		//bird eye view ī�޶� ��ġ ���� 
	glm::vec3 b_loc = glm::vec3(0, 4.f, -4.f);
	glm::vec3 b_dir = glm::vec3(0, 0.f, 0.f);
		//first person ī�޶� ��ġ ���� 
	glm::vec3 f_loc = glm::vec3(0.1f, 0.6f, -0.12f);
	glm::vec3 f_dir = glm::vec3(0.1f, 0.6f, 1.f);


	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;
	
	if ( g_camera_mode == 0 )// mouse control view 
	{
		float check_time = 1; //�ִϸ��̼� ���� �ð�
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;
		glm::vec3 free_dir;
		free_dir.x = g_camera.getRotationForGL().x;
		free_dir.y = g_camera.getRotationForGL().y;
		free_dir.z = g_camera.getRotationForGL().z;
		if (g_elapsed_time_s < end_time) { // bird -> free
			if (anim_mode == 5) {
				// Projection Transform Matrix ����.
				projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f); 
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				cam_pos = g_car_position + glm::rotate(get_anim(b_loc, g_camera.getTranslation(), move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(b_dir, free_dir,move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));

				view_matrix = glm::lookAt(cam_pos, cam_dir, glm::vec3(0.f, 1.f, 0.f));
			}
			else if (anim_mode == 6) {	 // first -> free
				projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
								
				cam_pos = g_car_position + glm::rotate(get_anim(f_loc, g_camera.getTranslation(), move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(f_dir, free_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));

				view_matrix = glm::lookAt(cam_pos, cam_dir, glm::vec3(0.f, 1.f, 0.f));
			}		
		}
		else {
			// Projection Transform Matrix ����.
			projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			// Camera Transform Matrix ����.
			view_matrix = g_camera.GetGLViewMatrix();	//ī�޶��� ��ġ ����
			

		}
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
			
	}
	else if( g_camera_mode == 1 ) // bird eye view 
	{
		float check_time = 1; //�ִϸ��̼� ���� �ð�
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;

		if (g_elapsed_time_s < end_time) {	//�ش� ���� ���� �� �ִϸ��̼� ��� 
			if (anim_mode == 1) {	// first -> bird ��ȯ �ִϸ��̼�
				// Projection Transform Matrix ����.
				projection_matrix = glm::perspective(glm::radians(100.f - move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //100 -> 45
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				//cam_pos = g_car_position + glm::rotate(glm::vec3(f_loc.x + move_time * (b_loc.x - f_loc.x / check_time), f_loc.y + move_time * (b_loc.y-f_loc.y / check_time), f_loc.z + move_time * (b_loc.z-f_loc.z / check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(f_loc, b_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(f_dir, b_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
			else if (anim_mode == 3) //free->bird
			{
				// Projection Transform Matrix ����.
				projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f); // 45->45
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				cam_pos = g_car_position + glm::rotate(get_anim(g_camera.getTranslation(), b_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(g_camera.getTranslationForGL(), b_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));

			}
		}
		else { 
			// Projection Transform Matrix ����.
			projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			// Camera Transform Matrix ����.		
			cam_pos = g_car_position + glm::rotate(b_loc, g_car_rotation_y, glm::vec3(0, 1, 0)); // ī�޶� ��ġ = ����ǥ + �̵��� ��ǥ�� ȸ����ȯ 
			cam_dir = g_car_position;
		
		}
		view_matrix = glm::lookAt(cam_pos, cam_dir , glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 2) //first person view 
	{
		float check_time = 1;  //�ִϸ��̼� ���� �ð�
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;		
			   		
		if (g_elapsed_time_s < end_time) {
			if (anim_mode == 2) { // bird->fisrt
				// ��ȯ �ִϸ��̼� 

				projection_matrix = glm::perspective(glm::radians(45.f + move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //45->100
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				// ī�޶� ��ȯ �ִϸ��̼�. �ð��� ���� ���� ����.
				//cam_pos = g_car_position + glm::rotate(glm::vec3(0.1f, 4.f - move_time * (3.4f/check_time), -4.f + move_time *(3.88f/check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(b_loc, f_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(b_dir, f_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
			else if (anim_mode == 4) //free->first 
			{
				projection_matrix = glm::perspective(glm::radians(45.f + move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //45->100
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				// ī�޶� ��ȯ �ִϸ��̼�. �ð��� ���� ���� ����.
				//cam_pos = g_car_position + glm::rotate(glm::vec3(0.1f, 4.f - move_time * (3.4f/check_time), -4.f + move_time *(3.88f/check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(g_camera.getTranslation(), f_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(g_camera.getTranslationForGL(), f_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
		}
		else { 
			
			// Projection Transform Matrix ����.
			projection_matrix = glm::perspective(glm::radians(100.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
			// Camera Transform Matrix ����.
			cam_pos = g_car_position + glm::rotate(f_loc, g_car_rotation_y, glm::vec3(0, 1, 0)); // ī�޶� ��ġ = ����ǥ + �̵��� ��ǥ�� ȸ����ȯ 
			cam_dir = g_car_position + glm::rotate(f_dir, g_car_rotation_y, glm::vec3(0, 1, 0)); // �ٶ󺸷��� ���� ���� �ڵ����� ���� �κ� �̹Ƿ� ȸ����ȯ ���� �ʿ�. 
		}
		view_matrix = glm::lookAt(cam_pos ,cam_dir , glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));	
	}

	
	{
		int shading_mode = glGetUniformLocation(s_program_id, "night_mode");
		
		int num_of_lights_loc = glGetUniformLocation(s_program_id, "num_of_lights");
		glUniform1i(num_of_lights_loc, lights_num);

		// Directional Light ����
		{
			// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light)
			int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
			glUniform1i(type_loc, 0);

			glm::vec3 light_dir = glm::vec3(-1.f, -1.f, -1.f);

			light_dir = glm::normalize(light_dir);

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  light_dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� ��0.f��

			light_dir = glm::vec3(view_matrix * glm::vec4(light_dir, 0.f));
			int light_dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
			glUniform3f(light_dir_loc, light_dir[0], light_dir[1], light_dir[2]);


			int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity"); //�� �㿡 ���� ���� ���� ����
			if (!night_mode) 
			{
				glUniform3f(intensity_loc, 1.f, 1.f, 1.f); 
				glUniform1i(shading_mode, 0); //��		
			}
			else //�޺� ���� 
			{
				float intens = 0.15f;
				glUniform3f(intensity_loc, intens, intens, intens);
				glUniform1i(shading_mode, 1); //��
			}
		}
		glm::vec3 sl_pos = g_car_position + glm::rotate(glm::vec3(0.14f, 0.37f, 0.4f), g_car_rotation_y, glm::vec3(0, 1, 0));
		// Spot Light ���� ����.
		{
			// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl ����.
			int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
			glUniform1i(type_loc, 2);

			// ���� ����ϴ� ��ġ(����) ����.
			glm::vec3 pos = sl_pos;
			// Apply Camera Matrices
			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  �̶� pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��° ��� 1.f���� ����)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[1].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

			// ���� ���� ����.
			glm::vec3 dir = glm::rotate(glm::vec3(0.1f, -0.2f, 1.f), g_car_rotation_y, glm::vec3(0, 1, 0));
			dir = glm::normalize(dir);
			//���� �ڵ��� ������Ʈ��� �� �� ���� ������ Ŀ���ϸ� ���߾�� �Ѵ�.
			//���� ���� �ణ ������ �ٱ���, �Ʒ����� ���ϰ� �������־���.

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� 0.f���� ����)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[1].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// ���� ���� ���� (�����).
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[1].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 0.f);

			// ���� ���� ���� ����.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[1].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(20.f / 180.f * glm::pi<float>()));

			//����Ʈ����Ʈ ���� ������Ʈ
			{
				glm::mat4 model_T;
				glm::vec3 pl_size = glm::vec3(0.03f, 0.03f, 0.03f); //��ü ũ�� 
				model_T = glm::translate(sl_pos) * glm::scale(pl_size);
				glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

				//������Ʈ �����Ʈ ���� 
				float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
				glUniform1f(ambient_factor, 1.50f);

				glVertexAttrib4f(2, 1.f, 1.f, 0.0f, 1.f);
				if (night_mode)
					DrawSphere();
			}
		}
		sl_pos = g_car_position + glm::rotate(glm::vec3(-0.14f, 0.37f, 0.4f), g_car_rotation_y, glm::vec3(0, 1, 0));
		// Spot Light ������ ����.
		{
			// ���� ���� ���� (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl ����.
			int type_loc = glGetUniformLocation(s_program_id, "lights[2].type");
			glUniform1i(type_loc, 2);

			// ���� ����ϴ� ��ġ(����) ����.
			glm::vec3 pos = sl_pos;
			// Apply Camera Matrices
			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  �̶� pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��° ��� 1.f���� ����)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[2].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

			// ���� ���� ����.
			glm::vec3 dir = glm::rotate(glm::vec3(-0.1f, -0.2f, 1.f), g_car_rotation_y, glm::vec3(0, 1, 0));
			dir = glm::normalize(dir);

			////// *** ���� ī�޶� ������ ����ϱ� ���� view transform ����  ***
			//  dir�� ������ ��Ÿ���� �����̹Ƿ� �̵�(Translation)��ȯ�� ���õǵ��� �Ѵ�. (�� ��° ��� 0.f���� ����)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[2].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// ���� ���� ���� (�����).
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[2].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 0.f);

			// ���� ���� ���� ����.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[2].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(20.f / 180.f * glm::pi<float>()));

			//����Ʈ����Ʈ ���� ������Ʈ
			{
				glm::mat4 model_T;
				glm::vec3 pl_size = glm::vec3(0.03f, 0.03f, 0.03f); //��ü ũ�� 
				model_T = glm::translate(sl_pos) * glm::scale(pl_size);
				glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

				//������Ʈ �����Ʈ ���� 
				float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
				glUniform1f(ambient_factor, 1.50f);

				glVertexAttrib4f(2, 1.f, 1.f, 0.0f, 1.f);
				if (night_mode)
					DrawSphere();
			}
		}

		//Point Light ����
		{	
			for (int i = 3;i!=lights_num-3; i++) {

				std::string str_tmp = "lights[" + std::to_string(i);
				//�迭 ����� ���ڿ� ���� 
				
				int type_loc = glGetUniformLocation(s_program_id, (char *)(str_tmp + "].type").c_str());
				glUniform1i(type_loc, 1);
				

				// ���� ����ϴ� ��ġ(����) ����.
				// �ð��� ���� ��ġ�� ���ϵ��� ��.
								
				glm::vec3 pl_pos(rand_pos[i].x + cos(rand_pos[i].x+g_elapsed_time_s), (cos(rand_pos[i].y + g_elapsed_time_s)+2)/5, rand_pos[i].z + sin(rand_pos[i].x+g_elapsed_time_s));

				glm::vec3 pos;
				pos = glm::vec3(view_matrix * glm::vec4(pl_pos, 1.f));

				//pos�� ��ġ�� ��Ÿ���� ����Ʈ�̹Ƿ� �̵�(Translation)��ȯ�� ����ǵ��� �Ѵ�. (�� ��°  ��Ҹ� 1.f��)
				int pos_loc = glGetUniformLocation(s_program_id, (char *)(str_tmp + "].position").c_str());
				glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

				//���� ���� ����
				int color_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].color").c_str());
				glUniform3f(color_loc, 0, 2, 2);

				//���ݻ� ���� 
				int spec_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].K_s").c_str());
				glUniform3f(spec_loc, 0.1f, 0.1f, 0.1f);

				// ���� ���� ����.
				float intens = 0.55f;
				int intensity_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].intensity").c_str());
				glUniform3f(intensity_loc, intens, intens, intens);

				//���� ������Ʈ
				{
					glm::mat4 model_T;
					glm::vec3 pl_size = glm::vec3(0.02f, 0.02f, 0.02f); //��ü ũ�� 
					model_T = glm::translate(pl_pos) * glm::scale(pl_size);
					glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

					//������Ʈ �����Ʈ ���� 
					float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
					glUniform1f(ambient_factor, 1.50f);

					glVertexAttrib4f(2, 0.f, 2.f, 2.0f, 1.f);
					if(night_mode)
						DrawSphere();
				}
			}
		}
		
		
	}


	// �ٴ� Texture
	{
		glUniform1i(glGetUniformLocation(s_program_id, "flag_texture"), true);

		glm::mat4 T0(1.f); // ���� ���
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(T0));

		float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
		glUniform1f(ambient_factor, 0.05f);

		DrawGroundTexture();
		glUniform1i(glGetUniformLocation(s_program_id, "flag_texture"), false);
	}
	// Moving Car
	{
		glm::mat4 car_T = glm::translate(g_car_position) * glm::rotate(g_car_rotation_y, glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_model_loc, 1, GL_FALSE,  glm::value_ptr(car_T));
		float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
		glUniform1f(ambient_factor, 0.05f);

		DrawCarModel();
	}

	// �ܵ� 
	{
		for (int i = 0; i != 20; i++) {
			for (int j = 0; j != 60; j++) {
				glm::mat4 model_G;
				model_G = glm::translate(glm::vec3(i * 0.45f - 4.f, 0.f, j * 0.15f - 4.f));
				glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_G));

				float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
				glUniform1f(ambient_factor, 0.05f);

				DrawGrassModel();
			}
		}
	}
	// ����
	for (int i = 0; i <= 6; i++)
	{
		for (int j = 0; j <= 6; j++)
		{
			glm::mat4 model_T;
			model_T = glm::translate(glm::vec3(i * 1.6f - 5.f, 0.f, j * 1.6f - 5.f));
			glUniformMatrix4fv(m_model_loc, 1, GL_FALSE, glm::value_ptr(model_T));

			float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
			glUniform1f(ambient_factor, 0.05f);
			
			DrawTreeModel(treeL[j+(i*5)] % NUM_TREES);
		}
	}
			
	// flipping the double buffers
	// glutSwapBuffers�� �׻� Display �Լ� ���� �Ʒ� �κп��� �� ���� ȣ��Ǿ���Ѵ�.
	glutSwapBuffers();
}


/**
Timer: ������ �ð� �Ŀ� �ڵ����� ȣ��Ǵ� callback �Լ�.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node64.html#SECTION000819000000000000000
*/
void Timer(int value)
{
	// Timer ȣ�� �ð� ������ �����Ͽ�, ���� Timer�� ȣ��� �ĺ��� ������� �귯�� ����Ѵ�.
	g_elapsed_time_s += value/1000.f;


	// Turn
	g_car_rotation_y += g_car_angular_speed;

	// Calculate Velocity
	glm::vec3 speed_v = glm::vec3(0.f, 0.f, g_car_speed);
	glm::vec3 velocity = glm::rotateY(speed_v, g_car_rotation_y);	// speed_v �� y���� �������� g_car_rotation_y ��ŭ ȸ���Ѵ�.

	// Move
	g_car_position += velocity;


	// glutPostRedisplay�� ������ ���� �ð� �ȿ� ��ü �׸��� �ٽ� �׸� ���� �ý��ۿ� ��û�Ѵ�.
	// ��������� Display() �Լ��� ȣ�� �ȴ�.
	glutPostRedisplay();

	// 1/60 �� �Ŀ� Timer �Լ��� �ٽ� ȣ��Ƿη� �Ѵ�.
	// Timer �Լ� �� ������ �ð� �������� �ݺ� ȣ��ǰ��Ͽ�,
	// �ִϸ��̼� ȿ���� ǥ���� �� �ִ�
	glutTimerFunc((unsigned int)(1000 / 60), Timer, (1000 / 60));
}




/**
Reshape: �������� ũ�Ⱑ ������ ������ �ڵ����� ȣ��Ǵ� callback �Լ�.

@param w, h�� ���� ������ �������� ���� ũ��� ���� ũ�� (�ȼ� ����).
ref: https://www.opengl.org/resources/libraries/glut/spec3/node48.html#SECTION00083000000000000000
*/
void Reshape(int w, int h)
{
	//  w : window width   h : window height
	g_window_w = w;
	g_window_h = h;

	glViewport(0, 0, (GLsizei)w, (GLsizei)h);

	glutPostRedisplay();
}

/**
Keyboard: Ű���� �Է��� ���� ������ �ڵ����� ȣ��Ǵ� �Լ�.
@param key�� ������ Ű������ ���ڰ�.
@param x,y�� ���� ���콺 �������� ��ǥ��.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = -0.03f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.03f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = glm::radians( 2.f );		// ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = -1 * glm::radians( 2.f );		//  ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case 'n':
		night_mode = !night_mode;
		glutPostRedisplay();
		break;

	case 't':	//���� ����� �Լ� ȣ��
		make_tree();
		break; 

	case '1': //mouse control view 
		
		if (g_camera_mode == 1) { //bird->free
			anim_mode = 5;
			save_time = g_elapsed_time_s;
		}
		else if (g_camera_mode == 2) { //first->free
			anim_mode = 6;
			save_time = g_elapsed_time_s;
		}
		else {
			anim_mode = 0;
		}
		
		g_camera_mode = 0;
		glutPostRedisplay();
		break;

	case '2': //bird eye view
		if (g_camera_mode == 2) { //first -> bird.
			anim_mode = 1;
			save_time = g_elapsed_time_s;
		}
		else if (g_camera_mode == 0) { //free -> bird
			anim_mode = 3;
			save_time = g_elapsed_time_s;
		}
		else {
			anim_mode = 0;
		}
		g_camera_mode = 1;
		
		glutPostRedisplay();
		break;
	
	case '3' : //First person view 
		if (g_camera_mode == 1) {// bird -> first.
			anim_mode = 2;
			save_time = g_elapsed_time_s;
		}
		else if (g_camera_mode == 0){ //free -> first
			anim_mode = 4;
			save_time = g_elapsed_time_s;		
		}
		else {
			anim_mode = 0;
		}
		g_camera_mode = 2;	
		glutPostRedisplay();
		break;
	}

}

/**
KeyboardUp: �������� Ű�� ������ ������ �ڵ����� ȣ��Ǵ� �Լ�.
@param key�� �ش� Ű������ ���ڰ�.
@param x,y�� ���� ���콺 �������� ��ǥ��.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = 0.f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.f;		// ���� �ӵ� ����
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = 0.f;		// ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = 0.f;		//  ��ȸ�� ���ӵ� ����
		glutPostRedisplay();
		break;

	}

}



/**
Mouse: ���콺 ��ư�� �Էµ� ������ �ڵ����� ȣ��Ǵ� �Լ�.
�Ķ������ �ǹ̴� ������ ����.
@param button: ���� ��ư�� ����
  GLUT_LEFT_BUTTON - ���� ��ư
  GLUT_RIGHT_BUTTON - ������ ��ư
  GLUT_MIDDLE_BUTTON - ��� ��ư (���� �������� ��)
  3 - ���콺 �� (���� ���� ���� ����).
  4 - ���콺 �� (���� �Ʒ��� ���� ����).
@param state: ���� ����
  GLUT_DOWN - ���� ����
  GLUT_UP - ��������
@param x,y: ������ �Ͼ�� ��, ���콺 �������� ��ǥ��.
*/
void Mouse(int button, int state, int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;


	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
		g_left_button_pushed = true;

	else if (button == GLUT_LEFT_BUTTON && state == GLUT_UP)
		g_left_button_pushed = false;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
		g_right_button_pushed = true;

	else if (button == GLUT_RIGHT_BUTTON && state == GLUT_UP)
		g_right_button_pushed = false;
	else if (button == 3)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, -1, 0.01f);
		glutPostRedisplay();
	}
	else if (button == 4)
	{
		g_camera.inputMouse(Camera::IN_TRANS_Z, 0, 1, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}





/**
MouseMotion: ���콺 �����Ͱ� ������ ������ �ڵ����� ȣ��Ǵ� �Լ�.
@prarm x,y�� ���� ���콺 �������� ��ǥ���� ��Ÿ����.
*/
void MouseMotion(int x, int y)
{
	float mouse_xd = (float)x / g_window_w;
	float mouse_yd = 1 - (float)y / g_window_h;
	float last_mouse_xd = (float)g_last_mouse_x / g_window_w;
	float last_mouse_yd = 1 - (float)g_last_mouse_y / g_window_h;

	if (g_left_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_ROTATION_Y_UP, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd);
		glutPostRedisplay();
	}
	else if (g_right_button_pushed)
	{
		g_camera.inputMouse(Camera::IN_TRANS, last_mouse_xd, last_mouse_yd, mouse_xd, mouse_yd, 0.01f);
		glutPostRedisplay();
	}

	g_last_mouse_x = x;
	g_last_mouse_y = y;
}


