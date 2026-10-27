export module core.camera;

import math.vectors;
import math.matrices;
import math.transforms;

export class Camera {
   public:
    vec3 eye;
    vec3 target;
    vec3 up;
    float fov;
    float aspect;
    float zNear;
    float zFar;

    constexpr Camera(vec3 eye, vec3 target, vec3 up, float fov, float aspect, float zNear, float zFar)
        : eye(eye), target(target), up(up), fov(fov), aspect(aspect), zNear(zNear), zFar(zFar) {}

    mat4 getLookAtMatrix() const { return lookAt(eye, target, up); }
    mat4 getPerspectiveMatrix() const { return perspective(fov, aspect, zNear, zFar); }
};