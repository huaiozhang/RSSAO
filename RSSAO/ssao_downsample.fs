#version 330 core
layout (location =0 ) out float z; 
in vec2 TexCoords;


uniform sampler2D  depth;
uniform vec2 DepthUnpackConsts;
uniform vec2 PixelSize;

float ScreenSpaceToViewSpaceDepth( float screenDepth )
{
    float depthLinearizeMul = DepthUnpackConsts.x;
    float depthLinearizeAdd = DepthUnpackConsts.y;

    return -depthLinearizeMul / ( depthLinearizeAdd +2*screenDepth-1.0);
}

void main()
{    
     z = ScreenSpaceToViewSpaceDepth(texture(depth,TexCoords).x);
}