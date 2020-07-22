#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "learnopengl/filesystem.h"
#include "learnopengl/shader.h"
#include "learnopengl/camera.h"
#include "learnopengl/model.h"
#include <iostream>
#include<time.h>

//#define  TIME 0
//#define  TIME0

//#define AUTO
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
void renderQuad();
void renderCube();

// settings
const unsigned int SCR_WIDTH = 1080;
const unsigned int SCR_HEIGHT = 720;

// arrays for many things
enum scene { SPONZA };
enum scene choiceForScene;
string scenesName[1] = { "Sponza" };
string texturesPath[1] = { "resources\\objects\\sponza\\sponza.obj" };

// -61.0013,12.1325,0.706687指向-0.130778,0.267238,0.954715 看上采样效果 大小0.5
// -88.9293,21.1966,-1.75392指向0.997683,-0.00174535,0.0680151 // 整体效果 大小0.1

glm::vec3 cameraPositions[1] = { glm::vec3(21.7514,26.1234,0.11178) }; // 存储相机初始位置信息
glm::vec3 lightDirections[1] = { glm::vec3(1.0,-1.0,0.0) }; // 平行光方向
glm::vec3 lightColors[1] = { glm::vec3(1.0,1.0,1.0) };
glm::mat4 modelMatrixs[1] = { glm::mat4({(0.5,0.0,0.0,0.0),
										 (0.0,0.5,0.0,0.0),
										 (0.0,0.0,0.5,0.0),
										 (0.0,0.0,0.0,1.0)}) };

// lighting informations
float ambientIntensitys[1] = { 1.0f }; // 环境光强度
float diffuseIntensitys[1] = { 0.0f };  // 漫反射强度
float specularIntensitys[1] = { 0.3 };// 镜面反射强度
float specularPowers[1] = { 16.0 }; // 高光指数

// screen information
float lastX = (float)SCR_WIDTH / 2.0;
float lastY = (float)SCR_HEIGHT / 2.0;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

Camera camera(glm::vec3(0.0, 0.0, 0.0));

int main()
{
	// key frame position
	double framePositions[11][6] = { {-252.551,36.8362,-1.04137,0.993778,0.0191973,0.109714},
		{ -145.141,31.5252,-9.92178,0.989199,-0.146083,0.0120855 },
		{-74.3061,21.1966,15.6565,0.746531,0.097583,-0.658155},
		{57.4312,25.4738,-39.4463,-0.0365297,0.0784591,0.996248},
		{136.718,28.53,-38.6472,-0.0279222,-0.00523598,0.999596},
		{214.083,47.6328,-2.35947,-0.975157,-0.209619,0.0716123},
		//转向
				{ 58.0713,105.28,-35.5543,-0.735886,0.483283,-0.474247 },
				{49.0725,108.199,-96.4139,-0.999583,-0.0261776,-0.0122115},
				{180.305,128.733,-94.0738,0.991625,0.125332,0.0311631},
				{ 215.837,119.993,8.23924,-0.0556338,-0.0819388,0.995083},
				{ 168.523,111.784,18.5203,-0.931009,-0.0645318,-0.359247 } };
	std::cout << framePositions[0][0] << endl;

	// glfw: initialize and configure
	// ------------------------------
	//camera.Front = glm::vec3(0.997683, -0.00174535, 0.0680151);
	camera.Front = glm::vec3(4.74073e-06, -0.474089, 0.880477);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	bool isFullScreen = false;
	GLFWmonitor* pMonitor = isFullScreen ? glfwGetPrimaryMonitor() : NULL;

	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "RSSAO", pMonitor, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	//glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// choose the scene
	// -----------------------------------------------------------------------------------
	choiceForScene = SPONZA;
	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader shaderGeometryPass("ssao_geometry.vs", "ssao_geometry.fs");
	Shader shaderDownSamplePass("ssao_downsample.vs", "ssao_downsample.fs");
	Shader shaderSSAO("ssao.vs", "ssao.fs");
	Shader shaderSSAOBlur("ssao_blur.vs", "ssao_blur.fs");
	Shader shaderUpSamplePass("ssao_upsample.vs", "ssao_upsample.fs");
	Shader shaderLightingPass("ssao_lighting.vs", "ssao_lighting.fs");

	// load models
	// -----------
	Model nanosuit(FileSystem::getPath(texturesPath[choiceForScene]));
	//Model nanosuit(FileSystem::getPath("resources\\objects\\room\\room.obj"));
	//Model nanosuit(FileSystem::getPath("resources\\objects\\nanosuit\\nanosuit.obj"));

	// configure g-buffer framebuffer
	// ------------------------------
	unsigned int gBuffer;
	glGenFramebuffers(1, &gBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
	unsigned int gPosition, gNormal, gDiffuse;

	// normal color buffer
	glGenTextures(1, &gNormal);
	glBindTexture(GL_TEXTURE_2D, gNormal);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gNormal, 0);
	// diffuse color buffer
	glGenTextures(1, &gDiffuse);
	glBindTexture(GL_TEXTURE_2D, gDiffuse);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gDiffuse, 0);
	// position color buffer

	// depth
	unsigned int m_depth;
	glGenTextures(1, &m_depth);

	glBindTexture(GL_TEXTURE_2D, m_depth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, SCR_WIDTH, SCR_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_depth, 0);

	// tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
	unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
	glDrawBuffers(2, attachments);
	// finally check if framebuffer is complete
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << " g Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//create downsample pass framebuffer 
	unsigned int downsampleFBO;
	glGenFramebuffers(1, &downsampleFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, downsampleFBO);
	unsigned int downDepth;
	glGenTextures(1, &downDepth);
	glBindTexture(GL_TEXTURE_2D, downDepth);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCR_WIDTH / 2, SCR_HEIGHT / 2, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, downDepth, 0);

	unsigned int attachment1[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment1);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << " DOWNSSAO Framebuffer not complete!" << std::endl;

	// also create framebuffer to hold SSAO processing stage 
	// -----------------------------------------------------
	unsigned int ssaoFBO, ssaoBlurXFBO;
	glGenFramebuffers(1, &ssaoFBO);
	glGenFramebuffers(1, &ssaoBlurXFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
	unsigned int ssaoColorBuffer;
	// SSAO color buffer
	glGenTextures(1, &ssaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCR_WIDTH / 2, SCR_HEIGHT / 2, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBuffer, 0);
	unsigned int attachment2[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment2);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Framebuffer not complete!" << std::endl;

	// and blur x stage
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurXFBO);
	unsigned int ssaoColorBufferBlurX;
	glGenTextures(1, &ssaoColorBufferBlurX);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlurX);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCR_WIDTH / 2, SCR_HEIGHT / 2, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlurX, 0);
	unsigned int attachment3[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment3);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//blur y stage
	unsigned int ssaoBlurYFBO;
	glGenFramebuffers(1, &ssaoBlurYFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurYFBO);
	unsigned int ssaoColorBufferBlurXY;
	glGenTextures(1, &ssaoColorBufferBlurXY);
	glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlurXY);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCR_WIDTH / 2, SCR_HEIGHT / 2, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, ssaoColorBufferBlurXY, 0);
	unsigned int attachment4[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment4);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "SSAO Blur Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// upample stage
	unsigned int upsampleFBO;
	glGenFramebuffers(1, &upsampleFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, upsampleFBO);
	unsigned int fullSsaoColorBuffer;
	glGenTextures(1, &fullSsaoColorBuffer);
	glBindTexture(GL_TEXTURE_2D, fullSsaoColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RED, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fullSsaoColorBuffer, 0);
	unsigned int attachment5[1] = { GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(1, attachment5);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		std::cout << "upsampleFBO  Framebuffer not complete!" << std::endl;
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// camera information
	// -----------------------------------------------------------------------------------
	camera.SetPosition(cameraPositions[choiceForScene]);
	float nearD = 0.5f;
	float farD = 1000.0f;
	glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, nearD, farD);

	glm::vec2 DepthUnpackConsts;
	DepthUnpackConsts = glm::vec2(projection[3][2], projection[2][2]);
	glm::vec2 CameraTanHalfFOV;
	CameraTanHalfFOV = glm::vec2(1.0 / projection[0][0], 1.0 / projection[1][1]);
	glm::vec2 PixelSize = glm::vec2(1.0 / (SCR_WIDTH - 1), 1.0 / (SCR_HEIGHT - 1));

	shaderGeometryPass.use();
	//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 220.0f);
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.2f));
	shaderGeometryPass.setMat4("projection", projection);
	shaderGeometryPass.setMat4("model", model);
	shaderGeometryPass.setVec2("DepthUnpackConsts", DepthUnpackConsts);
	shaderGeometryPass.setVec2("CameraTanHalfFOV", CameraTanHalfFOV);
	shaderGeometryPass.setVec2("PixelSize", PixelSize);


	int number = nanosuit.meshes.size();
	for (int i = 0; i < number; ++i)
	{
		(nanosuit.meshes[i]).SetShaderID(shaderGeometryPass.ID);
	}
	//ssao configuration
	// ----------------------------------------------------------------------------------
	glm::vec2 pixelSize = glm::vec2((2.0f) / (float)SCR_WIDTH, (2.0f) / (float)SCR_HEIGHT);
	float ssaoRadius_world = 2.0f;
	float ssaoMaxRadius_screen = 0.1f;
	float ssaoContrast = 5.0f;
	float ssaoaspect = (float)SCR_WIDTH / (float)SCR_HEIGHT;

	// shader configuration
	// --------------------
	shaderDownSamplePass.use();
	shaderDownSamplePass.setInt("depth", 0);
	shaderDownSamplePass.setVec2("DepthUnpackConsts", DepthUnpackConsts);
	shaderDownSamplePass.setVec2("PixelSize", 2.0 / SCR_WIDTH, 2.0 / SCR_HEIGHT);

	shaderSSAO.use();
	shaderSSAO.setInt("gNormal", 0);
	shaderSSAO.setInt("downDepth", 1);
	shaderSSAO.setFloat("ssaoRadius_world", ssaoRadius_world);
	shaderSSAO.setFloat("ssaoMaxRadius_screen", ssaoMaxRadius_screen);
	shaderSSAO.setFloat("ssaoContrast", ssaoContrast);
	shaderSSAO.setFloat("ssaoaspect", ssaoaspect);
	shaderSSAO.setFloat("screen_width", (float)SCR_WIDTH);
	shaderSSAO.setFloat("screen_hight", (float)SCR_HEIGHT);
	shaderSSAO.setVec2("CameraTanHalfFOV", CameraTanHalfFOV);
	shaderSSAO.setVec2("PixelSize", 2.0 / SCR_WIDTH, 2.0 / SCR_HEIGHT);

	shaderSSAOBlur.use();
	shaderSSAOBlur.setInt("ssaoInput", 0);
	shaderSSAOBlur.setInt("downDepth", 1);

	shaderUpSamplePass.use();;
	shaderUpSamplePass.setInt("ssaoInput", 0);
	shaderUpSamplePass.setInt("Depth", 1);
	shaderUpSamplePass.setInt("downDepth", 2);
	pixelSize = glm::vec2((2.0f) / (float)SCR_WIDTH, (2.0f) / (float)SCR_HEIGHT);
	shaderUpSamplePass.setVec2("pixelSize", pixelSize);
	shaderUpSamplePass.setVec2("DepthUnpackConsts", DepthUnpackConsts);

	shaderLightingPass.use();
	shaderLightingPass.setInt("gNormal", 0);
	shaderLightingPass.setInt("gDiffuse", 1);
	shaderLightingPass.setInt("ssao", 2);
	shaderLightingPass.setInt("depth", 3);

	shaderLightingPass.setVec3("uDirectionalLight.Base.Color", lightColors[choiceForScene]);
	shaderLightingPass.setFloat("uDirectionalLight.Base.AmbientIntensity", ambientIntensitys[choiceForScene]);
	shaderLightingPass.setFloat("uDirectionalLight.Base.DiffuseIntensity", diffuseIntensitys[choiceForScene]);
	shaderLightingPass.setFloat("uSpecularIntensity", specularIntensitys[choiceForScene]);
	shaderLightingPass.setFloat("uSpecularPower", specularPowers[choiceForScene]);
	shaderLightingPass.setVec2("DepthUnpackConsts", DepthUnpackConsts);
	shaderLightingPass.setVec2("CameraTanHalfFOV", CameraTanHalfFOV);

	double Totaltime[1000];
	int i = 0;
	int j = -1;
	// render loop
	// -----------
	clock_t start, ends;

	int frameStart = 0;
	int frameStage = 0;
	double linear = 0.0;
	double tempFrame[6] = { 0.0 };

	int s = 0;
	//std::cin >> s;
	while (!glfwWindowShouldClose(window))
	{

#ifdef TIME
		start = clock();
#endif
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// input
		// -----
		processInput(window);

		// render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// 1. geometry pass: render scene's geometry/color data into gbuffer
		// -----------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

		shaderGeometryPass.use();
#ifdef AUTO
		{
			frameStage = frameStart / 200;
			linear = (double)(frameStart % 200) / 200.0;

			for (int w = 0; w < 6; w++)
			{
				tempFrame[w] = (1.0 - linear)*framePositions[frameStage][w] + linear * framePositions[frameStage + 1][w];
			}
			camera.Position = glm::vec3(tempFrame[0], tempFrame[1], tempFrame[2]);
			camera.Front = glm::vec3(tempFrame[3], tempFrame[4], tempFrame[5]);
			frameStart++;
			if (frameStart >= 2000)
				frameStart = 1999;
		}
#endif
		glm::mat4 view = camera.GetViewMatrix();
		shaderGeometryPass.setMat4("view", view);
		//shaderGeometryPass.setInt("")
		// nanosuit model 
		//model = modelMatrixs[choiceForScene];
		//model = glm::translate(model, glm::vec3(0.0f, 0.0f, 5.0));
		//model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0, 1.0, 0.0));
		nanosuit.Draw(shaderGeometryPass);

		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef TIME
		glFinish();
		ends = clock();
		std::cout << "绘制阶段： " << ends - start << endl;
#endif

#ifdef TIME
		start = clock();
#endif

		double starTime = 0;
		double endTime = 0;
#ifdef TIME0
		glFinish();
		starTime = glfwGetTime();
#endif
		//2 down the gPosition texture
		glBindFramebuffer(GL_FRAMEBUFFER, downsampleFBO);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH / 2, SCR_HEIGHT / 2);

		shaderDownSamplePass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, m_depth);
		renderQuad();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef TIME
		//glFinish();
		ends = clock();
		std::cout << "下采样阶段： " << ends - start << endl;
#endif
#ifdef TIME
		glFinish();
		ends = clock();
		std::cout << "上采样阶段： " << ends - start << endl;
#endif


#ifdef TIME
		start = clock();
#endif

		// 3. generate SSAO texture
		// ------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoFBO);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderSSAO.use();
		//shaderSSAO.setMat4("projection", projection);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, downDepth);
		renderQuad();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);


#ifdef TIME
		//glFinish();
		ends = clock();
		std::cout << "ssao阶段： " << ends - start << endl;
#endif



#ifdef TIME
		start = clock();
#endif
		// 4. blur SSAO texture to remove noise
		// x ------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurXFBO);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderSSAOBlur.use();
		glm::vec2 pixelOffset = glm::vec2(2.0f / (float)SCR_WIDTH, 0.0f);
		shaderSSAOBlur.setVec2("pixelOffset", pixelOffset);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBuffer);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, downDepth);
		renderQuad();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glFlush();
		//ends = clock();


		//4 blur the ssao form the y  after processing with x
		// ----------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, ssaoBlurYFBO);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		shaderSSAOBlur.use();
		pixelOffset = glm::vec2(0.0f, 2.0f / (float)SCR_HEIGHT);
		shaderSSAOBlur.setVec2("pixelOffset", pixelOffset);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlurX);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, downDepth);
		renderQuad();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);

#ifdef TIME
		//glFinish();
		ends = clock();
		std::cout << "模糊阶段： " << ends - start << endl;
#endif

#ifdef TIME
		start = clock();
#endif

		// 5 upsample the ssao texture
		// -------------------------------------------------------------------------------------
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, upsampleFBO);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		shaderUpSamplePass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlurXY);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, m_depth);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, downDepth);
		renderQuad();
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
#ifdef TIME0
		glFinish();
		endTime = glfwGetTime();
		//std::cout << "时间： " << endTime - starTime << endl;
		j++;
		if (j >= 100)
		{
			Totaltime[i] = endTime - starTime;
			std::cout << Totaltime[i] * 1000.0 << endl;
			i++;
			if (i == 100)
			{
				int d;
				std::cin >> d;
			}
		}
		if (i == 1000)
		{
			double sum = 0.0;
			for (int k = 0; k < 1000; ++k)
			{
				sum += Totaltime[k];
			}
			std::cout << "时间 " << sum / 1000 << endl;
			int d;
			std::cin >> d;
		}
#endif




#ifdef TIME
		start = clock();
#endif
		// 6. lighting pass
		// --------------------------------------------------------------------------------------
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		shaderLightingPass.use();
		// Update camera location
		glm::mat3 lightlMatrix = glm::transpose(glm::inverse(glm::mat3(model*view)));
		glm::vec3 direction = glm::normalize(lightlMatrix*lightDirections[choiceForScene]);
		shaderLightingPass.setVec3("uDirectionalLight.Direction", direction);
		shaderLightingPass.setVec3("vCameraPosition", camera.Position);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, gNormal);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, gDiffuse);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, fullSsaoColorBuffer);
		//glBindTexture(GL_TEXTURE_2D, ssaoColorBufferBlurXY);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, m_depth);
		renderQuad();
#ifdef TIME
		glFinish();
		ends = clock();
		std::cout << "光照阶段： " << ends - start << endl;
#endif
		std::cout << camera.Position.x << "," << camera.Position.y << "," << camera.Position.z << "指向" << camera.Front.x << "," << camera.Front.y << "," << camera.Front.z << endl;
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
#ifdef TIME
		start = clock();
#endif
		glfwSwapBuffers(window);
		glFinish();
#ifdef TIME
		ends = clock();
		std::cout << "交换缓存阶段： " << ends - start << endl;
#endif

		glfwPollEvents();

	}

	glfwTerminate();

	return 0;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube()
{
	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
																  // front face
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																  1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
																  -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
																  -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
																														// left face
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																														-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
																														-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																														-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
																																											  // right face
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
																																											  1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
																																											  1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
																																											  1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
																																																								   // bottom face
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																								   1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
																																																								   -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
																																																								   -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
																																																																						 // top face
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
																																																																						 1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
																																																																						 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
																																																																						 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
	if (quadVAO == 0)
	{
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		// setup plane VAO
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}
	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}
