

//컴퓨터 정보공학부 201421314 강현신
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
int lights_num = 25; //게임 내 사용할 총 광원 숫자. 반딧불이 숫자 조절 가능.


glm::vec3 rand_pos[100]; //point light 랜덤 위치 변수
#define rand_range(range) rand()%(range*2) - range //랜덤생성 매크로함수


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
//// 자동차 제어 변수들.
//////////////////////////////////////////////////////////////////////
glm::vec3 g_car_position(0.f, 0.f, 0.f); //위치
float g_car_speed = 0;			          // 속도 (초당 이동 거리)
float g_car_rotation_y = 0;		          // 현재 방향 (y축 회전)
float g_car_angular_speed = 0;	          // 회전 속도 (각속도 - 초당 회전 각)

double Rounding(double x, int digit) //반올림 함수 
{
	return (floor((x)*pow(float(10), digit) + 0.5f) / pow(float(10), digit));
}

void make_tree() {
	for (int i = 0; i != 64; i++) {
		treeL[i] = rand();
	}
}

/**
InitOpenGL: 프로그램 초기 값 설정을 위해 최초 한 번 호출되는 함수. (main 함수 참고)
OpenGL에 관련한 초기 값과 프로그램에 필요한 다른 초기 값을 설정한다.
예를들어, VAO와 VBO를 여기서 생성할 수 있다.
*/
void InitOpenGL()
{
	s_program_id = CreateFromFiles("../Shaders/v_shader.glsl", "../Shaders/f_shader.glsl"); //쉐이더 정보 읽어옴. 
	glUseProgram(s_program_id);


	glViewport(0, 0, (GLsizei)g_window_w, (GLsizei)g_window_h);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);


	// Initial State of Camera
	// 카메라 초기 위치 설정한다.
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

	// 바닥 격자 VAO 생성
	InitGroundTexture();

	srand(time(NULL));
	int rand_num = 5;
	//랜덤 범위는 -5 부터 5까지
	for(int i=0;i!=lights_num;i++)
		rand_pos[i] = glm::vec3(rand_range(rand_num), rand_range(rand_num), rand_range(rand_num));
	
}

/**
ClearOpenGLResource: 프로그램이 끝나기 메모리 해제를 위해 한 번 호출되는 함수. (main 함수 참고)
프로그램에서 사용한 메모리를 여기에서 해제할 수 있다.
예를들어, VAO와 VBO를 여기서 지울 수 있다.
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
Display: 윈도우 화면이 업데이트 될 필요가 있을 때 호출되는 callback 함수.

윈도우 상에 최종 결과를 렌더링 하는 코드는 이 함수 내에 구현해야한다.
원도우가 처음 열릴 때, 윈도우 크기가 바뀔 때, 다른 윈도우에 의해 화면의 일부
또는 전체가 가려졌다가 다시 나타날 때 등 시스템이 해당 윈도우 내의 그림에 대한
업데이트가 필요하다고 판단하는 경우 자동으로 호출된다.
강제 호출이 필요한 경우에는 glutPostRedisplay() 함수를 호출하면된다.

이 함수는 불시에 빈번하게 호출된다는 것을 명심하고, 윈도우 상태 변화와 무관한
1회성 코드는 가능한한 이 함수 밖에 구현해야한다. 특히 메모리 할당, VAO, VBO 생성
등의 하드웨어 점유를 시도하는 코드는 특별한 이유가 없다면 절대 이 함수에 포함시키면
안된다. 예를 들어, 메시 모델을 정의하고 VAO, VBO를 설정하는 부분은 최초 1회만
실행하면되므로 main() 함수 등 외부에 구현해야한다. 정의된 메시 모델을 프레임 버퍼에
그리도록 지시하는 코드만 이 함수에 구현하면 된다.

만일, 이 함수 내에서 동적 메모리 할당을 해야하는 경우가 있다면 해당 메모리는 반드시
이 함수가 끝나기 전에 해제 해야한다.

ref: https://www.opengl.org/resources/libraries/glut/spec3/node46.html#SECTION00081000000000000000
*/
void Display()
{
	// 전체 화면을 지운다.
	// glClear는 Display 함수 가장 윗 부분에서 한 번만 호출되어야한다.

	if (night_mode)
		glClearColor(0.3, 0.3, 0.3, 1);
	else
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Vertex shader 의 matrix 변수들의 location을 받아온다.
	int m_proj_loc = glGetUniformLocation(s_program_id, "proj_matrix");
	int m_view_loc = glGetUniformLocation(s_program_id, "view_matrix");
	int m_model_loc = glGetUniformLocation(s_program_id, "model_matrix");

	//카메라 위치, 방향 관련 변수 선언 
	glm::vec3 cam_pos;
	glm::vec3 cam_dir;
		//bird eye view 카메라 위치 방향 
	glm::vec3 b_loc = glm::vec3(0, 4.f, -4.f);
	glm::vec3 b_dir = glm::vec3(0, 0.f, 0.f);
		//first person 카메라 위치 방향 
	glm::vec3 f_loc = glm::vec3(0.1f, 0.6f, -0.12f);
	glm::vec3 f_dir = glm::vec3(0.1f, 0.6f, 1.f);


	glm::mat4 projection_matrix;
	glm::mat4 view_matrix;
	
	if ( g_camera_mode == 0 )// mouse control view 
	{
		float check_time = 1; //애니메이션 실행 시간
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;
		glm::vec3 free_dir;
		free_dir.x = g_camera.getRotationForGL().x;
		free_dir.y = g_camera.getRotationForGL().y;
		free_dir.z = g_camera.getRotationForGL().z;
		if (g_elapsed_time_s < end_time) { // bird -> free
			if (anim_mode == 5) {
				// Projection Transform Matrix 설정.
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
			// Projection Transform Matrix 설정.
			projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			// Camera Transform Matrix 설정.
			view_matrix = g_camera.GetGLViewMatrix();	//카메라의 위치 설정
			

		}
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
			
	}
	else if( g_camera_mode == 1 ) // bird eye view 
	{
		float check_time = 1; //애니메이션 실행 시간
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;

		if (g_elapsed_time_s < end_time) {	//해당 조건 만족 시 애니메이션 재생 
			if (anim_mode == 1) {	// first -> bird 전환 애니메이션
				// Projection Transform Matrix 설정.
				projection_matrix = glm::perspective(glm::radians(100.f - move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //100 -> 45
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				//cam_pos = g_car_position + glm::rotate(glm::vec3(f_loc.x + move_time * (b_loc.x - f_loc.x / check_time), f_loc.y + move_time * (b_loc.y-f_loc.y / check_time), f_loc.z + move_time * (b_loc.z-f_loc.z / check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(f_loc, b_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(f_dir, b_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
			else if (anim_mode == 3) //free->bird
			{
				// Projection Transform Matrix 설정.
				projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f); // 45->45
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				cam_pos = g_car_position + glm::rotate(get_anim(g_camera.getTranslation(), b_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(g_camera.getTranslationForGL(), b_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));

			}
		}
		else { 
			// Projection Transform Matrix 설정.
			projection_matrix = glm::perspective(glm::radians(45.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

			// Camera Transform Matrix 설정.		
			cam_pos = g_car_position + glm::rotate(b_loc, g_car_rotation_y, glm::vec3(0, 1, 0)); // 카메라 위치 = 원좌표 + 이동할 좌표의 회전변환 
			cam_dir = g_car_position;
		
		}
		view_matrix = glm::lookAt(cam_pos, cam_dir , glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));
	}
	else if (g_camera_mode == 2) //first person view 
	{
		float check_time = 1;  //애니메이션 실행 시간
		float end_time = save_time + check_time;
		float start_time = save_time;
		float move_time = g_elapsed_time_s - start_time;		
			   		
		if (g_elapsed_time_s < end_time) {
			if (anim_mode == 2) { // bird->fisrt
				// 전환 애니메이션 

				projection_matrix = glm::perspective(glm::radians(45.f + move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //45->100
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				// 카메라 전환 애니메이션. 시간에 따라 배율 조정.
				//cam_pos = g_car_position + glm::rotate(glm::vec3(0.1f, 4.f - move_time * (3.4f/check_time), -4.f + move_time *(3.88f/check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(b_loc, f_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(b_dir, f_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
			else if (anim_mode == 4) //free->first 
			{
				projection_matrix = glm::perspective(glm::radians(45.f + move_time * (55 / check_time)), (float)g_window_w / g_window_h, 0.01f, 10000.f); //45->100
				glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));

				// 카메라 전환 애니메이션. 시간에 따라 배율 조정.
				//cam_pos = g_car_position + glm::rotate(glm::vec3(0.1f, 4.f - move_time * (3.4f/check_time), -4.f + move_time *(3.88f/check_time)), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_pos = g_car_position + glm::rotate(get_anim(g_camera.getTranslation(), f_loc, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
				cam_dir = g_car_position + glm::rotate(get_anim(g_camera.getTranslationForGL(), f_dir, move_time, check_time), g_car_rotation_y, glm::vec3(0, 1, 0));
			}
		}
		else { 
			
			// Projection Transform Matrix 설정.
			projection_matrix = glm::perspective(glm::radians(100.f), (float)g_window_w / g_window_h, 0.01f, 10000.f);
			glUniformMatrix4fv(m_proj_loc, 1, GL_FALSE, glm::value_ptr(projection_matrix));
			// Camera Transform Matrix 설정.
			cam_pos = g_car_position + glm::rotate(f_loc, g_car_rotation_y, glm::vec3(0, 1, 0)); // 카메라 위치 = 원좌표 + 이동할 좌표의 회전변환 
			cam_dir = g_car_position + glm::rotate(f_dir, g_car_rotation_y, glm::vec3(0, 1, 0)); // 바라보려는 방향 또한 자동차의 앞쪽 부분 이므로 회전변환 적용 필요. 
		}
		view_matrix = glm::lookAt(cam_pos ,cam_dir , glm::vec3(0.f, 1.f, 0.f));
		glUniformMatrix4fv(m_view_loc, 1, GL_FALSE, glm::value_ptr(view_matrix));	
	}

	
	{
		int shading_mode = glGetUniformLocation(s_program_id, "night_mode");
		
		int num_of_lights_loc = glGetUniformLocation(s_program_id, "num_of_lights");
		glUniform1i(num_of_lights_loc, lights_num);

		// Directional Light 설정
		{
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light)
			int type_loc = glGetUniformLocation(s_program_id, "lights[0].type");
			glUniform1i(type_loc, 0);

			glm::vec3 light_dir = glm::vec3(-1.f, -1.f, -1.f);

			light_dir = glm::normalize(light_dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  light_dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 를0.f로

			light_dir = glm::vec3(view_matrix * glm::vec4(light_dir, 0.f));
			int light_dir_loc = glGetUniformLocation(s_program_id, "lights[0].dir");
			glUniform3f(light_dir_loc, light_dir[0], light_dir[1], light_dir[2]);


			int intensity_loc = glGetUniformLocation(s_program_id, "lights[0].intensity"); //낮 밤에 따른 빛의 세기 설정
			if (!night_mode) 
			{
				glUniform3f(intensity_loc, 1.f, 1.f, 1.f); 
				glUniform1i(shading_mode, 0); //낮		
			}
			else //달빛 설정 
			{
				float intens = 0.15f;
				glUniform3f(intensity_loc, intens, intens, intens);
				glUniform1i(shading_mode, 1); //밤
			}
		}
		glm::vec3 sl_pos = g_car_position + glm::rotate(glm::vec3(0.14f, 0.37f, 0.4f), g_car_rotation_y, glm::vec3(0, 1, 0));
		// Spot Light 왼쪽 설정.
		{
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[1].type");
			glUniform1i(type_loc, 2);

			// 빛이 출발하는 위치(광원) 설정.
			glm::vec3 pos = sl_pos;
			// Apply Camera Matrices
			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[1].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

			// 빛의 방향 설정.
			glm::vec3 dir = glm::rotate(glm::vec3(0.1f, -0.2f, 1.f), g_car_rotation_y, glm::vec3(0, 1, 0));
			dir = glm::normalize(dir);
			//실제 자동차 헤드라이트라면 좀 더 넓은 범위를 커버하며 비추어야 한다.
			//따라서 각기 약간 차량의 바깥쪽, 아래쪽을 향하게 조정해주었다.

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[1].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// 빛의 세기 설정 (노란색).
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[1].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 0.f);

			// 빛의 퍼짐 정도 설정.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[1].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(20.f / 180.f * glm::pi<float>()));

			//스포트라이트 조명 오브젝트
			{
				glm::mat4 model_T;
				glm::vec3 pl_size = glm::vec3(0.03f, 0.03f, 0.03f); //구체 크기 
				model_T = glm::translate(sl_pos) * glm::scale(pl_size);
				glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

				//오브젝트 엠비언트 설정 
				float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
				glUniform1f(ambient_factor, 1.50f);

				glVertexAttrib4f(2, 1.f, 1.f, 0.0f, 1.f);
				if (night_mode)
					DrawSphere();
			}
		}
		sl_pos = g_car_position + glm::rotate(glm::vec3(-0.14f, 0.37f, 0.4f), g_car_rotation_y, glm::vec3(0, 1, 0));
		// Spot Light 오른쪽 설정.
		{
			// 빛의 종류 설정 (0: Directionl Light, 1: Point Light, 2: Spot Light), fshader_MultiLights.glsl 참고.
			int type_loc = glGetUniformLocation(s_program_id, "lights[2].type");
			glUniform1i(type_loc, 2);

			// 빛이 출발하는 위치(광원) 설정.
			glm::vec3 pos = sl_pos;
			// Apply Camera Matrices
			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  이때 pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째 요소 1.f으로 셋팅)
			pos = glm::vec3(view_matrix * glm::vec4(pos, 1.f));

			int pos_loc = glGetUniformLocation(s_program_id, "lights[2].position");
			glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

			// 빛의 방향 설정.
			glm::vec3 dir = glm::rotate(glm::vec3(-0.1f, -0.2f, 1.f), g_car_rotation_y, glm::vec3(0, 1, 0));
			dir = glm::normalize(dir);

			////// *** 현재 카메라 방향을 고려하기 위해 view transform 적용  ***
			//  dir는 방향을 나타내는 벡터이므로 이동(Translation)변환은 무시되도록 한다. (네 번째 요소 0.f으로 셋팅)
			dir = glm::vec3(view_matrix * glm::vec4(dir, 0.f));

			int dir_loc = glGetUniformLocation(s_program_id, "lights[2].dir");
			glUniform3f(dir_loc, dir[0], dir[1], dir[2]);

			// 빛의 세기 설정 (노랑색).
			int intensity_loc = glGetUniformLocation(s_program_id, "lights[2].intensity");
			glUniform3f(intensity_loc, 1.f, 1.f, 0.f);

			// 빛의 퍼짐 정도 설정.
			int light_cos_cutoff_loc = glGetUniformLocation(s_program_id, "lights[2].cos_cutoff");
			glUniform1f(light_cos_cutoff_loc, cos(20.f / 180.f * glm::pi<float>()));

			//스포트라이트 조명 오브젝트
			{
				glm::mat4 model_T;
				glm::vec3 pl_size = glm::vec3(0.03f, 0.03f, 0.03f); //구체 크기 
				model_T = glm::translate(sl_pos) * glm::scale(pl_size);
				glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

				//오브젝트 엠비언트 설정 
				float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
				glUniform1f(ambient_factor, 1.50f);

				glVertexAttrib4f(2, 1.f, 1.f, 0.0f, 1.f);
				if (night_mode)
					DrawSphere();
			}
		}

		//Point Light 설정
		{	
			for (int i = 3;i!=lights_num-3; i++) {

				std::string str_tmp = "lights[" + std::to_string(i);
				//배열 사용할 문자열 결합 
				
				int type_loc = glGetUniformLocation(s_program_id, (char *)(str_tmp + "].type").c_str());
				glUniform1i(type_loc, 1);
				

				// 빛이 출발하는 위치(광원) 설정.
				// 시간에 따라 위치가 변하도록 함.
								
				glm::vec3 pl_pos(rand_pos[i].x + cos(rand_pos[i].x+g_elapsed_time_s), (cos(rand_pos[i].y + g_elapsed_time_s)+2)/5, rand_pos[i].z + sin(rand_pos[i].x+g_elapsed_time_s));

				glm::vec3 pos;
				pos = glm::vec3(view_matrix * glm::vec4(pl_pos, 1.f));

				//pos는 위치를 나타내는 포인트이므로 이동(Translation)변환이 적용되도록 한다. (네 번째  요소를 1.f로)
				int pos_loc = glGetUniformLocation(s_program_id, (char *)(str_tmp + "].position").c_str());
				glUniform3f(pos_loc, pos[0], pos[1], pos[2]);

				//빛의 색상 설정
				int color_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].color").c_str());
				glUniform3f(color_loc, 0, 2, 2);

				//정반사 설정 
				int spec_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].K_s").c_str());
				glUniform3f(spec_loc, 0.1f, 0.1f, 0.1f);

				// 빛의 세기 설정.
				float intens = 0.55f;
				int intensity_loc = glGetUniformLocation(s_program_id, (char*)(str_tmp + "].intensity").c_str());
				glUniform3f(intensity_loc, intens, intens, intens);

				//조명 오브젝트
				{
					glm::mat4 model_T;
					glm::vec3 pl_size = glm::vec3(0.02f, 0.02f, 0.02f); //구체 크기 
					model_T = glm::translate(pl_pos) * glm::scale(pl_size);
					glUniformMatrix4fv(m_model_loc, 1, GL_EMISSION, glm::value_ptr(model_T));

					//오브젝트 엠비언트 설정 
					float ambient_factor = glGetUniformLocation(s_program_id, "ambient_factor");
					glUniform1f(ambient_factor, 1.50f);

					glVertexAttrib4f(2, 0.f, 2.f, 2.0f, 1.f);
					if(night_mode)
						DrawSphere();
				}
			}
		}
		
		
	}


	// 바닥 Texture
	{
		glUniform1i(glGetUniformLocation(s_program_id, "flag_texture"), true);

		glm::mat4 T0(1.f); // 단위 행렬
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

	// 잔디 
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
	// 나무
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
	// glutSwapBuffers는 항상 Display 함수 가장 아래 부분에서 한 번만 호출되어야한다.
	glutSwapBuffers();
}


/**
Timer: 지정된 시간 후에 자동으로 호출되는 callback 함수.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node64.html#SECTION000819000000000000000
*/
void Timer(int value)
{
	// Timer 호출 시간 간격을 누적하여, 최초 Timer가 호출된 후부터 현재까지 흘러간 계산한다.
	g_elapsed_time_s += value/1000.f;


	// Turn
	g_car_rotation_y += g_car_angular_speed;

	// Calculate Velocity
	glm::vec3 speed_v = glm::vec3(0.f, 0.f, g_car_speed);
	glm::vec3 velocity = glm::rotateY(speed_v, g_car_rotation_y);	// speed_v 를 y축을 기준으로 g_car_rotation_y 만큼 회전한다.

	// Move
	g_car_position += velocity;


	// glutPostRedisplay는 가능한 빠른 시간 안에 전체 그림을 다시 그릴 것을 시스템에 요청한다.
	// 결과적으로 Display() 함수가 호출 된다.
	glutPostRedisplay();

	// 1/60 초 후에 Timer 함수가 다시 호출되로록 한다.
	// Timer 함수 가 동일한 시간 간격으로 반복 호출되게하여,
	// 애니메이션 효과를 표현할 수 있다
	glutTimerFunc((unsigned int)(1000 / 60), Timer, (1000 / 60));
}




/**
Reshape: 윈도우의 크기가 조정될 때마다 자동으로 호출되는 callback 함수.

@param w, h는 각각 조정된 윈도우의 가로 크기와 세로 크기 (픽셀 단위).
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
Keyboard: 키보드 입력이 있을 때마다 자동으로 호출되는 함수.
@param key는 눌려진 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void Keyboard(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = -0.03f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.03f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = glm::radians( 2.f );		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = -1 * glm::radians( 2.f );		//  우회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'n':
		night_mode = !night_mode;
		glutPostRedisplay();
		break;

	case 't':	//나무 재생성 함수 호출
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
KeyboardUp: 눌려졌던 키가 놓여질 때마다 자동으로 호출되는 함수.
@param key는 해당 키보드의 문자값.
@param x,y는 현재 마우스 포인터의 좌표값.
ref: https://www.opengl.org/resources/libraries/glut/spec3/node49.html#SECTION00084000000000000000

*/
void KeyboardUp(unsigned char key, int x, int y)
{
	switch (key)						
	{
	case 's':
		g_car_speed = 0.f;		// 후진 속도 설정
		glutPostRedisplay();
		break;

	case 'w':
		g_car_speed = 0.f;		// 전진 속도 설정
		glutPostRedisplay();
		break;

	case 'a':
		g_car_angular_speed = 0.f;		// 좌회전 각속도 설정
		glutPostRedisplay();
		break;

	case 'd':
		g_car_angular_speed = 0.f;		//  우회전 각속도 설정
		glutPostRedisplay();
		break;

	}

}



/**
Mouse: 마우스 버튼이 입력될 때마다 자동으로 호출되는 함수.
파라메터의 의미는 다음과 같다.
@param button: 사용된 버튼의 종류
  GLUT_LEFT_BUTTON - 왼쪽 버튼
  GLUT_RIGHT_BUTTON - 오른쪽 버튼
  GLUT_MIDDLE_BUTTON - 가운데 버튼 (휠이 눌러졌을 때)
  3 - 마우스 휠 (휠이 위로 돌아 갔음).
  4 - 마우스 휠 (휠이 아래로 돌아 갔음).
@param state: 조작 상태
  GLUT_DOWN - 눌러 졌음
  GLUT_UP - 놓여졌음
@param x,y: 조작이 일어났을 때, 마우스 포인터의 좌표값.
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
MouseMotion: 마우스 포인터가 움직일 때마다 자동으로 호출되는 함수.
@prarm x,y는 현재 마우스 포인터의 좌표값을 나타낸다.
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


