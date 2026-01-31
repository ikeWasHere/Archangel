#include <glad/glad.h>
#include <SFML/Window.hpp>

#include <iostream>
#include <optional>

const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

int main()
{
    sf::ContextSettings settings;
    settings.majorVersion = 3;
    settings.minorVersion = 3;
    settings.attributeFlags = sf::ContextSettings::Core;

    sf::Window window(sf::VideoMode({1280, 720}), "OpenGL with SFML",
                            sf::Style::Default, sf::State::Windowed, settings);
    window.setActive(true);

   if (!gladLoadGLLoader((GLADloadproc)sf::Context::getFunction))
    return -1;

    // vertex data
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f
    };

    // create and bind VAO
    unsigned int VAO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);
    
    // storing vertex data in gpu memory
    unsigned int VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // vertex shader
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
   
    // check if shader compiled
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    // fragment shader
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    // check if shader compiled
    int fragSuccess;
    char fragInfoLog[512];
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &fragSuccess);
    if (!fragSuccess)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, fragInfoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << fragInfoLog << std::endl;
    }

    // shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    // check if program compiled
    int progSuccess;
    char progInfoLog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &progSuccess);
    if (!progSuccess)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::PROGRAM::COMPILATION_FAILED\n" << progInfoLog << std::endl;
    }

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // linking vertex attributes to input data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    bool running = true;
    while (running)
    {
        while (const std::optional<sf::Event> event = window.pollEvent())
        {
            if (event ->is<sf::Event::Closed>())
            {
                running = false;
            }
            else if (const auto* resized = event->getIf<sf::Event::Resized>())
            {
                glViewport(0, 0, resized->size.x, resized->size.y);
            }
        }

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // draw triangle
        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        window.display();
    }
  
}
