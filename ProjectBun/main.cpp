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

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
GLfloat lightAngle;

// shader uniform locations
GLuint modelLoc;
GLuint viewLoc;
GLuint projectionLoc;
GLuint normalMatrixLoc;
GLuint lightDirLoc;
GLuint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(100.0f, 25.0f, 40.0f),
    glm::vec3(10.0f, 8.0f, 20.0f),
    glm::vec3(10.0f, 10.0f, 2.0f));

GLfloat cameraSpeed = 1.0f;

GLboolean pressedKeys[1024];

// models
gps::Model3D teapot;
gps::Model3D sun;
gps::Model3D fullScene;
gps::Model3D moara;
gps::Model3D racoon;
gps::Model3D sperietoare;
gps::Model3D tank;
gps::Model3D tree;
gps::Model3D leaves;
gps::Model3D bird;
float angle;
glm::mat4 birdMatrix;
GLfloat birdRotation = 0.0f;


//skybox 
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;
gps::Shader skyboxShader;

//tank movement
float move1;
float move2;
float move3;

// shaders
gps::Shader myCustomShader;
gps::Shader lightShader;
gps::Shader depthMapShader;

//mouse
float lastX = 0, lastY = 0;
float yaw = -90.0f, pitch;
bool mouse = true;

//shadow
const unsigned int SHADOW_WIDTH = 4096;
const unsigned int SHADOW_HEIGHT = 2048;

//fog
int foginit = 0;
GLint foginitLoc;
GLfloat fogDensity = 0.000f;

//pont light
int pointinit = 0;
glm::vec3 lightPos1; 
GLuint lightPos1Loc;

GLuint shadowMapFBO;
GLuint depthMapTexture;
GLuint lightDirMatrixLoc;
glm::mat3 lightDirMatrix;
float var;

int retina_width = myWindow.getWindowDimensions().width;
int retina_height = myWindow.getWindowDimensions().height;

GLenum glCheckError_(const char* file, int line)
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

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    retina_width = myWindow.getWindowDimensions().width;
    retina_height = myWindow.getWindowDimensions().height;
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);

    myCustomShader.useShaderProgram();

    // set projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    //send matrix data to shader
    GLint projLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    lightShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // set Viewport transform
    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void initFaces()
{

    faces.push_back("textures/skybox1/right.tga");
    faces.push_back("textures/skybox1/left.tga");
    faces.push_back("textures/skybox1/top.tga");
    faces.push_back("textures/skybox1/bottom.tga");
    faces.push_back("textures/skybox1/back.tga");
    faces.push_back("textures/skybox1/front.tga");
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (mouse)
    {
        lastX = xpos;
        lastY = ypos;
        mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);
}

void processMovement() {
    // right rotate
    if (pressedKeys[GLFW_KEY_E]) {
        angle += 0.5f;
        if (angle > 360.0f)
            angle -= 360.0f;
    }

    // left rotate
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 0.5f;
        if (angle < 0.0f)
            angle += 360.0f;
    }

    // forward
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }

    // backward
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }

    // left
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }

    // right
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }

    // move light
    if (pressedKeys[GLFW_KEY_L]) {

        lightAngle += 0.5f;
        if (lightAngle > 360.0f)
            lightAngle -= 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myCustomShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    // move light
    if (pressedKeys[GLFW_KEY_J]) {
        lightAngle -= 0.5f;
        if (lightAngle < 0.0f)
            lightAngle += 360.0f;
        glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
        myCustomShader.useShaderProgram();
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDirTr));
    }

    // INCREASE fog
    if (pressedKeys[GLFW_KEY_5])
    {
        fogDensity = glm::min(fogDensity + 0.0001f, 1.0f);
    }

    // DECREASE fog
    if (pressedKeys[GLFW_KEY_6])
    {
        fogDensity = glm::max(fogDensity - 0.0001f, 0.0f);
    }

    // move LEFT ( tank )
    if (pressedKeys[GLFW_KEY_LEFT]) {
        if (move2 > -8)
            move2 -= 0.25;
    }

    // move RIGHT ( tank )
    if (pressedKeys[GLFW_KEY_RIGHT]) {
        if (move2 < 8)
            move2 += 0.25;
    }

    // move FRONT ( tank )
    if (pressedKeys[GLFW_KEY_UP]) {
        if (move3 < 43)
            move3 += 0.25;
    }

    // move BACK ( tank )
    if (pressedKeys[GLFW_KEY_DOWN]) {
        if (move3 > -17)
            move3 -= 0.25;
    }

    // start pointlight
    if (pressedKeys[GLFW_KEY_3]) {
        myCustomShader.useShaderProgram();
        pointinit = 1;
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
    }

    // stop pointlight
    if (pressedKeys[GLFW_KEY_4]) {
        myCustomShader.useShaderProgram();
        pointinit = 0;
        glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "pointinit"), pointinit);
    }

    // line view
    if (pressedKeys[GLFW_KEY_7]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    // point view
    if (pressedKeys[GLFW_KEY_8]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    // normal view
    if (pressedKeys[GLFW_KEY_9]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "Proiect fain la grafica -_-");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    //glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initFBO()
{
    //generate FBO ID
    glGenFramebuffers(1, &shadowMapFBO);

    //create depth texture for FBO
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    //attach texture to FBO
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 computeLightSpaceTrMatrix()
{
    const GLfloat near_plane = 35.0f, far_plane = 200.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, -100.0f, 100.0f, near_plane, far_plane);

    glm::vec3 lightDirTr = glm::vec3(glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f)) * glm::vec4(lightDir, 1.0f));
    glm::mat4 lightView = glm::lookAt(lightDirTr, myCamera.getCameraTarget(), glm::vec3(0.0f, 1.0f, 0.0f));

    return lightProjection * lightView;
}

void initModels() {
    sun.LoadModel("models/sun/13913_Sun_v2_l3.obj", "models/sun/");
    fullScene.LoadModel("models/Castle/Castle OBJ.obj", "models/Castle/");
    tank.LoadModel("models/tank/uaz.obj", "models/tank/");
    bird.LoadModel("models/bird/13625_Pterodactylus_v1_L1.obj", "models/bird/");
    tree.LoadModel("models/tree/treeG.obj", "models/tree/");
    leaves.LoadModel("models/leaves/treeG.obj", "models/leaves/");
}

void initShaders() {
    myCustomShader.loadShader("shaders/shaderStart.vert", "shaders/shaderStart.frag");
    lightShader.loadShader("shaders/lightCube.vert", "shaders/lightCube.frag");
    depthMapShader.loadShader("shaders/simpleDepthMap.vert", "shaders/simpleDepthMap.frag");
}

void initUniforms() {


    myCustomShader.useShaderProgram();

    modelLoc = glGetUniformLocation(myCustomShader.shaderProgram, "model");

    viewLoc = glGetUniformLocation(myCustomShader.shaderProgram, "view");

    normalMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "normalMatrix");

    lightDirMatrixLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDirMatrix");

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myCustomShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 2.5f, 0.5f) * 20.0f;
    lightDirLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    // set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myCustomShader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    // pointlight
    lightPos1Loc = glGetUniformLocation(myCustomShader.shaderProgram, "lightPos1");
    glUniform3fv(lightPos1Loc, 1, glm::value_ptr(lightPos1));

    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send teapot model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send teapot normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw teapot
    teapot.Draw(shader);
}

void initSkyBoxShader()
{
    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE,
        glm::value_ptr(view));

    projection = glm::perspective(glm::radians(45.0f), (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE,
        glm::value_ptr(projection));
}

void renderBird(gps::Shader shader) {

    shader.useShaderProgram();
    birdMatrix = glm::mat4(0.5f);
    birdMatrix = glm::rotate(birdMatrix, glm::radians(angle), glm::vec3(0, 1, 0));
    birdMatrix = glm::rotate(birdMatrix, glm::radians(birdRotation), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(birdMatrix));
   
    bird.Draw(myCustomShader);

    
    if (birdRotation < 360.0f) {
        birdRotation += 0.3f;
    }
    else {
        birdRotation = 0;
    }
    
}

void renderTank(gps::Shader shader) {

    shader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(move2, move1, -move3));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  
    tank.Draw(shader);
}

void renderTree(gps::Shader shader) {

    shader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
   
    tree.Draw(shader);
}

void renderLeaves(gps::Shader shader) {

    shader.useShaderProgram();
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
    model = glm::translate(model, glm::vec3(var, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
 

    leaves.Draw(shader);
}

void renderBackgroundScene(gps::Shader shader) {

    shader.useShaderProgram();
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0, 1, 0));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
  
    fullScene.Draw(shader);
}




void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // wind effect
    var = sin(glfwGetTime()) * 0.1f;

    // 1st step: render the scene to the depth buffer 

    depthMapShader.useShaderProgram();

    glUniformMatrix4fv(glGetUniformLocation(depthMapShader.shaderProgram, "lightSpaceTrMatrix"),
        1,
        GL_FALSE,
        glm::value_ptr(computeLightSpaceTrMatrix()));

    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    // render the bird
    renderBird(depthMapShader);
   
    // render the tank
    renderTank(depthMapShader);

    // render trees
    renderTree(depthMapShader);

    // render leaves
    renderLeaves(depthMapShader);

    // render the scene
    renderBackgroundScene(depthMapShader);
   

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 2nd step: render the scene

    myCustomShader.useShaderProgram();

    // send lightSpace matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(computeLightSpaceTrMatrix()));

    // send view matrix to shader
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(myCustomShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // compute light direction transformation matrix
    lightDirMatrix = glm::mat3(glm::inverseTranspose(view));

    // send lightDir matrix data to shader
    glUniformMatrix3fv(lightDirMatrixLoc, 1, GL_FALSE, glm::value_ptr(lightDirMatrix));

    glViewport(0, 0,myWindow.getWindowDimensions().width , myWindow.getWindowDimensions().height);
    myCustomShader.useShaderProgram();

    // bind the depth map
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(myCustomShader.shaderProgram, "shadowMap"), 3);

    // bind the fog map
    glUniform1f(glGetUniformLocation(myCustomShader.shaderProgram, "fogDensity"), fogDensity);

    // draw the bird
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderBird(myCustomShader);


    // draw the tank
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderTank(myCustomShader);

    // draw the tree
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderTree(myCustomShader);

    // draw the leaves
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderLeaves(myCustomShader);

    // draw the scene
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    renderBackgroundScene(myCustomShader);


    // draw a white circle
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    model = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::translate(model, lightDir);
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));

    sun.Draw(lightShader);

    mySkyBox.Draw(skyboxShader, view, projection);

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
    initOpenGLState();
    initFBO();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    initFaces();
    initSkyBoxShader();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();
        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}
