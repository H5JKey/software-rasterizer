module;
#include <print>

module scene.camera;
import math;

void Camera::update(double dt) {
    buildCoordinateSystem();
    vec3 worldVel = right * velocity.x + up * velocity.y + forward * velocity.z;

    eye += worldVel * dt * 20.f;

    const float damping = std::exp(-15.0f * dt);
    velocity *= damping;
    if (length(velocity) < 1e-4f) velocity = vec3(0, 0, 0);
}
