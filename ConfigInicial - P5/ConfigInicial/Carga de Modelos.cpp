// Standard Library Includes
#include <string>

// GLEW (OpenGL Extension Wrangler Library)
#include <GL/glew.h>

// GLFW (Graphics Library Framework)
#include <GLFW/glfw3.h>

// Custom GL includes
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

// GLM Mathematics 
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Other Libraries
#include "SOIL2/SOIL2.h"  // For image loading
#include "stb_image.h"    // For texture loading

// Window properties
const GLint WIDTH = 1200, HEIGHT = 800;
int screenWidth, screenHeight;  // Actual framebuffer dimensions

// Function prototypes
void Inputs(GLFWwindow* window, float deltaTime);
void MouseCallback(GLFWwindow* window, double xpos, double ypos);
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// Camera system variables
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 8.0f);    // Camera position
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // Camera front vector
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);     // Camera up vector

// Mouse control variables
bool firstMouse = true;          // First mouse movement flag
float lastX = WIDTH / 2.0f;      // Last X position
float lastY = HEIGHT / 2.0f;     // Last Y position
float yaw = -90.0f;              // Yaw angle (initialized to -90°)
float pitch = 0.0f;              // Pitch angle
float fov = 45.0f;               // Field of view

// Timing variables
float deltaTime = 0.0f;          // Time between frames
float lastFrame = 0.0f;          // Time of last frame

// Mouse button states
bool rightMousePressed = false;   // Right mouse button state
bool leftMousePressed = false;    // Left mouse button state

// Sunset effect variables
float sunsetFactor = 0.0f;        // Current sunset progression (0-1)
float sunsetSpeed = 0.05f;        // Speed of sunset transition
bool isSunsetActive = false;      // Flag for active sunset transition
bool keyTPressedLastFrame = false; // Previous frame T key state

// Color definitions
glm::vec3 dayColor(0.60f, 0.82f, 0.96f);     // Daytime sky color (light blue)
glm::vec3 sunsetColor(0.96f, 0.64f, 0.38f);  // Sunset color (orange-red)

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
    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Model loading and synthetic camera", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Make the window's context current
    glfwMakeContextCurrent(window);
    glfwGetFramebufferSize(window, &screenWidth, &screenHeight);

    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK)
    {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return EXIT_FAILURE;
    }

    // Set viewport dimensions
    glViewport(0, 0, screenWidth, screenHeight);

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Create and compile shaders
    Shader shader("Shader/modelLoading.vs", "Shader/modelLoading.frag");

    // Load 3D models
    Model dog((char*)"Models/casa.obj");
    Model Piso((char*)"Models/piso.obj");

    // Initialize projection matrix
    glm::mat4 projection = glm::perspective(glm::radians(fov),
        (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
    glm::vec3 color = glm::vec3(0.0f, 0.0f, 1.0f);

    // Set GLFW callbacks
    glfwSetCursorPosCallback(window, MouseCallback);
    glfwSetScrollCallback(window, ScrollCallback);
    glfwSetMouseButtonCallback(window, MouseButtonCallback);

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

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use shader program
        shader.Use();

        // Update projection matrix if FOV changed
        projection = glm::perspective(glm::radians(fov),
            (GLfloat)screenWidth / (GLfloat)screenHeight, 0.1f, 100.0f);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"),
            1, GL_FALSE, glm::value_ptr(projection));

        // Create view matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"),
            1, GL_FALSE, glm::value_ptr(view));

        // Draw dog model
        glm::mat4 model(1);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(1.1f, 1.1f, 1.1f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(model));
        dog.Draw(shader);

        // Draw floor model
        glm::mat4 pisoModel = glm::mat4(1.0f);
        pisoModel = glm::translate(model, glm::vec3(0.0f, 0.34f, 0.0f));
        pisoModel = glm::scale(pisoModel, glm::vec3(5.0f, 1.0f, 5.0f));
        glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"),
            1, GL_FALSE, glm::value_ptr(pisoModel));
        Piso.Draw(shader);

        // Swap buffers
        glfwSwapBuffers(window);
    }

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

    // T key detection for sunset effect
    bool keyTPressedThisFrame = (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS);
    if (keyTPressedThisFrame && !keyTPressedLastFrame) {
        isSunsetActive = true;
    }
    keyTPressedLastFrame = keyTPressedThisFrame;
}

// Mouse movement callback
void MouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    // Calculate mouse offset
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // Reversed y-coordinates
    lastX = xpos;
    lastY = ypos;

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