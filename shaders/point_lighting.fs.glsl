#version 330 core

layout(location = 0) out vec4 FragColor;

// Flat shading in four steps
#define STEP_A 0.4
#define STEP_B 0.6
#define STEP_C 0.8
#define STEP_D 1.0

in vec3 FragPos;  
in vec2 TexCoords;

uniform sampler2D diffuseMap;
uniform sampler2D normalMap;

uniform vec2 LightPos;
// uniform vec3 ViewPos; 
uniform vec2 Resolution;      
uniform vec4 diffuse_c;      
uniform vec4 ambient_c;    
uniform vec3 Falloff;         
uniform float LightSize;         

void main()
{
	// get texel
	vec4 tex_color = texture(diffuseMap, TexCoords);
	
	// get normals
	vec3 normals = texture(normalMap, TexCoords).rgb;

	// compute light dir
	float depth = 0.05;
	vec3 LightDir = vec3(LightPos - (FragPos.xy / Resolution), depth);
	
	// fixed light size 
	LightDir.x /= (LightSize / Resolution.x);
	LightDir.y /= (LightSize / Resolution.y);

	// dist for attenuation
	float D = length(LightDir);
	
	// normalize
	vec3 N = normalize(normals * 2.0 - 1.0);
	vec3 L = normalize(LightDir);

	// reduce intensity of the normal map 
	// N = mix(N, vec3(0), 0.5);

	// compute bounce vec
	float df = max(dot(N, L), 0.0);

	// compute full diffuse 
	vec3 Diffuse = (diffuse_c.rgb * diffuse_c.a) * df;

	// ambient 
	vec3 Ambient = ambient_c.rgb * ambient_c.a;
	
	// calculate attenuation
	float Attenuation = 1.0 / ( Falloff.x + (Falloff.y*D) + (Falloff.z*D*D) );

	// Toon steps
	// if (Attenuation < STEP_A) 
	// 	Attenuation = 0.0;
	// else if (Attenuation < STEP_B) 
	// 	Attenuation = STEP_B;
	// else if (Attenuation < STEP_C) 
	// 	Attenuation = STEP_C;
	// else 
	// 	Attenuation = STEP_D;

	Attenuation = smoothstep(STEP_A, STEP_D, Attenuation);

	// combine
	vec3 Intensity = Ambient + Diffuse * Attenuation;
	vec3 FinalColor = tex_color.rgb * Intensity;

	FragColor = vec4(FinalColor, tex_color.a);

    // // specular
    // vec3 viewDir = normalize(TangentViewPos - TangentFragPos);
    // vec3 reflectDir = reflect(-lightDir, normal);
    // vec3 halfwayDir = normalize(lightDir + viewDir);  
    // float spec = pow(max(dot(normal, halfwayDir), 0.0), 32.0);
	// vec3 specular = vec3(0.2) * spec;


}