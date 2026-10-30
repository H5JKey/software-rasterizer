export module scene.camera;

import math;

export class Camera {
    vec3 forward;
    vec3 up;
    vec3 eye;
    vec3 right;

    constexpr void buildCoordinateSystem() {
        mat4 R = rotationY(yaw) * rotationX(pitch);
        forward = vec3(R[0, 2], R[1, 2], R[2, 2]);
        up = vec3(R[0, 1], R[1, 1], R[2, 1]);
        right = vec3(R[0, 0], R[1, 0], R[2, 0]);
    }

   public:
    float fov;
    float aspect;
    float zNear;
    float zFar;

    float pitch, yaw;
    vec3 velocity;

    constexpr Camera(vec3 eye, float pitch, float yaw, float fov, float aspect, float zNear, float zFar)
        : eye(eye), pitch(pitch), yaw(yaw), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {
        buildCoordinateSystem();
    }

    constexpr mat4 getLookAtMatrix() const { return lookAt(eye, forward, up); }
    constexpr mat4 getPerspectiveMatrix() const { return perspective(fov, aspect, zNear, zFar); }

    void update(double dt);
};