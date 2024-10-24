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


uniform sampler2D normalMap2;
uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;
uniform sampler2D texmap3;
uniform sampler2D texmap4;
uniform sampler2D texmap5;
uniform samplerCube cubeMap;
uniform PointLight pointLights[6];
uniform SpotLight spotLights[2];
uniform DirectionalLight dirLight; 
uniform int pointON;
uniform int dirON;
uniform int spotON;
uniform int fogON;
uniform int texMode;
uniform int diffMapCount;

uniform Materials mat;


uniform sampler2D texUnitDiff;
uniform sampler2D texUnitSpec;
uniform sampler2D texUnitDiff1;
uniform sampler2D texUnitNormalMap;
uniform bool normalMap;
uniform bool specularMap;

vec4 diff, auxSpec;

in Data {
	vec3 normal;
	vec3 eye;
	vec3 lightDir;
	vec2 tex_coord;
	vec3 skyboxtex_coord;
} DataIn;

void main() {
	vec4 texel, texel1;
	vec4 result = vec4(0.0);
	vec4 spec = vec4(0.0);

	// Directional Light
	vec3 n;

	if (normalMap) {
		n= normalize(2.0*texture(texUnitNormalMap, DataIn.tex_coord).rgb - 1.0);
	}else{
		n= normalize(DataIn.normal); }

	if(texMode == 5)  // lookup normal from normal map, move from [0,1] to [-1, 1] range, normalize
		n = normalize(2.0 * texture(normalMap2, DataIn.tex_coord).rgb - 1.0);
	else
		n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);
	vec3 l = normalize(vec3(dirLight.direction));
	float intensity=0;



	if(mat.texCount==0){
		diff =mat.diffuse;
		auxSpec= mat.specular;
	}
	else{
		if(diffMapCount==0){
			diff=mat.diffuse;
		}
		else if(diffMapCount==1){
			diff=mat.diffuse*texture(texUnitDiff, DataIn.tex_coord);
		}
		else{
			diff=mat.diffuse*texture(texUnitDiff, DataIn.tex_coord)*texture(texUnitDiff1, DataIn.tex_coord);
		}
		if (specularMap){
			auxSpec=mat.specular*texture(texUnitSpec, DataIn.tex_coord);
		}
		else{
			auxSpec=mat.specular;
		}
	}
	

	 if (dirON == 1) {
		intensity = max(dot(n,l), 0.0);
		if (intensity > 0.0) {
			vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = auxSpec * pow(intSpec, mat.shininess);
		}
			result += intensity * diff + spec;

	}
	// Point Lights
	if ( pointON == 1){
		for (int i = 0; i < 6; i++) {
			l = normalize(vec3(pointLights[i].position) + DataIn.eye);
			intensity = max(dot(n,l), 0.0);
			if (intensity > 0.0) {
				vec3 h = normalize(l + e);
				float intSpec = max(dot(h,n), 0.0);
				spec = auxSpec * pow(intSpec, mat.shininess);
			}
			result += intensity * diff + spec;
		}
	}

	
	//Spot Lights
	
	if (spotON == 1){
        for (int i = 0; i < 2; i++) {
            l = normalize(vec3(spotLights[i].position) + DataIn.eye);
            //only if the angle is smaller than the spot angle
            if(cos(radians(spotLights[i].angle)) < dot(l, normalize(vec3(spotLights[i].direction)))) {
				intensity = max(dot(n,l), 0.5);
                if (intensity > 0.0) {
                    vec3 h = normalize(l + e);
                    float intSpec = max(dot(h,n), 0.0);
                    spec = auxSpec * pow(intSpec, mat.shininess);
                }
				result += intensity * diff + spec;
            }
            
        }
    }
	
	//fog factor
    float dist = 0;
    vec3 final_color;
    if (fogON == 0) {
        colorOut = max(vec4(result.rgb, diff.a), mat.ambient);
    }
    else {
        dist = length(-DataIn.eye);
        float fogAmount = exp( -dist*0.05 );
        //clamp(fogAmount, 0, 1.0);
        vec3 fogColor = vec3(0.5,0.6,0.7);
        final_color = mix(fogColor, vec3(result), fogAmount );
		colorOut = max(vec4(final_color, diff.a), mat.ambient);
    }

    
    if (texMode == 1) {
        texel = texture(texmap2, DataIn.tex_coord);
        texel1 = texture(texmap1, DataIn.tex_coord);
        colorOut = max(colorOut + intensity*texel*texel1 + spec, mat.ambient);
		colorOut = vec4(colorOut.rgb, diff.a);
    }
	else if (texMode == 2) {
		texel = texture(texmap3, DataIn.tex_coord); 
		if(texel.a == 0.0) discard;
		else
			colorOut = vec4(max(intensity*texel.rgb + vec3(spec), 0.1*texel.rgb), texel.a);
	} else if (texMode == 3) {
		texel = texture(texmap5, DataIn.tex_coord);  //texel from element flare texture
		if((texel.a == 0.0)  || (diff.a == 0.0) ) discard;
		else
			colorOut = diff * texel;
	} else if (texMode == 4) {
		texel = texture(texmap4, DataIn.tex_coord);
		if((texel.a == 0.0)  || (diff.a == 0.0) ) discard;
		else
			colorOut = diff * texel;
	} else if (texMode == 6) { //SkyBox
		colorOut = texture(cubeMap, DataIn.skyboxtex_coord);
	} else if (texMode == 7 || texMode == 5) { //Normal Map
		texel = texture(texmap0, DataIn.tex_coord); 
		if(texel.a == 0.0) discard;
		else
			colorOut = vec4(max(intensity*texel.rgb + vec3(spec), 0.1*texel.rgb), texel.a);

	} 

}