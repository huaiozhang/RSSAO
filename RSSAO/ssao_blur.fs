#version 330 core
out float FragColor;

in vec2 TexCoords;

uniform sampler2D ssaoInput;
uniform sampler2D downDepth;
uniform vec2 pixelOffset; 
 float gaussWeightsSigma1[7] = float[7]
(
	0.00598f,
	0.060626f,
	0.241843f,
	0.383103f,
	0.241843f,
	0.060626f,
	0.00598f
);

 float gaussWeightsSigma3[7] = float[7]
(
	0.106595f,
	0.140367f,
	0.165569f,
	0.174938f,
	0.165569f,
	0.140367f,
	0.106595f
);



void main() 
{   
	float sum = 0.0f;
	float weightSum = 0.0f;

	//float depth = texture(downgPosition,TexCoords).z;
	float depth = texture(downDepth,TexCoords).x;

	for (int i = -3; i <= 3; i++)
	{
		vec2 sampleTexCoord = TexCoords + i*pixelOffset;
		//float sampleDepth = texture(downgPosition,sampleTexCoord).z;
		float sampleDepth = texture(downDepth,sampleTexCoord).x;

		float depthsDiff = 0.1f * abs(depth - sampleDepth);
		depthsDiff *= depthsDiff;
		float weight = 1.0f / (depthsDiff + 0.001f);
		weight *= gaussWeightsSigma3[3 + i];

		sum += weight * texture(ssaoInput,sampleTexCoord).r;
		weightSum += weight;
	}

    FragColor = sum /weightSum;
	
}  