#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gNormal;
uniform sampler2D gDiffuse;
uniform sampler2D ssao;
uniform sampler2D depth;

uniform vec2 DepthUnpackConsts;
uniform vec2 CameraTanHalfFOV;

struct SBaseLight
{
	vec3 Color;
	float AmbientIntensity;         //环境光强度
	float DiffuseIntensity;         //漫反射强度
};

struct SDirectionLight
{
	SBaseLight Base;
	vec3 Direction;
};

uniform SDirectionLight uDirectionalLight;

uniform vec3	vCameraPosition;
uniform float	uSpecularIntensity;
uniform float	uSpecularPower;

/*
float depthToZ(float depth)
{
	return  (-projection[3][2])/(2.0*depth-1.0+projection[2][2]);
} */

float ScreenSpaceToViewSpaceDepth( float screenDepth )
{
    float depthLinearizeMul = DepthUnpackConsts.x;
    float depthLinearizeAdd = DepthUnpackConsts.y;

    // Optimised version of "-cameraClipNear / (cameraClipFar - projDepth * (cameraClipFar - cameraClipNear)) * cameraClipFar"

    // Set your depthLinearizeMul and depthLinearizeAdd to:
    // depthLinearizeMul = ( cameraClipFar * cameraClipNear) / ( cameraClipFar - cameraClipNear );
    // depthLinearizeAdd = cameraClipFar / ( cameraClipFar - cameraClipNear );

    return -depthLinearizeMul / ( depthLinearizeAdd +2*screenDepth-1.0);
}

vec3 position(vec2 coords)
{
   vec3  position_view;
   vec2 temp= 2.0*coords-vec2(1.0,1.0);
   position_view.z=texture(depth,coords).x;
   position_view.z = ScreenSpaceToViewSpaceDepth(position_view.z);
   position_view.x=-position_view.z*CameraTanHalfFOV.x*temp.x;
   position_view.y=-position_view.z*CameraTanHalfFOV.y*temp.y;
   return position_view;
}
/*
vec3 position(float z)
{
	vec3 position_z_view;
	position_z_view.z = z;
	position_z_view.x=parameter.x*z*ssaoaspect;
	position_z_view.y=parameter.y*z;
	return position_z_view;
}*/

vec3 calculateLightInternal(SBaseLight vBaseLight, vec3 vLightDirection, vec3 vWorldPos, vec3 vNormal)
{
	vec3 AmbientColor  = vBaseLight.Color * vBaseLight.AmbientIntensity;

	float ao= texture(ssao, TexCoords).x;
	AmbientColor *= ao;
	vec3 DiffuseColor  = vec3(0.0, 0.0, 0.0);
	//vec3 SpecularColor = vec3(0.0, 0.0, 0.0);

	
	float DiffuseFactor = max(dot(vNormal, -vLightDirection),0.0);
	if (DiffuseFactor >0.0)
	{
		DiffuseColor		= vBaseLight.Color * vBaseLight.DiffuseIntensity * DiffuseFactor;
		//DiffuseColor		= vBaseLight.Color * vBaseLight.DiffuseIntensity ;
	}
	//else{
	  //   DiffuseColor		= vBaseLight.Color * vBaseLight.DiffuseIntensity * DiffuseFactor;
	//}
	
	return (AmbientColor + DiffuseColor);
}

vec3 calculateDirectionalLight(vec3 vWorldPos, vec3 vNormal)
{
	return calculateLightInternal(uDirectionalLight.Base, uDirectionalLight.Direction, vWorldPos, vNormal);
}


void main()
{             
    // retrieve data from gbuffer
	vec2 TexCoord = TexCoords;
	vec3 WorldPos = position(TexCoords);
	//vec3 WorldPos = texture(gPosition,TexCoords).xyz;
	vec3 Normal = texture(gNormal, TexCoord).xyz;

	Normal = normalize(Normal);
	vec4 color = texture(gDiffuse, TexCoord);
   // FragColor = vec4(color.xyz * calculateDirectionalLight(WorldPos, Normal),color.w);
	FragColor = vec4(calculateDirectionalLight(WorldPos, Normal),1.0);
	//FragColor = color;
}