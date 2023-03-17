#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fPosLightSpace;
in vec4 fPosEye;


out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform mat3 lightDirMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

//spotLight
uniform int enableSpotLight;
float spotQuadratic = 0.02f;
float spotLinear = 0.09f;
float spotConstant = 1.0f;
float shininess = 64.0f;
vec3 spotLightAmbient = vec3(0.0f, 0.0f, 0.0f);
vec3 spotLightSpecular = vec3(1.0f, 1.0f, 1.0f);
vec3 spotLightColor = vec3(12,12,12);
uniform float spotLight1;
uniform float spotLight2;
uniform vec3 spotLightDirection;
uniform vec3 spotLightPosition;

//pointLight
uniform int enablePointLight;
uniform vec3 posPointLight1;


//fog
uniform float fogDensity;

vec3 computeLightComponents()
{		
	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
	
	//transform normal
	vec3 normalEye = normalize(normalMatrix * fNormal);	
	
	//compute light direction
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);	

	//compute view direction 
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	
	//compute half vector
	vec3 halfVector = normalize(lightDirN + viewDirN);
		
	//compute ambient light
	ambient = ambientStrength * lightColor *2.0f;
	
	//compute diffuse light
	diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	
	//compute specular light
	float specCoeff = pow(max(dot(halfVector, normalEye), 0.0f), shininess);
	specular = specularStrength * specCoeff * lightColor;
		
	return (ambient + diffuse + specular);
	
}

//pointLight
vec3 computePointLight(vec4 lightPosEye)
{
	vec3 cameraPosEye = vec3(0.0f);
	vec3 normalEye = normalize(normalMatrix * fNormal);
	vec3 lightDirN = normalize(lightPosEye.xyz - fPosEye.xyz);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 ambient = 0.5 * lightColor;
	vec3 diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;
	vec3 halfVector = normalize(lightDirN + viewDirN);
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32);
	vec3 specular = 0.5 * specCoeff * lightColor;
	float distance = length(lightPosEye.xyz - fPosEye.xyz);
	float att = 1.0f / (1 + 0.00225 * distance + 0.00375 * distance * distance);
	return (ambient + diffuse + specular) * att * vec3(1.0f,1.0f,1.0f);
}

//spotLight
vec3 computeSpotLightComponents() {
	vec3 cameraPosEye = vec3(0.0f);
	vec3 lightDir = normalize(spotLightPosition - fPosition);
	vec3 normalEye = normalize(normalMatrix * fNormal);
	vec3 lightDirN = normalize(lightDirMatrix * lightDir);
	vec3 viewDirN = normalize(cameraPosEye - fPosEye.xyz);
	vec3 halfVector = normalize(lightDirN + viewDirN);

	float diff = max(dot(fNormal, lightDir), 0.0f);
	float spec = pow(max(dot(normalEye, halfVector), 0.0f), shininess);
	float distance = length(spotLightPosition - fPosition);
	float attenuation = 1.0f / (spotConstant + spotLinear * distance + spotQuadratic * distance * distance);

	float theta = dot(lightDir, normalize(-spotLightDirection));
	float epsilon = spotLight1-spotLight2;
	float intensity = clamp((theta - spotLight2)/epsilon, 0.0, 1.0);

	vec3 ambient = spotLightColor * spotLightAmbient * vec3(texture(diffuseTexture, fTexCoords));
	vec3 diffuse = spotLightColor * spotLightSpecular * diff * vec3(texture(diffuseTexture, fTexCoords));
	vec3 specular = spotLightColor * spotLightSpecular * spec * vec3(texture(specularTexture, fTexCoords));
	ambient *= attenuation * intensity;
	diffuse *= attenuation * intensity;
	specular *= attenuation * intensity;
	
	return ambient + diffuse + specular;
}

float computeFog()
{
     float fragmentDistance = length(fPosEye);
     float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

     return clamp(fogFactor, 0.0f, 1.0f);
}

float computeShadow()
{	
	// perform perspective divide
    vec3 normalizedCoords = fPosLightSpace.xyz / fPosLightSpace.w;
    if(normalizedCoords.z > 1.0f)
        return 0.0f;
    
	// Transform to [0,1] range
    normalizedCoords = normalizedCoords * 0.5f + 0.5f;
   
   // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, normalizedCoords.xy).r;    
   
   // Get depth of current fragment from light's perspective
    float currentDepth = normalizedCoords.z;
   
   // Check whether current frag pos is in shadow
    float bias = 0.005f;
    float shadow = currentDepth - bias> closestDepth  ? 1.0f : 0.0f;

    return shadow;	
}


void main() 
{
    vec3 light = computeLightComponents();
    float shadow = computeShadow();

    // modulate with diffuse map
	ambient *= texture(diffuseTexture, fTexCoords).rgb;
	diffuse *= texture(diffuseTexture, fTexCoords).rgb;
	specular *= texture(specularTexture, fTexCoords).rgb;
	

	if(enablePointLight==1){
		vec4 lightPosEye1 = view * vec4(posPointLight1, 1.0f);
		light += computePointLight(lightPosEye1);
		vec4 diffuseColor = texture(diffuseTexture, fTexCoords);
	}

	// spotlight
	if (enableSpotLight == 1){
		light += computeSpotLightComponents();
	}

	//calc fog
	float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0);

	//calc shadow
	vec3 color = min((ambient + (1.0f - shadow)*diffuse) + (1.0f - shadow) * specular, 1.0f);
	vec4 colorWithShadow = vec4(color,1.0f);

	fColor = mix(fogColor, min(colorWithShadow * vec4(light, 1.0f), 1.0f), fogFactor);
}
