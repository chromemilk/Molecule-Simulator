#define GLM_ENABLE_EXPERIMENTAL
#include "Camera.h"
#include <glm/gtx/norm.hpp>

constexpr float FLOOR_Y = -0.2f;  // Same as grid plane Y level
constexpr float CAMERA_CLEARANCE = 0.3f; // How high the camera must stay above floor

Camera::Camera( glm::vec3 position )
    : Front( glm::vec3( 0.0f, 0.0f, -1.0f ) ),
    MovementSpeed( 5.0f ),
    MouseSensitivity( 0.1f ),
    Zoom( 45.0f ),
    WorldUp( glm::vec3( 0.0f, 1.0f, 0.0f ) ),
    Yaw( -90.0f ),
    Pitch( 0.0f ) {
    Position = position;
    updateCameraVectors();
}

glm::mat4 Camera::GetViewMatrix() const {
    return glm::lookAt( Position, Position + Front, Up );
}

void Camera::ProcessKeyboard( Camera_Movement direction, float deltaTime ) {
    float velocity = MovementSpeed * deltaTime;
    if (direction == Camera_Movement::FORWARD)
        Position += Front * velocity;
    if (direction == Camera_Movement::BACKWARD)
        Position -= Front * velocity;
    if (direction == Camera_Movement::LEFT)
        Position -= Right * velocity;
    if (direction == Camera_Movement::RIGHT)
        Position += Right * velocity;

    if (Position.y < FLOOR_Y + CAMERA_CLEARANCE)
        Position.y = FLOOR_Y + CAMERA_CLEARANCE;
}

void Camera::ProcessMouseMovement( float xoffset, float yoffset ) {
    xoffset *= MouseSensitivity;
    yoffset *= MouseSensitivity;

    Yaw += xoffset;
    Pitch += yoffset;

    // Clamp pitch angle to avoid flipping
    if (Pitch > 89.0f)
        Pitch = 89.0f;
    if (Pitch < -89.0f)
        Pitch = -89.0f;

    updateCameraVectors();
}

void Camera::ProcessMouseScroll( float yoffset ) {
    Zoom -= yoffset;
    if (Zoom < 1.0f)
        Zoom = 1.0f;
    if (Zoom > 45.0f)
        Zoom = 45.0f;
}

void Camera::updateCameraVectors() {
    glm::vec3 front;
    front.x = cos( glm::radians( Yaw ) ) * cos( glm::radians( Pitch ) );
    front.y = sin( glm::radians( Pitch ) );
    front.z = sin( glm::radians( Yaw ) ) * cos( glm::radians( Pitch ) );
    Front = glm::normalize( front );

    Right = glm::normalize( glm::cross( Front, WorldUp ) );
    Up = glm::normalize( glm::cross( Right, Front ) );
}


void Camera::FollowTargetEuler( const glm::vec3 &target, float deltaTime, float speed, float deadZoneAngle ) {
    glm::vec3 dir = glm::normalize( target - Position );
    if (glm::length2( dir ) < 1e-6f) return;  // too close to compute

    float desiredYaw = glm::degrees( atan2( dir.z, dir.x ) );
    float desiredPitch = glm::degrees( asin( dir.y ) );

    auto shortestAngle = [&]( float from, float to ) {
        float d = to - from;
        d = fmod( d + 540.0f, 360.0f ) - 180.0f;
        return d;
        };

    float deltaYaw = shortestAngle( Yaw, desiredYaw );
    float deltaPitch = shortestAngle( Pitch, desiredPitch );

    if (fabs( deltaYaw ) < deadZoneAngle &&
        fabs( deltaPitch ) < deadZoneAngle)
        return;

    float t = glm::clamp( speed * deltaTime, 0.0f, 1.0f );
    Yaw += deltaYaw * t;
    Pitch += deltaPitch * t;

    Pitch = glm::clamp( Pitch, -89.0f, +89.0f );

    updateCameraVectors();
}