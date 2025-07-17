#pragma once

#include "utils/NonCopyable.hpp"
#include "utils/NonMovable.hpp"

#include <glm/vec3.hpp>
#include <glm/glm.hpp>

namespace ve {

class Camera : public utils::NonCopyable,
               public utils::NonMovable {
public:
    void update( float deltaTime );

    void processKey( int key, int action );
    void processMouse( double xpos, double ypos, int button, int mode );

    glm::mat4 getViewMartix() const;
    glm::vec3 getPosition() const noexcept { return m_position; }

private:
    glm::vec3 m_position{ 0.0F, 0.0F, 5.0F };
    glm::vec3 m_velocity{};
    float m_pitch{ 0.0F };
    float m_yaw{ 0.0F };
    float m_speed{ 0.7f };

    glm::mat4 getRotationMatrix() const;
};

} // namespace ve
