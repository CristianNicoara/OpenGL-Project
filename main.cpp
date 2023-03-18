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
glm::mat4 bladesMatrix;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;
glm::vec3 lightPosEye;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPosEyeLoc;

int fog = 0;
GLfloat fogLoc, fogLocSkyBox;

// camera
gps::Camera myCamera(
    glm::vec3(10.0f, 3.0f, 10.0f),
    glm::vec3(0.0f, 0.0f, -10.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

float lastX, lastY;
float pitch, yaw;
bool firstMouse = true;
bool animation = false;
bool selfMove = false;
bool show = true;
int showTime = 0;
float bladesMovement = 0.0f;

GLfloat cameraSpeed = 0.75f, sensitivity = 0.1f;

GLboolean pressedKeys[1024];

// models
gps::Model3D scene;
gps::Model3D blades;
gps::Model3D blades1;
gps::Model3D blades2;
gps::Model3D blades3;
gps::Model3D windmillBlades;
GLfloat angle;

// shaders
gps::Shader myBasicShader;

//skybox
gps::SkyBox skyBox;
gps::Shader skyBoxShader;

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
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key == GLFW_KEY_C && action == GLFW_PRESS) {
        myCamera.pposition();
    }

    //Start/stop animations
    if (key == GLFW_KEY_M && action == GLFW_PRESS) {
        animation = !animation;
    }

    //Wireframe
    if (key == GLFW_KEY_J && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }
    //Normal
    if (key == GLFW_KEY_K && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    //Points
    if (key == GLFW_KEY_L && action == GLFW_PRESS) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    //fog
    if (key == GLFW_KEY_F && action == GLFW_RELEASE) {
        fog = 1 - fog;

        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
        glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

        glUniform1i(fogLoc, fog);
        skyBoxShader.useShaderProgram();
        glUniform1i(fogLocSkyBox, fog);

    }

    //self moving camera
    if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
        selfMove = !selfMove;
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


void mouseCallback(GLFWwindow* window, double xpos, double ypos) {

    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xDiff = (float)xpos - lastX;
    float yDiff = (float)ypos - lastY;
    lastX = (float)xpos;
    lastY = (float)ypos;

    xDiff *= sensitivity;
    yDiff *= sensitivity;

    yaw -= xDiff;
    pitch -= yDiff;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    myCamera.rotate(pitch, yaw);

    view = myCamera.getViewMatrix();
    myBasicShader.useShaderProgram();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        yaw += 1.0;
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        yaw -= 1.0;
        myCamera.rotate(pitch, yaw);
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
}

void initOpenGLWindow() {
    myWindow.Create(1920, 1080, "Proiect Prelucrare Grafica Nicoara Cristian-Catalin");
    glfwGetFramebufferSize(myWindow.getWindow(), &retina_width, &retina_height);
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initSkyBox()
{
    std::vector<const GLchar*> faces;
    faces.push_back("skybox/plains-of-abraham_rt.tga");
    faces.push_back("skybox/plains-of-abraham_lf.tga");
    faces.push_back("skybox/plains-of-abraham_up.tga");
    faces.push_back("skybox/plains-of-abraham_dn.tga");
    faces.push_back("skybox/plains-of-abraham_bk.tga");
    faces.push_back("skybox/plains-of-abraham_ft.tga");


    skyBox.Load(faces);
    skyBoxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyBoxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    projection = glm::perspective(glm::radians(90.0f), (float)retina_width / (float)retina_height, 0.1f, 75.0f);
    glUniformMatrix4fv(glGetUniformLocation(skyBoxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initModels() {
    scene.LoadModel("models/objects/scena1.obj");
    blades.LoadModel("models/objects/Blades.obj");
    blades1.LoadModel("models/objects/Blades1.obj");
    blades2.LoadModel("models/objects/Blades2.obj");
    blades3.LoadModel("models/objects/Blades3.obj");
    windmillBlades.LoadModel("models/objects/WindmillBlades.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 400.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    lightPosEye = glm::vec3(154.788f, 12.426f, 4.751f);
    lightPosEyeLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPosEye");
    // send light position to shader
    glUniform3fv(lightPosEyeLoc, 1, glm::value_ptr(lightPosEye));

    fogLoc = glGetUniformLocation(myBasicShader.shaderProgram, "fog");
    glUniform1i(fogLoc, fog);

    skyBoxShader.useShaderProgram();
    fogLocSkyBox = glGetUniformLocation(skyBoxShader.shaderProgram, "fog");
    glUniform1i(fogLocSkyBox, fog);
}

void renderBlade(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send blades model matrix data to shader

    bladesMatrix = glm::mat4(1.0f);
    bladesMatrix = glm::translate(bladesMatrix, glm::vec3(125.444f, 35.511f, 79.00f));
    bladesMatrix = glm::rotate(bladesMatrix, glm::radians(bladesMovement), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bladesMatrix));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * bladesMatrix));
    //send blades normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw objects

    blades.Draw(shader);
}

void renderBlade1(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send blades model matrix data to shader

    bladesMatrix = glm::mat4(1.0f);
    bladesMatrix = glm::translate(bladesMatrix, glm::vec3(43.80f, 35.511f, 77.881f));
    bladesMatrix = glm::rotate(bladesMatrix, glm::radians(bladesMovement), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bladesMatrix));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * bladesMatrix));
    //send blades normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw objects

    blades1.Draw(shader);
}

void renderBlade2(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send blades model matrix data to shader

    bladesMatrix = glm::mat4(1.0f);
    bladesMatrix = glm::translate(bladesMatrix, glm::vec3(54.434f, 35.511f, -89.514f));
    bladesMatrix = glm::rotate(bladesMatrix, glm::radians(bladesMovement), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bladesMatrix));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * bladesMatrix));
    //send blades normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw objects
    blades2.Draw(shader);
}

void renderBlade3(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send blades model matrix data to shader

    bladesMatrix = glm::mat4(1.0f);
    bladesMatrix = glm::translate(bladesMatrix, glm::vec3(124.673f, 35.511f, -98.123f));
    bladesMatrix = glm::rotate(bladesMatrix, glm::radians(bladesMovement), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bladesMatrix));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * bladesMatrix));
    //send blades normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw objects
    blades3.Draw(shader);
}

void renderWindmillBlade(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send blades model matrix data to shader

    bladesMatrix = glm::mat4(1.0f);
    bladesMatrix = glm::translate(bladesMatrix, glm::vec3(35.317f, 16.916f, 16.683f));
    bladesMatrix = glm::rotate(bladesMatrix, glm::radians(bladesMovement), glm::vec3(0.0f, 0.0f, 1.0f));

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bladesMatrix));

    normalMatrix = glm::mat3(glm::inverseTranspose(view * bladesMatrix));
    //send blades normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw objects
    windmillBlades.Draw(shader);
}

void renderObjects(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send scene model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

    //send scene normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scene
    scene.Draw(shader);
}

void cameraMovement() {
    if (show) {
        if (showTime >= 0 && showTime < 150) {
            if (yaw > -145.0f) {
                yaw -= 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 150 && showTime < 200) {
            if (pitch > -60.0f) {
                pitch -= 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 200 && showTime < 250) {
            myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        }
        else if (showTime >= 250 && showTime < 300) {
            if (pitch < 50.0f) {
                pitch += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 300 && showTime < 325) {
            myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        }
        else if (showTime >= 325 && showTime < 425) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        else if (showTime >= 425 && showTime < 480) {
            if (yaw < 30.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 480 && showTime < 580) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        else if (showTime >= 580 && showTime < 650) {
            if (yaw < 45.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 650 && showTime < 680) {
            if (pitch < 20.0f) {
                pitch += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 680 && showTime < 750) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        else if (showTime >= 750 && showTime < 860) {
            if (yaw < 100.0f) {
                yaw += 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 860 && showTime < 880) {
            if (pitch > -15.0f) {
                pitch -= 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        else if (showTime >= 880 && showTime < 1000) {
            myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        }
        else if (showTime >= 1000 && showTime < 1200) {
            if (yaw > -180.0f) {
                yaw -= 1.0;
                myCamera.rotate(pitch, yaw);
            }
        }
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }
    if (showTime > 1200)
        show = false;
    else
        showTime++;
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //render the scene


    if (animation == true) {
        if (bladesMovement >= 360)
            bladesMovement = 0;
        bladesMovement += 2;
    }
    renderBlade(myBasicShader);
    renderBlade1(myBasicShader);
    renderBlade2(myBasicShader);
    renderBlade3(myBasicShader);
    renderWindmillBlade(myBasicShader);
    renderObjects(myBasicShader);
    skyBox.Draw(skyBoxShader, view, projection);
    if (selfMove) {
        cameraMovement();
    }

}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
    glfwTerminate();
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
    initModels();
    initSkyBox();
    initShaders();
    initUniforms();
    setWindowCallbacks();

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
