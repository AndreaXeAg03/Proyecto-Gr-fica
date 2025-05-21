#include <iostream>
#include <cmath>

// GLEW for OpenGL function loading
#include <GL/glew.h>

// GLFW for window creation and input
#include <GLFW/glfw3.h>

// Image loading library
#include "stb_image.h"

// GLM for mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Model loading library
#include "SOIL2/SOIL2.h"

// Custom classes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// Function prototypes
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void Inputs(GLFWwindow* window, float deltaTime);
void Animation();

// Window dimensions
const GLuint WIDTH = 1600, HEIGHT = 1200;
int SCREEN_WIDTH, SCREEN_HEIGHT;

// Camera system
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
bool keys[1024]; // Keyboard state array

// Mouse control variables
bool firstMouse = true;          // First mouse movement flag
float lastX = WIDTH / 2.0f;      // Last X position
float lastY = HEIGHT / 2.0f;     // Last Y position
float yaw = -90.0f;              // Yaw angle (initialized to -90°)
float pitch = 0.0f;              // Pitch angle
float fov = 45.0f;               // Field of view

// Mouse button states
bool rightMousePressed = false;   // Right mouse button state
bool leftMousePressed = false;    // Left mouse button state

// Camera system variables
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 8.0f);    // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera front vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // Camera up vector

// Sunset effect variables
float sunsetFactor = 0.0f;        // Current sunset progression (0-1)
float sunsetSpeed = 0.05f;        // Speed of sunset transition
bool isSunsetActive = false;      // Flag for active sunset transition
bool keyTPressedLastFrame = false; // Previous frame T key state

// Color definitions
glm::vec3 dayColor(0.60f, 0.82f, 0.96f);     // Daytime sky color (light blue)
glm::vec3 sunsetColor(0.96f, 0.64f, 0.38f);  // Sunset color (orange-red)

// Door animation variables
float doorAngle = 0.0f;          // Current door rotation angle (0-90)
bool isDoorOpening = false;      // Flag for door opening state
float doorOpenSpeed = 45.0f;     // Door opening speed (degrees per second)
glm::vec3 doorPosition(0.0f, 0.0f, 0.0f); // Door position

// Second door animation variables
float door2Angle = 0.0f;          // Current second door rotation angle
bool isDoor2Opening = false;      // Flag for second door opening state
float door2OpenSpeed = 45.0f;     // Second door opening speed
glm::vec3 door2Position(0.0f, 0.0f, 0.0f); // Second door position

// Chair animation variables
float chairRotation = 0.0f;                  // Current chair rotation in Y
glm::vec3 chairPosition(0.0f, 0.0f, 0.0f);  // Initial chair position
bool chairAdjusted = false;                 // Chair adjustment state
float animSpeed = 1.8f;                      // Animation speed
glm::vec3 chairTargetPosition(-1.8f, 0.0f, 0.0f); // Target position when adjusted
glm::vec3 pivotOffset(0.3f, 0.0f, -0.4f);  // Pivot point (front right leg)
float chairTargetRotation = -180.0f;        // Target rotation when adjusted

// Shower animation variables
glm::vec3 showerPosition(0.0f, 0.0f, 0.0f);  // Initial shower position
glm::vec3 showerTargetPosition(0.21f, 0.0f, -0.4f); // Closed position
bool showerClosed = false;  // Shower state
float showerSpeed = 2.0f;   // Movement speed

// House position and rotation
glm::vec3 housePos(0.0f, 0.0f, 0.0f);
float houseRot = 0.0f;

// Time variables
GLfloat deltaTime = 0.0f;    // Time between current frame and last frame
GLfloat lastFrame = 0.0f;    // Time of last frame

int main()
{
    // Initialize GLFW
    glfwInit();

    // Configure GLFW window hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    // Create GLFW window
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "State Machine Animation", nullptr, nullptr);
    if (nullptr == window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (GLEW_OK != glewInit())
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Set viewport dimensions
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Create and compile shaders
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");
    Shader lightingShader("Shader/lighting.vs", "Shader/lighting.frag");
    Shader lampShader("Shader/lamp.vs", "Shader/lamp.frag");

    // Load 3D models
    Model House((char*)"Models/casa.obj");
    Model Floor((char*)"Models/piso.obj");
    Model Glass((char*)"Models/Crystal.obj");
    Model Door((char*)"Models/door.obj");
    Model Door2((char*)"Models/door2.obj");
    Model Chair((char*)"Models/chair.obj");
    Model Shower((char*)"Models/shower.obj");

    // Initialize projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(fov),
        (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);

    // Set GLFW callbacks
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

    // Set vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Set texture units for lighting shader
    lightingShader.Use();
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.difuse"), 0);
    glUniform1i(glGetUniformLocation(lightingShader.Program, "Material.specular"), 1);

    // Main game loop
    while (!glfwWindowShouldClose(window))
    {
        // Calculate delta time
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Process inputs
        Inputs(window, deltaTime);

        // Poll events
        glfwPollEvents();

        // Sunset effect logic
        if (isSunsetActive) {
            sunsetFactor += sunsetSpeed * deltaTime;

            // Reset when sunset completes
            if (sunsetFactor >= 1.0f) {
                sunsetFactor = 0.0f;
                isSunsetActive = false;
            }
        }

        // Set clear color based on sunset progression
        glm::vec3 currentColor = glm::mix(dayColor, sunsetColor, sunsetFactor);
        glClearColor(currentColor.r, currentColor.g, currentColor.b, 1.0f);

        // Update animations
        Animation();

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        shader.Use();

        // Update projection matrix if FOV changed
        projection = glm::perspective(glm::radians(fov),
            (GLfloat)SCREEN_WIDTH / (GLfloat)SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        // Create view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"),
            1, GL_FALSE, glm::value_ptr(view));

        lightingShader.Use();
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetZoom()),
            (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "view"),
            1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        // Draw FLOOR (opaque)
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.34f, 0.0f));
        model = glm::scale(model, glm::vec3(5.0f, 1.0f, 5.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Floor.Draw(lightingShader);

        // Draw DOOR 
        model = glm::mat4(1.0f);
        model = glm::translate(model, doorPosition);

        const float hingeOffsetX = 0.37f;
        const float hingeOffsetY = 0.0f;
        const float hingeOffsetZ = 0.3f;

        model = glm::translate(model, glm::vec3(hingeOffsetX / 2, hingeOffsetY, hingeOffsetZ));
        model = glm::rotate(model, glm::radians(doorAngle), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-hingeOffsetX / 2, -hingeOffsetY, -hingeOffsetZ));

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        Door.Draw(lightingShader);

        // Draw CHAIR with rotation around leg
        model = glm::mat4(1.0f);
        model = glm::translate(model, chairPosition);
        model = glm::translate(model, pivotOffset);
        model = glm::rotate(model, glm::radians(chairRotation), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::translate(model, -pivotOffset);

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        Chair.Draw(lightingShader);

        // Draw SHOWER (translation only)
        model = glm::mat4(1.0f);
        model = glm::translate(model, showerPosition);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        Shower.Draw(lightingShader);

        // Draw HOUSE (opaque)
        model = glm::mat4(1.0f);
        model = glm::translate(model, housePos);
        model = glm::rotate(model, glm::radians(houseRot), glm::vec3(0.0f, 1.0f, 0.0f));
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 0);
        House.Draw(lightingShader);

        // Draw GLASS (transparent)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        model = glm::mat4(1.0f);
        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.5f);
        Glass.Draw(lightingShader);
        glDisable(GL_BLEND);

        //// Draw SECOND DOOR (transparent)
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        model = glm::mat4(1.0f);
        model = glm::translate(model, doorPosition);

        model = glm::translate(model, glm::vec3(hingeOffsetX / 2, hingeOffsetY, hingeOffsetZ));
        model = glm::rotate(model, glm::radians(doorAngle), glm::vec3(0.0f, -1.0f, 0.0f));
        model = glm::translate(model, glm::vec3(-hingeOffsetX / 2, -hingeOffsetY, -hingeOffsetZ));

        glUniformMatrix4fv(glGetUniformLocation(lightingShader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        glUniform1i(glGetUniformLocation(lightingShader.Program, "transparency"), 1);
        glUniform1f(glGetUniformLocation(lightingShader.Program, "alpha"), 0.5f);
        Door2.Draw(lightingShader);
        glDisable(GL_BLEND);

        glfwSwapBuffers(window);
    }

    // Clean up
    glfwTerminate();
    return 0;
}

// Input processing function
void Inputs(GLFWwindow* window, float deltaTime) {
    // Close window on ESC
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Camera movement speed
    float cameraSpeed = 2.5f * deltaTime;

    // WASD camera movement
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // QE for vertical movement
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraUp;
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraUp;

    // Key '1' to open/close door
    static bool key1PressedLastFrame = false;
    bool key1PressedThisFrame = (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS);

    if (key1PressedThisFrame && !key1PressedLastFrame) {
        isDoorOpening = !isDoorOpening;
        isDoor2Opening = !isDoor2Opening;
    }
    key1PressedLastFrame = key1PressedThisFrame;

    // Key '2' to toggle chair adjustment
    static bool key2PressedLastFrame = false;
    bool key2PressedThisFrame = (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS);

    if (key2PressedThisFrame && !key2PressedLastFrame) {
        chairAdjusted = !chairAdjusted;
    }
    key2PressedLastFrame = key2PressedThisFrame;

    // Key '3' to open/close shower
    static bool key3PressedLastFrame = false;
    bool key3PressedThisFrame = (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS);

    if (key3PressedThisFrame && !key3PressedLastFrame) {
        showerClosed = !showerClosed;
    }
    key3PressedLastFrame = key3PressedThisFrame;

    // Key '4' for sunset effect
    bool keyTPressedThisFrame = (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS);
    if (keyTPressedThisFrame && !keyTPressedLastFrame) {
        isSunsetActive = true;
    }
    keyTPressedLastFrame = keyTPressedThisFrame;
}

// Animation update function
void Animation() {
    // Door animation
    if (isDoorOpening) {
        doorAngle += doorOpenSpeed * deltaTime;
        if (doorAngle >= 90.0f) doorAngle = 90.0f;
    }
    else {
        doorAngle -= doorOpenSpeed * deltaTime;
        if (doorAngle <= 0.0f) doorAngle = 0.0f;
    }

    // Second door animation
    if (isDoor2Opening) {
        door2Angle += door2OpenSpeed * deltaTime;
        if (door2Angle >= 90.0f) door2Angle = 90.0f;
    }
    else {
        door2Angle -= door2OpenSpeed * deltaTime;
        if (door2Angle <= 0.0f) door2Angle = 0.0f;
    }

    // Chair rotation and movement
    float targetRotation = chairAdjusted ? -50.0f : 0.0f;
    glm::vec3 targetPosition = chairAdjusted ? chairTargetPosition : glm::vec3(0.0f);

    // Smooth interpolation for both rotation and position
    chairRotation = glm::mix(chairRotation, targetRotation, animSpeed * deltaTime * 2.0f);
    chairPosition = glm::mix(chairPosition, targetPosition, animSpeed * deltaTime * 1.5f);

    // Shower animation
    glm::vec3 targetShowerPos = showerClosed ? showerTargetPosition : glm::vec3(0.0f);
    showerPosition = glm::mix(showerPosition, targetShowerPos, showerSpeed * deltaTime);
}

// Mouse movement callback
void MouseCallback(GLFWwindow* window, double xPos, double yPos)
{
    if (firstMouse)
    {
        lastX = xPos;
        lastY = yPos;
        firstMouse = false;
    }

    // Calculate mouse offset
    float xoffset = xPos - lastX;
    float yoffset = lastY - yPos; // Reversed y-coordinates
    lastX = xPos;
    lastY = yPos;

    // Apply sensitivity
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Update yaw and pitch
    yaw += xoffset;
    pitch += yoffset;

    // Constrain pitch to avoid flipping
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    // Calculate new front vector
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

// Mouse button callback
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Right mouse button handling
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightMousePressed = true;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            firstMouse = true;
        }
        else if (action == GLFW_RELEASE) {
            rightMousePressed = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        }
    }

    // Left mouse button handling
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS)
            leftMousePressed = true;
        else if (action == GLFW_RELEASE)
            leftMousePressed = false;
    }
}

// Mouse scroll callback
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    fov -= (float)yoffset;
    if (fov < 1.0f) fov = 1.0f;
    if (fov > 45.0f) fov = 45.0f;
}