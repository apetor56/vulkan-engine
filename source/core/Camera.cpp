#include "Camera.hpp"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <GLFW/glfw3.h>
#include <iostream>

namespace ve {

void Camera::update( float deltaTime ) {
    static constexpr float velocityFactor{ 0.02F };
    m_position += glm::vec3{ getRotationMatrix() * glm::vec4{ m_velocity * velocityFactor, 0.0F } } * deltaTime;
}

glm::mat4 Camera::getViewMartix() const {
    const glm::mat4 cameraTranslation{ glm::translate( glm::mat4( 1.0F ), m_position ) };
    const glm::mat4 cameraRotation{ getRotationMatrix() };
    return glm::inverse( cameraTranslation * cameraRotation );
}

glm::mat4 Camera::getRotationMatrix() const {
    const glm::quat pitchRotation{ glm::angleAxis( m_pitch, glm::vec3{ 1.0F, 0.0F, 0.0F } ) };
    const glm::quat yawRotation{ glm::angleAxis( m_yaw, glm::vec3{ 0.0F, -1.0F, 0.0F } ) };

    return glm::toMat4( yawRotation ) * glm::toMat4( pitchRotation );
}

void Camera::processKey( int key, int action ) {
    if ( action == GLFW_PRESS ) {
        if ( key == GLFW_KEY_W )
            m_velocity.z = -1.0F;

        if ( key == GLFW_KEY_S )
            m_velocity.z = 1.0F;

        if ( key == GLFW_KEY_A )
            m_velocity.x = -1.0F;

        if ( key == GLFW_KEY_D )
            m_velocity.x = 1.0F;
    }

    if ( action == GLFW_RELEASE ) {
        if ( key == GLFW_KEY_W )
            m_velocity.z = 0.0F;

        if ( key == GLFW_KEY_S )
            m_velocity.z = 0.0F;

        if ( key == GLFW_KEY_A )
            m_velocity.x = 0.0F;

        if ( key == GLFW_KEY_D )
            m_velocity.x = 0.0F;
    }
}

void Camera::processMouse( double xpos, double ypos, int button, int mode ) {
    static constexpr float sensitivity = 1.0f / 200.0f;
    static double lastX                = xpos;
    static double lastY                = ypos;

    double xoffset = xpos - lastX;
    double yoffset = ypos - lastY;

    lastX = xpos;
    lastY = ypos;

    if ( button == GLFW_MOUSE_BUTTON_LEFT && mode == GLFW_PRESS ) {
        m_yaw += static_cast< float >( xoffset ) * sensitivity;
        m_pitch -= static_cast< float >( yoffset ) * sensitivity;
    }
}

} // namespace ve
