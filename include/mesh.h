#ifndef SMSTRIKERS_MESH_H
#define SMSTRIKERS_MESH_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

namespace SMStrikers {

/**
 * @brief Simple mesh for rendering 3D objects
 */
class Mesh {
public:
    Mesh();
    ~Mesh();

    /**
     * @brief Create a cube mesh
     */
    static Mesh* createCube(float size = 1.0f);

    /**
     * @brief Initialize mesh with vertex data
     */
    void initialize(const std::vector<float>& vertices, const std::vector<unsigned int>& indices);

    /**
     * @brief Render the mesh
     */
    void render();

    /**
     * @brief Get number of vertices
     */
    unsigned int getVertexCount() const;
    
    /**
     * @brief Get number of indices (for indexed rendering)
     */
    unsigned int getIndexCount() const { return m_indexCount; }
    
    /**
     * @brief Get number of triangles
     */
    unsigned int getTriangleCount() const { return m_indexCount / 3; }

    /**
     * @brief Clean up OpenGL resources
     */
    void cleanup();

private:
    GLuint m_vao;
    GLuint m_vbo;
    GLuint m_ebo;
    unsigned int m_indexCount;
    unsigned int m_vertexCount;
};

/**
 * @brief Simple shader program
 */
class Shader {
public:
    Shader();
    ~Shader();

    /**
     * @brief Create basic colored shader with lighting
     */
    bool createBasicShader();
    
    /**
     * @brief Create unlit colored shader
     */
    bool createUnlitShader();

    /**
     * @brief Use this shader
     */
    void use();

    /**
     * @brief Set uniform matrix
     */
    void setMat4(const char* name, const glm::mat4& mat);

    /**
     * @brief Set uniform vec3
     */
    void setVec3(const char* name, const glm::vec3& vec);

    /**
     * @brief Get program ID
     */
    GLuint getProgram() const { return m_program; }

private:
    GLuint m_program;
    
    GLuint compileShader(const char* source, GLenum type);
};

} // namespace SMStrikers

#endif // SMSTRIKERS_MESH_H
