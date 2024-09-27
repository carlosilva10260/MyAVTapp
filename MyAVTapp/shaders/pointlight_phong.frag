#version 430

out vec4 colorOut;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

struct PointLight {
	vec4 position;
};

struct DirectionalLight {
	vec4 direction;
};

struct SpotLight {
	vec4 position;
	float angle;
	vec4 direction;
};



uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform PointLight pointLights[2];
uniform SpotLight spotLights[2];
uniform DirectionalLight dirLight; 
uniform int pointON;
uniform int dirON;
uniform int spotON;

uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
} DataIn;

void main() {
	vec4 result = vec4(0.0);
	vec4 spec = vec4(0.0);


	// Directional Light
	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);
	vec3 l = normalize(vec3(dirLight.direction));
	float intensity = max(dot(n,l), 0.0);
	

	 if (dirON == 1) {
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
		}
			result += intensity * mat.diffuse + spec;

	}
	// Point Lights
	if ( pointON == 1){
		for (int i = 0; i < 2; i++) {
			l = normalize(vec3(pointLights[i].position) + DataIn.eye);
			float intensity = max(dot(n,l), 0.0);
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = mat.specular * pow(intSpec, mat.shininess);
			}
			result += intensity * mat.diffuse + spec;
		}
	}
	


	colorOut = max(result, mat.ambient);
}