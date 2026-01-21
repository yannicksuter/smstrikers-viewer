#include "camera.h"
#include <glm/gtc/constants.hpp>
#include <algorithm>

namespace SMStrikers {

Camera::Camera()
    : m_target(0.0f, 0.0f, 0.0f)
    , m_distance(10.0f)
    , m_yaw(glm::quarter_pi<float>())
    , m_pitch(glm::quarter_pi<float>())
    , m_fov(45.0f)
    , m_nearPlane(0.1f)
    , m_farPlane(1000.0f)
    , m_rotationSpeed(0.005f)
    , m_panSpeed(0.01f)
    , m_zoomSpeed(0.1f)
{
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(getPosition(), m_target, glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::getProjectionMatrix(float aspect) const {
    return glm::perspective(glm::radians(m_fov), aspect, m_nearPlane, m_farPlane);
}

void Camera::rotate(float deltaX, float deltaY) {
    m_yaw += deltaX * m_rotationSpeed;
    m_pitch += deltaY * m_rotationSpeed;
    
    // Clamp pitch to avoid gimbal lock
    m_pitch = std::clamp(m_pitch, -glm::half_pi<float>() + 0.1f, glm::half_pi<float>() - 0.1f);
}

void Camera::pan(float deltaX, float deltaY) {
    glm::vec3 right = glm::normalize(glm::cross(
        getPosition() - m_target,
        glm::vec3(0.0f, 1.0f, 0.0f)
    ));
    glm::vec3 up = glm::normalize(glm::cross(right, getPosition() - m_target));
    
    m_target += right * deltaX * m_panSpeed * m_distance;
    m_target += up * deltaY * m_panSpeed * m_distance;
}

void Camera::zoom(float delta) {
    m_distance -= delta * m_zoomSpeed * m_distance;
    m_distance = std::clamp(m_distance, 0.5f, 100.0f);
}

void Camera::reset() {
    m_target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_distance = 10.0f;
    m_yaw = glm::quarter_pi<float>();
    m_pitch = glm::quarter_pi<float>();
}

glm::vec3 Camera::getPosition() const {
    float x = m_target.x + m_distance * cos(m_pitch) * cos(m_yaw);
    float y = m_target.y + m_distance * sin(m_pitch);
    float z = m_target.z + m_distance * cos(m_pitch) * sin(m_yaw);
    return glm::vec3(x, y, z);
}

} // namespace SMStrikers
