#include <glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <iostream>
#include <vector>

#include "LoadShaders.h"
#include "camera.h"

struct IndirectRenderParams
{
    GLuint count;
    GLuint primCount;
    GLuint firstIndex;
    GLuint baseVertex;
    GLuint baseInstance;
};

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

const int SCREEN_WIDTH = 800;
const int SCREEN_HEIGHT = 600;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;
 
// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

glm::vec3 lightPos(5.0f, 1.0f, 2.0f);

int num_blades = 100;

struct Blade
{
    glm::vec3 v0;
    glm::vec3 v1;
    glm::vec3 v2;

    glm::vec3 up;
    glm::vec3 dir;
    float height;
    float width;
};

std::vector<Blade> blades;

float zTrans = 1.77f;
float xTrans = 1.77f;
float qScale = 1.77f;

int main(void)
{
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program;
    GLint mvp_location, vpos_location, vcol_location;
  
    if (!glfwInit())
        exit(EXIT_FAILURE);
 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
 
    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }
  
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    glfwSwapInterval(1);

    glewInit();

    glEnable(GL_DEPTH_TEST);

    ShaderInfo light_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/light.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/light.fs"},
        {GL_NONE, NULL}
    };

    GLuint light_shader_program = LoadShaders(light_shader_info);

    printf("light shader program: %d \n", light_shader_program);

float verts[] = {
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f, 

    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,

     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,

    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,

    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f
};

    //light cube
    unsigned int lightVAO, lightVBO;
    glGenVertexArrays(1, &lightVAO);
    glGenBuffers(1, &lightVBO);

    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //grass blades 
    ShaderInfo grass_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/grass.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/grass.fs"},
        {GL_TESS_CONTROL_SHADER, "../shaders/grass.tc"},
        {GL_TESS_EVALUATION_SHADER, "../shaders/grass.te"},
        {GL_NONE, NULL}
    };

    GLuint grass_shader_program = LoadShaders(grass_shader_info);

    printf("grass shader program: %d \n", grass_shader_program);

    std::vector<glm::vec4> grass_blade_positions;
    std::vector<glm::vec4> grass_blade_v1s;
    std::vector<glm::vec4> grass_blade_v2s;
    for (int i = 0; i < num_blades; i++)
    {
        float grass_height = 1.0f;
        float grass_width = 0.2f;
            
        int pos_max = -5;
        int pos_min = 5;
            
        float xCoord = (float)(std::rand() % ((pos_max - pos_min) + 1) + pos_min);
        float zCoord = (float)(std::rand() % ((pos_max - pos_min) + 1) + pos_min);

        glm::vec3 controlPoint0 = glm::vec3(xCoord, 0.0, zCoord);
        glm::vec3 controlPoint1 = controlPoint0 + glm::vec3(0, grass_height, 0);
        glm::vec3 controlPoint2 = controlPoint1;

        glm::vec3 grass_up = glm::vec3(0, 1.0f, 0);
        glm::vec3 grass_direction = glm::vec3(0.5f, 0, 0.5f);

        Blade _blade;
        _blade.dir = grass_direction;
        _blade.up = grass_up;
        _blade.v0 = controlPoint0;
        _blade.v1 = controlPoint1;
        _blade.v2 = controlPoint2;
        _blade.width = grass_width;
        _blade.height = grass_height;

        grass_blade_positions.push_back(glm::vec4(controlPoint0, 1.0f));
        grass_blade_v1s.push_back(glm::vec4(controlPoint1, 1.0f));
        grass_blade_v2s.push_back(glm::vec4(controlPoint2, 1.0f));

        blades.push_back(_blade);
    }

    unsigned int grassVAO, grassVBO, grassPosBuffer, grassV1Buffer, grassV2Buffer;
    glGenVertexArrays(1, &grassVAO);
    glGenBuffers(1, &grassVBO);
    glGenBuffers(1, &grassPosBuffer);
    glGenBuffers(1, &grassV1Buffer);
    glGenBuffers(1, &grassV2Buffer);


    glBindVertexArray(grassVAO);

    //Vertex Attribs
    //*****
    //1. Blade Position Location = 0
    //2. V1 Location = 1
    //2. V2 Location = 2
    //
    //*****
    glBindBuffer(GL_ARRAY_BUFFER, grassPosBuffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_positions.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassV1Buffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_v1s.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    glBindBuffer(GL_ARRAY_BUFFER, grassV2Buffer);
    glBufferData(GL_ARRAY_BUFFER, num_blades * sizeof(glm::vec4), grass_blade_v2s.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);//reset bound buffer

    // //Indirect----
    IndirectRenderParams indirectRenderParams = { (GLuint)num_blades, (GLuint)1, (GLuint)0, (GLuint)0, (GLuint)0 };
    unsigned int grassVBO_Indirect;
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grassVBO_Indirect);
    glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(IndirectRenderParams), &indirectRenderParams, GL_STATIC_DRAW);
    glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);//probably don't need this but to be safe keep it in.
    // //End Indirect----

    glPatchParameteri(GL_PATCH_VERTICES, 4);

    //ground plane
    float quad_vertices[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,
        -0.5f, 0.5f
    };

    ShaderInfo ground_plane_shader_info[] = {
        {GL_VERTEX_SHADER, "../shaders/ground.vs"},
        {GL_FRAGMENT_SHADER, "../shaders/ground.fs"},
        {GL_NONE, NULL}
    };

    GLuint ground_shader_program = LoadShaders(ground_plane_shader_info);

    printf("ground shader program: %d \n", ground_shader_program);

    unsigned int groundVAO, groundVBO;
    glGenVertexArrays(1, &groundVAO);
    glGenBuffers(1, &groundVBO);

    glBindVertexArray(groundVAO);
    glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        float ratio;
        int width, height;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;
 
        //draw
        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        //Display light cube
        glUseProgram(light_shader_program);

        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));

        glUniformMatrix4fv(glGetUniformLocation(light_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //End Display light cube

        //Display ground plane
        glUseProgram(ground_shader_program);

        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "view"), 1, GL_FALSE, &view[0][0]);
        
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(xTrans, 0, zTrans));
        model = glm::scale(model, glm::vec3(qScale));
        model = glm::rotate(model, 1.5708f, glm::vec3(1,0,0)); //1.5708 radians = 90 degrees

        glUniformMatrix4fv(glGetUniformLocation(ground_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glBindVertexArray(groundVAO);
        glDrawArrays(GL_QUADS, 0, 4);
        //End Display ground plane

        //Display tessellated quad
        glUseProgram(grass_shader_program);
        glBindVertexArray(grassVAO);
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, grassVBO_Indirect);

        Blade _blade = blades[0];
        //glUniform3fv(glGetUniformLocation(grass_shader_program, "up"), 1, &_blade.up[0]);
        glUniform3fv(glGetUniformLocation(grass_shader_program, "direction"), 1, &_blade.dir[0]);

        glUniform1f(glGetUniformLocation(grass_shader_program, "w"), _blade.width); 

        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "projection"), 1, GL_FALSE, &projection[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "view"), 1, GL_FALSE, &view[0][0]);

        model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.2f, 0.2f, 0.2f));

        glUniformMatrix4fv(glGetUniformLocation(grass_shader_program, "model"), 1, GL_FALSE, &model[0][0]);

        glDrawArraysIndirect(GL_PATCHES, 0);

        //glBindBuffer(GL_DRAW_INDIRECT_BUFFER, 0);
        //glBindVertexArray(0);
        //End display tessellated quad

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
 
    glfwDestroyWindow(window);
 
    glfwTerminate();
    exit(EXIT_SUCCESS);
}

void processInput(GLFWwindow *window)
{
    float offset = 0.01f;

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
    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
    {
        xTrans += offset;
        printf("X: %f \n", xTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        xTrans -= offset;
        printf("X: %f \n", xTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        zTrans += offset;
        printf("Z: %f \n", zTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS)
    {
        zTrans -= offset;
        printf("Z: %f \n", zTrans);
    }
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
    {
        qScale += offset;
        printf("qscale: %f \n", qScale);
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
    {
        qScale -= offset;
        printf("qscale: %f \n", qScale);
    }

        
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