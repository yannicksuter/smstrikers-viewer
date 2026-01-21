#include "mesh.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace SMStrikers {

// ============================================================================
// Mesh Implementation
// ============================================================================

Mesh::Mesh()
    : m_vao(0)
    , m_vbo(0)
    , m_ebo(0)
    , m_indexCount(0)
{
}

Mesh::~Mesh() {
    cleanup();
}

Mesh* Mesh::createCube(float size) {
    float halfSize = size * 0.5f;
    
    // Vertex data: position (3) + normal (3) + color (3)
    std::vector<float> vertices = {
        // Front face (red)
        -halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.2f, 0.2f,
         halfSize, -halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.2f, 0.2f,
         halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.2f, 0.2f,
        -halfSize,  halfSize,  halfSize,  0.0f,  0.0f,  1.0f,  1.0f, 0.2f, 0.2f,
        
        // Back face (green)
        -halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.2f, 1.0f, 0.2f,
         halfSize, -halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.2f, 1.0f, 0.2f,
         halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.2f, 1.0f, 0.2f,
        -halfSize,  halfSize, -halfSize,  0.0f,  0.0f, -1.0f,  0.2f, 1.0f, 0.2f,
        
        // Top face (blue)
        -halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  0.3f, 0.5f, 1.0f,
         halfSize,  halfSize, -halfSize,  0.0f,  1.0f,  0.0f,  0.3f, 0.5f, 1.0f,
         halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  0.3f, 0.5f, 1.0f,
        -halfSize,  halfSize,  halfSize,  0.0f,  1.0f,  0.0f,  0.3f, 0.5f, 1.0f,
        
        // Bottom face (yellow)
        -halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.2f,
         halfSize, -halfSize, -halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.2f,
         halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.2f,
        -halfSize, -halfSize,  halfSize,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f, 0.2f,
        
        // Right face (cyan)
         halfSize, -halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.2f, 1.0f, 1.0f,
         halfSize,  halfSize, -halfSize,  1.0f,  0.0f,  0.0f,  0.2f, 1.0f, 1.0f,
         halfSize,  halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.2f, 1.0f, 1.0f,
         halfSize, -halfSize,  halfSize,  1.0f,  0.0f,  0.0f,  0.2f, 1.0f, 1.0f,
        
        // Left face (magenta)
        -halfSize, -halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.2f, 1.0f,
        -halfSize,  halfSize, -halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.2f, 1.0f,
        -halfSize,  halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.2f, 1.0f,
        -halfSize, -halfSize,  halfSize, -1.0f,  0.0f,  0.0f,  1.0f, 0.2f, 1.0f,
    };
    
    std::vector<unsigned int> indices = {
        0, 2, 1, 0, 3, 2,       // Front (CCW from outside)
        6, 5, 4, 7, 6, 4,       // Back (CCW from outside)
        8, 10, 9, 8, 11, 10,    // Top (CCW from outside)
        14, 13, 12, 15, 14, 12, // Bottom (CCW from outside)
        18, 17, 16, 19, 18, 16, // Right (CCW from outside)
        22, 21, 20, 23, 22, 20  // Left (CCW from outside)
    };
    
    Mesh* mesh = new Mesh();
    mesh->initialize(vertices, indices);
    return mesh;
}

void Mesh::initialize(const std::vector<float>& vertices, const std::vector<unsigned int>& indices) {
    m_indexCount = indices.size();
    
    // Generate and bind VAO
    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);
    
    // Generate and bind VBO
    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    // Generate and bind EBO
    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Color attribute
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Unbind
    glBindVertexArray(0);
}

void Mesh::render() {
    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, m_indexCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Mesh::cleanup() {
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
}

// ============================================================================
// Shader Implementation
// ============================================================================

Shader::Shader()
    : m_program(0)
{
}

Shader::~Shader() {
    if (m_program) {
        glDeleteProgram(m_program);
    }
}

bool Shader::createBasicShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec3 aColor;
        
        out vec3 FragPos;
        out vec3 Normal;
        out vec3 Color;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            Normal = mat3(transpose(inverse(model))) * aNormal;
            Color = aColor;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 FragPos;
        in vec3 Normal;
        in vec3 Color;
        
        out vec4 FragColor;
        
        uniform vec3 lightPos;
        uniform vec3 viewPos;
        
        void main() {
            // Stronger ambient for better visibility
            float ambientStrength = 0.7;
            vec3 ambient = ambientStrength * Color;
            
            // Diffuse
            vec3 norm = normalize(Normal);
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * Color * 0.8;
            
            // Specular
            float specularStrength = 0.3;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, norm);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * vec3(1.0);
            
            vec3 result = ambient + diffuse + specular;
            FragColor = vec4(result, 1.0);
        }
    )";
    
    // Compile shaders
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (!vertexShader) return false;
    
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Link program
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader program linking failed\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

bool Shader::createUnlitShader() {
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec3 aNormal;
        layout (location = 2) in vec3 aColor;
        
        out vec3 Color;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            Color = aColor;
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        in vec3 Color;
        
        out vec4 FragColor;
        
        void main() {
            // Output the vertex color (albedo) without lighting
            FragColor = vec4(Color, 1.0);
        }
    )";
    
    // Compile shaders
    GLuint vertexShader = compileShader(vertexShaderSource, GL_VERTEX_SHADER);
    if (!vertexShader) return false;
    
    GLuint fragmentShader = compileShader(fragmentShaderSource, GL_FRAGMENT_SHADER);
    if (!fragmentShader) {
        glDeleteShader(vertexShader);
        return false;
    }
    
    // Link program
    m_program = glCreateProgram();
    glAttachShader(m_program, vertexShader);
    glAttachShader(m_program, fragmentShader);
    glLinkProgram(m_program);
    
    // Check for linking errors
    GLint success;
    glGetProgramiv(m_program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(m_program, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader program linking failed\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }
    
    // Clean up shaders (they're linked into the program now)
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    return true;
}

GLuint Shader::compileShader(const char* source, GLenum type) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    // Check for compilation errors
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "ERROR: Shader compilation failed (" 
                  << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") 
                  << ")\n" << infoLog << std::endl;
        glDeleteShader(shader);
        return 0;
    }
    
    return shader;
}

void Shader::use() {
    glUseProgram(m_program);
}

void Shader::setMat4(const char* name, const glm::mat4& mat) {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name), 1, GL_FALSE, glm::value_ptr(mat));
}

void Shader::setVec3(const char* name, const glm::vec3& vec) {
    glUniform3fv(glGetUniformLocation(m_program, name), 1, glm::value_ptr(vec));
}

} // namespace SMStrikers
