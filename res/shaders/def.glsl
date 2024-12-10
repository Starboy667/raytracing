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
    vec3 color;
    int sphereIndex;
};

const highp float pos_infinity = 3.402823466e+38;

#define M_PI 3.1415926535897932384626433832795

// float rand(vec2 st) {
//     return fract(sin(dot(st.xy,
//                          vec2(12.9898,78.233)))*
//         43758.5453123);
// }

// vec3 rand_vec3(float min, float max, vec2 st) {
//     return vec3(rand(st) * (max - min) + min, rand(st + vec2(1.0f, 0.0f)) * (max - min) + min, rand(st + vec2(0.0f, 1.0f)) * (max - min) + min);
// }

// PCG random number generator
uint wang_hash(uint seed)
{
    seed = (seed ^ 61u) ^ (seed >> 16u);
    seed *= 9u;
    seed = seed ^ (seed >> 4u);
    seed *= 0x27d4eb2du;
    seed = seed ^ (seed >> 15u);
    return seed;
}

float rand(vec2 pixel_coord, int frame_number, int sample_index) 
{
    uint seed = uint(pixel_coord.x) + 1920u * uint(pixel_coord.y) + 
                uint(frame_number) * 1920u * 1080u + 
                uint(sample_index) * 1920u * 1080u * 256u;
    return float(wang_hash(seed)) / 4294967296.0;
}

// Random vector on unit sphere
vec3 random_unit_vector(vec2 pixel_coord, int frame_number, int sample_index) 
{
    float z = rand(pixel_coord, frame_number, sample_index) * 2.0 - 1.0;
    float a = rand(pixel_coord, frame_number, sample_index + 1) * 2.0 * M_PI;
    float r = sqrt(1.0 - z * z);
    float x = r * cos(a);
    float y = r * sin(a);
    return vec3(x, y, z);
}

// Random vector in hemisphere oriented around normal
vec3 random_hemisphere_vector(vec3 normal, vec2 pixel_coord, int frame_number, int sample_index) 
{
    vec3 random_vec = random_unit_vector(pixel_coord, frame_number, sample_index);
    return normalize(random_vec * sign(dot(random_vec, normal)));
}