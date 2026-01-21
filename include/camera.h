#ifndef SMSTRIKERS_CAMERA_H
#define SMSTRIKERS_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace SMStrikers {

/**
 * @brief Orbital camera for 3D viewport
 * 
 * Supports orbit, pan, and zoom controls similar to Blender/Maya
 */
class Camera {
public:
    Camera();

    /**
     * @brief Get the view matrix
     */
    glm::mat4 getViewMatrix() const;

    /**
     * @brief Get the projection matrix
     */
    glm::mat4 getProjectionMatrix(float aspect) const;

    /**
     * @brief Rotate camera (orbit around target)
     * @param deltaX Horizontal rotation delta
     * @param deltaY Vertical rotation delta
     */
    void rotate(float deltaX, float deltaY);

    /**
     * @brief Pan camera (move target point)
     * @param deltaX Horizontal pan delta
     * @param deltaY Vertical pan delta
     */
    void pan(float deltaX, float deltaY);

    /**
     * @brief Zoom camera (change distance)
     * @param delta Zoom delta (positive = zoom in)
     */
    void zoom(float delta);

    /**
     * @brief Reset camera to default position
     */
    void reset();

    // Getters
    glm::vec3 getPosition() const;
    glm::vec3 getTarget() const { return m_target; }
    float getDistance() const { return m_distance; }

private:
    void updatePosition();

    glm::vec3 m_target;      // Point camera is looking at
    float m_distance;         // Distance from target
    float m_yaw;             // Horizontal rotation (radians)
    float m_pitch;           // Vertical rotation (radians)
    
    float m_fov;             // Field of view
    float m_nearPlane;
    float m_farPlane;
    
    float m_rotationSpeed;
    float m_panSpeed;
    float m_zoomSpeed;
};

} // namespace SMStrikers

#endif // SMSTRIKERS_CAMERA_H
