#version 430

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;
uniform mat4 m_Model;   //por causa do cubo para a skybox
uniform mat4 m_View;

uniform vec4 l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec3 tangent, bitangent;
in vec4 texCoord;

uniform bool normalMap;

out Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	vec3 skyboxtex_coord;
} DataOut;

void main () {
	vec3 n, t, b;
	vec3 lightDir, eyeDir;
	vec3 aux;
	
	vec4 pos = m_viewModel * position;


	if (normalMap){
		t = normalize(m_normal * tangent.xyz);
		b = normalize(m_normal * bitangent.xyz);

		aux.x = dot(lightDir, t);
		aux.y = dot(lightDir, b);
		aux.z = dot(lightDir, n);
		lightDir = normalize(aux);

		aux.x = dot(eyeDir, t);
		aux.y = dot(eyeDir, b);
		aux.z = dot(eyeDir, n);
		eyeDir = normalize(aux);
	}

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.lightDir = vec3(l_pos - pos);
	DataOut.eye = vec3(-pos);
	DataOut.tex_coord = texCoord.st;
	DataOut.skyboxtex_coord = vec3(m_Model * position);	//Transformação de modelação do cubo unitário 
	DataOut.skyboxtex_coord.x = - DataOut.skyboxtex_coord.x; //Texturas mapeadas no interior logo negar a coordenada x

	gl_Position = m_pvm * position;	
}