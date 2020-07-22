#version 400 core
out float fullSsao;
in vec2 TexCoords;

uniform sampler2D  ssaoInput;
uniform sampler2D  Depth;
uniform sampler2D  downDepth;
uniform vec2 pixelSize;

uniform vec2 DepthUnpackConsts;


/*
float depthToZ(float depth)
{
	return (-projection[3][2])/(2.0*depth-1.0+projection[2][2]);
}*/

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

void main()
{

	float depth=ScreenSpaceToViewSpaceDepth(texture(Depth,TexCoords).x); 
	//float depth = texture(gPosition,TexCoords).z;
	vec4 depths_x4=textureGather(downDepth,TexCoords,0);// 01 11 10 00
	//vec4 depths_x4=textureGather(downgPosition,TexCoords,2);// 01 11 10 00
	vec4 depthsDiffs = abs(vec4(depth,depth,depth,depth) - depths_x4);
	vec4 ssaos_x4 = textureGather(ssaoInput,TexCoords,0);

	vec2 imageCoord = TexCoords / pixelSize; //ø’º‰”Ú
	vec2 fractional = fract(imageCoord);
	float a = (1.0f - fractional.x) * (1.0f - fractional.y); //11 
	float b = fractional.x * (1.0f - fractional.y);// 01
	float c = (1.0f - fractional.x) * fractional.y;// 10
	float d = fractional.x * fractional.y;// 00

	float ssao = 0.0f;
	float weightsSum = 0.0f;

	float weight00 = a / (depthsDiffs.y + 0.001f);
	ssao += weight00 * ssaos_x4.y;
	weightsSum += weight00;

	float weight10 = b / (depthsDiffs.a + 0.001f);
	ssao += weight10 * ssaos_x4.a;
	weightsSum += weight10;

	float weight01 = c / (depthsDiffs.z + 0.001f);
	ssao += weight01 * ssaos_x4.z;
	weightsSum += weight01;

	float weight11 = d / (depthsDiffs.w + 0.001f);
	ssao += weight11 * ssaos_x4.w;
	weightsSum += weight11;

	ssao /= weightsSum;

	fullSsao=ssao;
}