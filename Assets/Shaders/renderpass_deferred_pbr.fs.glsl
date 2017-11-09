
#version 330 core
out vec4 fragColor;

in vec2 uv;
uniform sampler2D passes[7];

// ibl
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

// lights
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];

// transform
uniform mat4 projection;
uniform vec3 cameraPos;

// parameters
const float PI = 3.14159265359;

// brdf
float normalDistributionGGX(vec3 n, vec3 h, float roughness);
float geometrySchlickGGX(float ndotv, float roughness);
float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness);
vec3 fresnelSchlick(float cosTheta, vec3 f0);
vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness);


void main() {
	// passes
	vec3 albedo = texture(passes[0], uv).rgb;
	vec3 n = texture(passes[1], uv).rgb;
	vec3 mrc = texture(passes[2], uv).rgb;
	vec3 position = texture(passes[3], uv).rgb;
	vec3 alpha = texture(passes[4], uv).rgb;

	float metallic = mrc.r;
	float roughness = mrc.g;
	float cavity = mrc.b;

	vec3 v = normalize(cameraPos - position);
	vec3 r = reflect(-v, n);

	vec3 f0 = vec3(0.04);
	f0 = mix(f0, albedo, vec3(metallic));

	// real time lights reflectance equation
	vec3 lightReflection = vec3(0.0);
	for(int i = 0; i < 4; ++i) {
		// calculate per-light radiance
		vec3 l = normalize(lightPositions[i] - position);
		vec3 h = normalize(v + l);
		float distance = length(lightPositions[i] - position);
		float attenuation = 1.0 / (distance * distance);
		vec3 radiance = lightColors[i] * attenuation;

		// Cook-Torrance BRDF
		float specularD = normalDistributionGGX(n, h, roughness);
		float specularG = geometrySmith(n, v, l, roughness);
		vec3 specularF = fresnelSchlick(max(dot(h, v), 0.0), f0);
		vec3 nominator = specularD * specularG * specularF;
		float denominator = 4 * max(dot(n, v), 0.0) * max(dot(n, l), 0.0) + 0.001; // 0.001 to prevent divide by zero.
		vec3 specular = nominator / denominator;

		vec3 diffuseF = vec3(1.0) - specularF;
		diffuseF *= 1.0 - metallic;

		// scale light by normal
		float ndotl = max(dot(n, l), 0.0);

		// add to outgoing radiance
		lightReflection += (diffuseF * albedo / PI + specular) * radiance * ndotl;
	}

	// ibl ambient lighting
	vec3 specularF = fresnelSchlickRoughness(max(dot(n, v), 0.0), f0, roughness);
	vec3 diffuseF = 1.0 - specularF;
	diffuseF *= 1.0 - metallic;
	// ibl diffuse
	vec3 irradiance = texture(irradianceMap, n).rgb;
	vec3 diffuse = irradiance * albedo;
	// ibl specular
	const float MAX_REFLECTION_LOD = 4.0;
	vec3 prefilteredColor = textureLod(prefilterMap, r,  roughness * MAX_REFLECTION_LOD).rgb;
	vec2 brdf  = texture(brdfLUT, vec2(max(dot(n, v), 0.0), roughness)).rg;
	vec3 specular = prefilteredColor * (specularF * brdf.x + brdf.y);

	// ambient
	vec3 ambient = diffuseF * diffuse + specularF * specular * diffuse;

	// vec3 color = ambient + lightReflection;
	vec3 color = lightReflection;

	// exposion
	color *= 5 * cavity;

	// hdr tonemapping
	color = color / (color + vec3(1.0));
	// gamma correct
	color = pow(color, vec3(1.0/2.2));

	fragColor = vec4(mix(vec3(0), color, alpha), 1.0);
}


// normal distribution function (NDF), GGX/Trowbridge-Reitz
float normalDistributionGGX(vec3 n, vec3 h, float roughness) {
	float a = roughness * roughness;
	float a2 = a * a;
	float ndoth = max(dot(n, h), 0.0);
	float ndoth2 = ndoth * ndoth;

	float nominator = a2;
	float denominator = (ndoth2 * (a2 - 1.0) + 1.0);
	denominator = PI * denominator * denominator;

	return nominator / denominator;
}


// specular geometric attenuation, Schlick model
float geometrySmith(vec3 n, vec3 v, vec3 l, float roughness) {
	float ndotv = max(dot(n, v), 0.0);
	float ndotl = max(dot(n, l), 0.0);
	float ggx1 = geometrySchlickGGX(ndotv, roughness);
	float ggx2 = geometrySchlickGGX(ndotl, roughness);

	return ggx1 * ggx2;
}

float geometrySchlickGGX(float ndotv, float roughness) {
	// k direct
	// float a = (roughness + 1.0);
	// float k = (a * a) / 8.0;
	// k IBL
	float a = roughness;
	float k = (a * a) / 2.0;

	float nominator = ndotv;
	float denominator = ndotv * (1.0 - k) + k;

	return nominator / denominator;
}


// Schlick’s approximation, Spherical Gaussian approximation
vec3 fresnelSchlick(float cosTheta, vec3 f0) {
	return f0 + (1.0 - f0) * pow(1.0 - cosTheta, 5.0);
	// return f0 + (1.0 - f0) * pow(2, (−5.55473 * cosTheta − 6.98316) * cosTheta);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 f0, float roughness) {
	return f0 + (max(vec3(1.0 - roughness), f0) - f0) * pow(1.0 - cosTheta, 5.0);
}