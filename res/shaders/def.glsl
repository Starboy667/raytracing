struct Sphere {
    vec3 center;
    float radius;
    vec3 color;
};

struct Ray {
    vec3 origin;
    vec3 direction;
};

struct Camera {
    vec3 position;
    vec3 forwards;
    vec3 right;
    vec3 up;
};

struct RayHit {
    vec3 position;
    vec3 normal;
    float distance;
};

const highp float pos_infinity = 3.402823466e+38;

#define M_PI 3.1415926535897932384626433832795

float rand(vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}
