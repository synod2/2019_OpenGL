#version 330
				
in vec4 fs_color;
in vec3 fs_normal;
in vec3 fs_eye_dir;
in vec2 fs_uv;

layout (location = 0) out vec4 color;

struct Light
{
	// 0: Directionl Light
	// 1: Point Light
	// 2: Spot Light
	int type;


	vec3 dir;		
	vec3 position;
	vec3 intensity; // I_l
	vec3 color;
	vec3 K_s; // set the spectular by light
	float cos_cutoff;
};

uniform sampler2D tex0;
uniform bool flag_texture;		//UV for using texture

uniform int num_of_lights;		// 사용될 빛의 개수	
uniform Light lights[100];		// 최대 사용가능 빛의 개수를 100로 한정.


float I_a = 1.0;				  // ambient light intensity

uniform float ambient_factor = 0.05f ;
uniform float shininess_n = 100.f;	
//uniform vec3 K_s;
uniform int night_mode = 0;

void main()
{
	if ( night_mode == 1)
	{	
	
		vec3 K_a = ambient_factor * fs_color.rgb;
		vec3 K_d = fs_color.rgb;
	
		if (flag_texture)
		{
			K_a = ambient_factor * texture(tex0, fs_uv).rgb;
			K_d = texture(tex0, fs_uv).rgb;
		}
		else
		{
			K_a = ambient_factor * fs_color.rgb;
			K_d = fs_color.rgb;
		}

		vec3 ambient = I_a * K_a;
		vec3 diffuse = vec3(0.f, 0.f, 0.f);
		vec3 specular = vec3(0.f, 0.f, 0.f);

		for ( int i=0; i<num_of_lights; i++ )
		{
		
			// Directional Light
			if ( lights[i].type == 0 )
			{
				vec3 L = normalize(-lights[i].dir);
				vec3 N = normalize(fs_normal);
				vec3 V = normalize(-fs_eye_dir);
				vec3 H = (L+V)/length(L+V);

				diffuse += lights[i].intensity * K_d * max(0.f, dot(L, N));
				specular += lights[i].intensity * lights[i].K_s * pow(max(0.f, dot(N, H)), shininess_n);
			}

			// Point Light
			else if ( lights[i].type == 1 )
			{
				vec3 L = normalize(lights[i].position-fs_eye_dir);
				vec3 N = normalize(fs_normal);
				vec3 V = normalize(-fs_eye_dir);
				vec3 H = (L+V)/length(L+V);
				float d =  length(lights[i].position-fs_eye_dir);

				vec3 I_l = lights[i].intensity;
				if ( d > 0.001f )
					I_l = lights[i].intensity * min(1.0f/(100*d*d), 1.0f);	
				
				K_d = lights[i].color;

				diffuse += I_l * K_d * max(0.f, dot(L, N));
				specular += I_l * lights[i].K_s * pow(max(0.f, dot(N, H)), shininess_n);
			}

			// Spot Light
			else if ( lights[i].type == 2 )
			{
				vec3 L = normalize(lights[i].position-fs_eye_dir);
				vec3 N = normalize(fs_normal);
				vec3 V = normalize(-fs_eye_dir);
				vec3 H = (L+V)/length(L+V);
				vec3 Sd = normalize(-lights[i].dir);

				if ( dot(Sd,L) >= lights[i].cos_cutoff )
				{
					diffuse += lights[i].intensity * K_d * max(0.f, dot(L, N));
					specular += lights[i].intensity * lights[i].K_s * pow(max(0.f, dot(N, H)), shininess_n);
				}
			}

		
		}




		color.rgb = ambient + diffuse + specular;
		color.rgb = min(vec3(1.f, 1.f, 1.f), color.rgb);
		color.a = fs_color.a;
	}
	else if ( night_mode == 0 )
	{
		vec3 L = normalize(-lights[0].dir);
		vec3 N = normalize(fs_normal);
		vec3 V = normalize(-fs_eye_dir);
		vec3 H = (L+V)/length(L+V);

		vec3 K_a = ambient_factor * fs_color.rgb;
		vec3 K_d = fs_color.rgb;

		if (flag_texture)
		{
			K_a = ambient_factor * texture(tex0, fs_uv).rgb;
			K_d = texture(tex0, fs_uv).rgb;
		}
		else
		{
			K_a = ambient_factor * fs_color.rgb;
			K_d = fs_color.rgb;
		}


		vec3 ambient = I_a * K_a;		
		vec3 diffuse = vec3(0.f, 0.f, 0.f);
		vec3 specular = vec3(0.f, 0.f, 0.f);

		diffuse += lights[0].intensity * K_d * max(0.f, dot(L, N));
		specular += lights[0].intensity * lights[0].K_s * pow(max(0.f, dot(N, H)), shininess_n);

		color.rgb = ambient + diffuse + specular;
		color.rgb = min(vec3(1.f, 1.f, 1.f), color.rgb);
		color.a = fs_color.a;
		
	}
}

