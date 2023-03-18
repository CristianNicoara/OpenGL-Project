#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        //TODO - Update the rest of camera parameters
        this->cameraFrontDirection = glm::normalize(cameraTarget - cameraPosition);
        this->cameraRightDirection = -glm::normalize(glm::cross(cameraUp, cameraFrontDirection));
        this->originalUp = cameraUp;
        this->originalFront = cameraFrontDirection;
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraTarget, cameraUpDirection);
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction, float speed) {
        //TODO
        glm::vec3 newPos;
        if (direction == MOVE_FORWARD) {
            newPos = cameraPosition + speed * cameraFrontDirection;
        }
        if (direction == MOVE_BACKWARD) {
            newPos = cameraPosition + speed * (- cameraFrontDirection);
        }
        if (direction == MOVE_LEFT) {
            newPos = cameraPosition + speed * (-cameraRightDirection);
        }
        if (direction == MOVE_RIGHT) {
            newPos = cameraPosition + speed * cameraRightDirection;
        }
        if (newPos.y >= 0.5f && newPos.x <= 189.0f && newPos.x >= -187.0f && newPos.z <= 189.0f && newPos.z >= -188.5f)
            cameraPosition = newPos;
        cameraTarget = cameraPosition + cameraFrontDirection;
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        //TODO
        glm::mat4 eulerAngle = glm::yawPitchRoll(glm::radians(yaw), glm::radians(pitch), 0.0f);

        cameraFrontDirection = glm::vec3(glm::normalize((eulerAngle * glm::vec4(originalFront, 0.0f))));
        cameraRightDirection = glm::normalize(glm::cross(cameraFrontDirection, originalUp));
        cameraUpDirection = glm::cross(cameraRightDirection, cameraFrontDirection);
        cameraTarget = cameraPosition + cameraFrontDirection;
    }
    void Camera::pposition()
    {
        printf("X: %f y: %f z: %f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z);
    }
}