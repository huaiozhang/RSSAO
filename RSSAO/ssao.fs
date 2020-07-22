  #version 330 core

#define PI               3.1415f
#define TWO_PI           2.0f*PI
#define GOLDEN_ANGLE     2.4f
#define  SAMPLES_COUNT   15

out float FragColor;
in vec2 TexCoords;
//in vec2 ViewRay;

uniform vec2 CameraTanHalfFOV;
uniform sampler2D gNormal;
uniform sampler2D downDepth;
    
uniform float ssaoRadius_world;
uniform	float ssaoMaxRadius_screen;
uniform	float ssaoContrast;
uniform	float ssaoaspect;
uniform float screen_width;
uniform float screen_hight; 


 vec2  vogelDiskOffsets16[16] = vec2[16]
(
	vec2(0.176777f, 0.0f),
	vec2(-0.225718f, 0.206885f),
	vec2(0.0343507f, -0.393789f),
	vec2(0.284864f, 0.370948f),
	vec2(-0.52232f, -0.0918239f),
	vec2(0.494281f, -0.315336f),
	vec2(-0.164493f, 0.615786f),
	vec2(-0.316681f, -0.607012f),
	vec2(0.685167f, 0.248588f),
	vec2(-0.711557f, 0.295696f),
	vec2(0.341422f, -0.73463f),
	vec2(0.256072f, 0.808194f),
	vec2(-0.766143f, -0.440767f),
	vec2(0.896453f, -0.200303f),
	vec2(-0.544632f, 0.780785f),
	vec2(-0.130341f, -0.975582f)
);

 vec2 alchemySpiralOffsets16[16] = vec2[16]
(
	vec2(0.19509f, 0.980785f),
	vec2(-0.55557f, -0.83147f),
	vec2(0.831469f, 0.555571f),
	vec2(-0.980785f, -0.195091f),
	vec2(0.980785f, -0.19509f),
	vec2(-0.83147f, 0.555569f),
	vec2(0.555571f, -0.831469f),
	vec2(-0.195092f, 0.980785f),
	vec2(-0.195089f, -0.980786f),
	vec2(0.555569f, 0.83147f),
	vec2(-0.831469f, -0.555572f),
	vec2(0.980785f, 0.195092f),
	vec2(-0.980786f, 0.195088f),
	vec2(0.831471f, -0.555568f),
	vec2(-0.555572f, 0.831468f),
	vec2(0.195093f, -0.980785f)
);

vec2 RotatePoint(vec2 pt, float angle)
{
	float sine=0.0f;
	float cosine=0.0f;

	sine=sin(angle);
	cosine=cos(angle);
	
	vec2 rotatedPoint;
	rotatedPoint.x = cosine*pt.x + -sine*pt.y;
	rotatedPoint.y = sine*pt.x + cosine*pt.y;
	
	return rotatedPoint;
}


vec2 VogelDiskOffset(int sampleIndex, float phi)
{
	float r = sqrt(sampleIndex + 0.5f) / sqrt(SAMPLES_COUNT);
	float theta = sampleIndex*GOLDEN_ANGLE + phi;

	float sine=0.0f;
	float cosine=0.0f;

	sine=sin(theta);
	cosine=cos(theta);
	
	return vec2(r * cosine, r * sine);
}


vec2 AlchemySpiralOffset(int sampleIndex, float phi)
{
	float alpha = float(sampleIndex + 0.5f) / SAMPLES_COUNT;
	float theta = 7.0f*TWO_PI*alpha + phi;

	float sine=0.0f;
	float cosine=0.0f;

	sine=sin(theta);
	cosine=cos(theta);
	
	return vec2(cosine, sine);
}


float InterleavedGradientNoise(vec2 position_screen)
{
	vec3 magic = vec3(0.06711056f, 4.0f*0.00583715f, 52.9829189f);
	return fract(magic.z * fract(dot(position_screen, magic.xy)));
}


vec3 positiontrs(vec2 coords)
{
   vec3  position_view;
   vec2 temp= 2.0*coords-vec2(1.0,1.0);
   position_view.z=texture(downDepth,coords).x;
   position_view.x=-position_view.z*CameraTanHalfFOV.x*temp.x;
   position_view.y=-position_view.z*CameraTanHalfFOV.y*temp.y;
   return position_view;
}

/*
vec3 positionn(vec2 coords)
{
   vec3  position_view;
   vec2 temp= 2.0*coords-vec2(1.0,1.0);
   position_view.z=texture(downViewSpaceZ,coords).x;
   position_view.x=-position_view.z*CameraTanHalfFOV.x*temp.x;
   position_view.y=-position_view.z*CameraTanHalfFOV.y*temp.y;
   return position_view;
}

*/

void main()
{ 
    float ssaoRadius = ssaoRadius_world;
	ssaoRadius = 1.0;
     // TexCoords = TexCoords+vec2(-0.25,-0.25)*PixelSize;
	vec3 position =positiontrs(TexCoords);
	//vec3 position = texture(downgPosition,TexCoords).xyz;
	//vec3 position =positionn(TexCoords);
	vec3 normal = normalize(texture(gNormal,TexCoords).rgb);
	
	vec2 screen_position = TexCoords*vec2(screen_width,screen_hight);
	float noise = InterleavedGradientNoise(screen_position);

	vec2 radius_screen=vec2(ssaoRadius/position.z,ssaoRadius/position.z);
	radius_screen.x=min(radius_screen.x, ssaoMaxRadius_screen);
	radius_screen.y=min(radius_screen.y, ssaoMaxRadius_screen);
	radius_screen.y *= ssaoaspect;
	
	float ao = 0.0f;

    for (int i = 0; i < SAMPLES_COUNT; i++)
	{
		vec2 sampleOffset = vec2(0.0f,0.0f);
		sampleOffset = VogelDiskOffset(i, TWO_PI*noise);
		vec2 sampleTexCoord = TexCoords + radius_screen*sampleOffset;
       //vec3 samplePosition = texture(downgPosition,sampleTexCoord).xyz;
		vec3 samplePosition=positiontrs(sampleTexCoord);
		//vec3 samplePosition=positionn(sampleTexCoord);
		vec3 v = samplePosition - position;// 纹理空间不线性，因为x y 都在0 1之间
		ao += max(0.0f, dot(v, normal) + 0.002f*position.z) / (dot(v, v) + 0.001f);
	}

	ao = clamp(ao / SAMPLES_COUNT,0.0f,1.0f);
	ao = 1.0f - ao;
	float temp1 = ssaoContrast;
	temp1=5.0;
	ao = pow(ao, temp1);
    FragColor = ao;
}