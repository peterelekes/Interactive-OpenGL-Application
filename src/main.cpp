#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"

#include <iostream>

// window
gps::Window myWindow;
int retina_width, retina_height;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;


// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;
glm::mat3 lightDirMatrix;
GLuint lightDirMatrixLoc;

glm::mat4 lightRotation;
GLfloat lightAngle;

//light
glm::vec3 lightDir;
glm::vec3 lightColor;

//spotLight
int enableSpotLight = 0;
float spotLight1;
float spotLight2;
glm::vec3 spotLightDirection;
glm::vec3 spotLightPosition;

//pointLight
int enablePointLight = 0;
glm::vec3 posPointLight1;
GLuint posPointLightLoc1;


// camera at eye level
gps::Camera myCamera(
    glm::vec3(2.0f, 5.0f, -10.0f),
    glm::vec3(0.0f, 0.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 0.0f));

GLfloat cameraSpeed = 0.2;

float deltaTime = 0.0f;


bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 1600.0 / 2.0;
float lastY = 900.0 / 2.0;
float fov = 90.0f;
float nearPlane = 0.01f;
float farPlane = 100.00f;

GLboolean pressedKeys[1024];


GLfloat angle;
// models
gps::Model3D teapot;
gps::Model3D mainScene;
gps::Model3D leftGate;
gps::Model3D rightGate;
gps::Model3D lightCube;
gps::Model3D screenQuad;
gps::Model3D audience;
std::vector<glm::mat4> modelMatrices;
gps::Model3D discoBall;
bool startAnimations=false;
float leftGateAngle=0.0f;
float rightGateAngle=0.0f;
float discoBallAngle = 0.0f;
bool leftGateOpening=false;
bool rightGateOpening =false;
std::vector<bool> jumpUp = { 0 };

// shaders
gps::Shader myBasicShader;
gps::Shader skyBoxShader;
gps::Shader depthMapShader;
gps::Shader screenQuadShader;
gps::Shader lightShader;

//Skybox
gps::SkyBox mySkyBox;
std::vector<const GLchar*> faces;

//shadow
GLuint shadowMapFBO;
GLuint depthMapTexture;
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;
bool showDepthMap = false;

//fog
GLfloat fogDensity = 0.01f;




GLenum glCheckError_(const char *file, int line)
{
	GLenum errorCode;
	while ((errorCode = glGetError()) != GL_NO_ERROR) {
		std::string error;
		switch (errorCode) {
            case GL_INVALID_ENUM:
                error = "INVALID_ENUM";
                break;
            case GL_INVALID_VALUE:
                error = "INVALID_VALUE";
                break;
            case GL_INVALID_OPERATION:
                error = "INVALID_OPERATION";
                break;
            case GL_STACK_OVERFLOW:
                error = "STACK_OVERFLOW";
                break;
            case GL_STACK_UNDERFLOW:
                error = "STACK_UNDERFLOW";
                break;
            case GL_OUT_OF_MEMORY:
                error = "OUT_OF_MEMORY";
                break;
            case GL_INVALID_FRAMEBUFFER_OPERATION:
                error = "INVALID_FRAMEBUFFER_OPERATION";
                break;
        }
		std::cout << error << " | " << file << " (" << line << ")" << std::endl;
	}
	return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void initUniforms() {
    myBasicShader.useShaderProgram();

    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");

    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    lightDirMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDirMatrix");

    projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));


    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.5f, 13.2f, 6.5f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    //pointLight
    posPointLight1 = glm::vec3(2.0f, 10.0f, 18.0f); //on stage, middle
    posPointLightLoc1 = glGetUniformLocation(myBasicShader.shaderProgram, "posPointLight1");
    glUniform3fv(posPointLightLoc1, 1, glm::value_ptr(posPointLight1));

    //spotLight
    spotLight1 = glm::cos(glm::radians(40.0f));
    spotLight2 = glm::cos(glm::radians(50.0f));
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight1"), spotLight1);
    glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "spotLight2"), spotLight2);
    spotLightDirection = myCamera.getCameraFrontDirection();
    spotLightPosition = myCamera.getCameraPosition();
    glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
	glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
}

void windowResizeCallback(GLFWwindow* window, int width, int height) {
	fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
	//TODO
}

bool previewStart = false;

void previewScene() {
    if (previewStart) {
        myCamera.previewAnimation();
    }
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        showDepthMap = !showDepthMap;

	if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        } else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }

    }
}


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
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

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}


void processMovement() {
	if (pressedKeys[GLFW_KEY_W]) {
		myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_S]) {
		myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_A]) {
		myCamera.move(gps::MOVE_LEFT, cameraSpeed);
	}

	if (pressedKeys[GLFW_KEY_D]) {
		myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
	}

    if(pressedKeys[GLFW_KEY_Q]) {
        startAnimations = true;
    }

    if (pressedKeys[GLFW_KEY_E]) {
        startAnimations = false;
        leftGateAngle = 90.0f;
        rightGateAngle = 90.0f;
        leftGateOpening = false;
        rightGateOpening = false;
    }

    //enable spotLight
    if (pressedKeys[GLFW_KEY_Z]) {
        myBasicShader.useShaderProgram();
        enableSpotLight = 1;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "enableSpotLight"), enableSpotLight);
        spotLightDirection = myCamera.getCameraFrontDirection();
        spotLightPosition = myCamera.getCameraPosition();
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightDirection"), 1, glm::value_ptr(spotLightDirection));
        glUniform3fv(glGetUniformLocation(myBasicShader.shaderProgram, "spotLightPosition"), 1, glm::value_ptr(spotLightPosition));
    }

    //disable spotLight
    if (pressedKeys[GLFW_KEY_X]) {
        myBasicShader.useShaderProgram();
        enableSpotLight = 0;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "enableSpotLight"), enableSpotLight);
    }

    //enable pointLight
    if (pressedKeys[GLFW_KEY_C]) {
        myBasicShader.useShaderProgram();
        enablePointLight = 1;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "enablePointLight"), enablePointLight);
    }

    //disable pointLight
    if (pressedKeys[GLFW_KEY_V]) {
        myBasicShader.useShaderProgram();
        enablePointLight = 0;
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "enablePointLight"), enablePointLight);
    }

    if (pressedKeys[GLFW_KEY_F])
    {
        fogDensity = glm::min(fogDensity + 0.001f, 1.0f);
    }

    // decrease the intensity of fog
    if (pressedKeys[GLFW_KEY_G])
    {
        fogDensity = glm::max(fogDensity - 0.001f, 0.0f);
    }

    // move light
    if (pressedKeys[GLFW_KEY_J]) {

        lightAngle += 0.5f;
        if (lightAngle > 360.0f)
            lightAngle -= 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    // move light
    if (pressedKeys[GLFW_KEY_L]) {
        lightAngle -= 0.5f;
        if (lightAngle < 0.0f)
            lightAngle += 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myBasicShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    if (pressedKeys[GLFW_KEY_ENTER]) {
        previewStart = true;
    }

    if (pressedKeys[GLFW_KEY_BACKSPACE]) {
        previewStart = false;
    }

    //viewing solid, wireframe objects, polygonal and smooth surfaces

    if(pressedKeys[GLFW_KEY_1]){
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if(pressedKeys[GLFW_KEY_2]){
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if(pressedKeys[GLFW_KEY_3]){
            glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1600, 900, "OpenGL Interactive Application");
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
    //pointer to hide
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.5, 0.5, 0.5, 1.0);   
    glViewport(0, 0, retina_width, retina_height);
    glEnable(GL_FRAMEBUFFER_SRGB);
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glDisable(GL_CULL_FACE); // cull face
	glCullFace(GL_BACK); // cull back face
	glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBO() {
    //TODO - Create the FBO, the depth texture and attach the depth texture to the FBO
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);

    //bind nothing to attachment points
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    //unbind until ready to use
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix() {
    //TODO - Return the light-space transformation matrix
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, 0.1f, 200.0f);

    glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

    return lightProjection * lightView;
}

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
    mainScene.LoadModel("models/main_scene/main_scene.obj");
    leftGate.LoadModel("models/gate/gate.obj");
    rightGate.LoadModel("models/gate/gate.obj");
    lightCube.LoadModel("models/cube/cube.obj");
    screenQuad.LoadModel("models/quad/quad.obj");
    audience.LoadModel("models/audience/audience.obj");
    discoBall.LoadModel("models/discoball/discoball.obj");
    faces.push_back("skybox/nightsky_rt.tga");
    faces.push_back("skybox/nightsky_lf.tga");
    faces.push_back("skybox/nightsky_up.tga");
    faces.push_back("skybox/nightsky_dn.tga");
    faces.push_back("skybox/nightsky_bk.tga");
    faces.push_back("skybox/nightsky_ft.tga");
    mySkyBox.Load(faces);
}

void initShaders() {
    myBasicShader.loadShader("shaders/basic.vert", "shaders/basic.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    screenQuadShader.loadShader("shaders/screenQuad.vert", "shaders/screenQuad.frag");
    skyBoxShader.loadShader("shaders/skyBoxShader.vert", "shaders/skyBoxShader.frag");
    depthMapShader.loadShader("shaders/depthMapShader.vert", "shaders/depthMapShader.frag");
}


void renderSkyBox(gps::Shader shader) {
    shader.useShaderProgram();
    mySkyBox.Draw(shader, view, projection);
}

void calculateAudiencePositions(){
    //generate object positions
    //y axis should be 0.0
    //they can rotate randomly around y axis
    //they should form a square with origin at (-2.0, 0.0, 0.0) and the distance between them at least 2.5

    //starting position
    glm::mat4 model1(1.0f);
    model1 = glm::translate(model1, glm::vec3(-8.5f, 0.0f, -10.0f));
    //nested loop to create a square
    for(int i=0; i<8; i++){
        for(int j=0; j<8; j++){
            glm::mat4 model2(1.0f);
            model2 = glm::translate(model2, glm::vec3(0.0f, 0.0f, 0.0f));
            model2 = glm::translate(model2, glm::vec3(i*2.5f, 0.0f, j*2.5f));
            srand(glfwGetTime());
            //at max 45 dgrees rotation
            model2 = glm::rotate(model2, glm::radians((float)(rand() % 45)), glm::vec3(0.0f, 1.0f, 0.0f));
            modelMatrices.push_back(model1 * model2);
        }
    }
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    //teapot
    model = glm::mat4(1.0f);

    //stage
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    mainScene.Draw(shader);


    //gates
    model = glm::translate(glm::mat4(1.0f), glm::vec3(4.4f, 0.3f, -16.0f));
    if (startAnimations) {
        if (leftGateAngle == 90.0f)
            leftGateOpening = false;
        else if (leftGateAngle == 0.0f)
            leftGateOpening = true;

        if (leftGateOpening) {
            leftGateAngle += 0.5;
        }
        else {
            leftGateAngle -= 0.5;
        }
    }
    else {
        leftGateAngle = 90.0f;
    }

    model = glm::rotate(model, glm::radians(-leftGateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    leftGate.Draw(shader);

    model = glm::translate(glm::mat4(1.0f), glm::vec3(-3.8f, 0.3f, -16.4f));
    if (startAnimations) {
        if (rightGateAngle == 90.0f)
            rightGateOpening = false;
        else if (rightGateAngle == 0.0f)
            rightGateOpening = true;

        if (leftGateOpening) {
            rightGateAngle += 0.5;
        }
        else {
            rightGateAngle -= 0.5;
        }
    }
    else {
        rightGateAngle = 90.0f;
    }

    model = glm::rotate(model, glm::radians(rightGateAngle), glm::vec3(0.0f, 1.0f, 0.0f));

    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    rightGate.Draw(shader);
    //audience
    for (unsigned int i = 0; i < modelMatrices.size(); i++) {
    model = modelMatrices[i];
    float jumpHeight = (float)(rand() % 16 + 7) / 10.0f + model[3][1];
    float jumpSpeed = (float)(rand() % 100) / 250.0f;
    float delayTimer = (float)(rand() % 10);
    if (startAnimations) {
        if (jumpUp[i]) {
            if (model[3][1] >= jumpHeight) {
                jumpUp[i] = false;
                delayTimer = (float)(rand() % 100) / 100.0f;
            }
            else {
                model = glm::translate(model, glm::vec3(0.0f, jumpSpeed, 0.0f));
            }
        }
        else {
            if (model[3][1] <= jumpHeight - jumpSpeed) {
                jumpUp[i] = true;
            }
            else {
                model = glm::translate(model, glm::vec3(0.0f, -jumpSpeed, 0.0f));
            }
        }
    }
    delayTimer -= deltaTime;
    if (delayTimer <= 0.0f) {
        jumpUp[i] = true;
    }
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        if (!depthPass) {
            normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
            glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
        }
        audience.Draw(shader);
    }


    //discoBall
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 9.5f, 14.6f));
    if (startAnimations) {
        discoBallAngle += 0.5f;
    }

    model = glm::rotate(model, glm::radians(discoBallAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    discoBall.Draw(shader);

    //teapot
    model = glm::translate(glm::mat4(1.0f), glm::vec3(4.4f, 2.0f, 12.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    if (!depthPass) {
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    }
    teapot.Draw(shader);
}


void renderScene() {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    previewScene();

    processMovement();

    // 1st step: render the scene to the depth buffer 
    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    //drawObjects
    drawObjects(depthMapShader, 1);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (showDepthMap) {
        glViewport(0, 0, retina_width, retina_height);

        glClear(GL_COLOR_BUFFER_BIT);

        screenQuadShader.useShaderProgram();

        //bind the depth map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(screenQuadShader.shaderProgram, "depthMap"), 0);

        glDisable(GL_DEPTH_TEST);
        screenQuad.Draw(screenQuadShader);
        glEnable(GL_DEPTH_TEST);
    }
    else {


        //2nd step render everything else
        myBasicShader.useShaderProgram();

        // send lightSpace matrix to shader
        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

        // send view matrix to shader
        view = myCamera.getViewMatrix();

        glUniformMatrix4fv(glGetUniformLocation(myBasicShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

        // compute light direction transformation matrix
        lightDirMatrix = glm::mat3(glm::inverseTranspose(view));
        // send lightDir matrix data to shader
        glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

        glViewport(0, 0, retina_width, retina_height);
        myBasicShader.useShaderProgram();

        // bind the depth map
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, depthMapTexture);
        glUniform1i(glGetUniformLocation(myBasicShader.shaderProgram, "shadowMap"), 3);


        glUniform1f(glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity"), fogDensity);


        //draw objects
        drawObjects(myBasicShader, false);

        //light
        lightShader.useShaderProgram();
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
        model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(3.0f, 5.0f, 3.0f));
        model = glm::scale(model, glm::vec3(1.0f, 1.0f, 1.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
        lightCube.Draw(lightShader);

        //render skybox
        renderSkyBox(skyBoxShader);
    }
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    calculateAudiencePositions();
    initShaders();
    initUniforms();
    initFBO();
    setWindowCallbacks();
	glCheckError();
    float lastFrame = 0;

	// application loop
	while (!glfwWindowShouldClose(myWindow.getWindow())) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
	    renderScene();

		glfwPollEvents();
		glfwSwapBuffers(myWindow.getWindow());

		glCheckError();
	}

	cleanup();

    return EXIT_SUCCESS;
}
