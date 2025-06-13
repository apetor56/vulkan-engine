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

private:
    glm::vec3 m_position{ -9.24F, 1.37F, 0.48F };
    glm::vec3 m_velocity{};
    float m_pitch{ 0.03F };
    float m_yaw{ -23.86F };

    glm::mat4 getRotationMatrix() const;
};

} // namespace ve
