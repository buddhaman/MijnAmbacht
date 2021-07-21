#ifndef MATH_2D_HEADER
#define MATH_2D_HEADER

#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef union 
{
    float m[3][3];
    struct {
        float m00, m01, m02;
        float m10, m11, m12;
        float m20, m21, m22;
    };
} mat3_t;

static inline mat3_t mat3(
        float m00, float m10, float m20,
        float m01, float m11, float m21,
        float m02, float m12, float m22
        )
{
    return (mat3_t) {
        .m[0][0] = m00, .m[1][0] = m10, .m[2][0] = m20,
        .m[0][1] = m01, .m[1][1] = m11, .m[2][1] = m21,
        .m[0][2] = m02, .m[1][2] = m12, .m[2][2] = m22 
    };
}

static inline vec2_t  v2_add(vec2_t a, vec2_t b)    { return (vec2_t){a.x+b.x, a.y+b.y}; }
static inline vec2_t  v2_sub(vec2_t a, vec2_t b)    { return (vec2_t){a.x-b.x, a.y-b.y}; }
static inline vec2_t  v2_mul(vec2_t a, vec2_t b)    { return (vec2_t){a.x*b.x, a.y*b.y}; }

static inline vec2_t  v2_adds(vec2_t a, float s)    { return (vec2_t){a.x+s, a.y+s}; }
static inline vec2_t  v2_subs(vec2_t a, float s)    { return (vec2_t){a.x-s, a.y-s}; }
static inline vec2_t  v2_muls(vec2_t a, float s)    { return (vec2_t){a.x*s, a.y*s}; }

static inline float   v2_dot(vec2_t a, vec2_t b)    { return a.x*b.x + a.y*b.y; }
static inline float   v2_length(vec2_t v)           { return sqrtf(v.x*v.x + v.y*v.y); }
static inline float   v2_length2(vec2_t v)           { return v.x*v.x + v.y*v.y; }
static inline float   v2_dist(vec2_t a, vec2_t b)   { return v2_length(v2_sub(b, a)); }

static inline vec2_t  v2_norm(vec2_t v)             { return v2_muls(v, 1.0/v2_length(v)); }

static inline vec2_t v2_rotate(vec2_t v, float t)   
{ 
    float c = cosf(t);
    float s = sinf(t);
    return vec2(c*v.x - s*v.y, s*v.x+c*v.y);
}

static inline vec2_t v2_lerp(vec2_t from, vec2_t to, float s)
{
    vec2_t diff = v2_sub(to, from);
    return v2_add(from, v2_muls(diff, s));
}

mat3_t m3_translation(vec2_t translation)
{
    return mat3(
            1,0,translation.x,
            0,1,translation.y,
            0,0,1
            );
}

mat3_t m3_translation_and_scale(vec2_t translation, float scaleX, float scaleY)
{
    return mat3(
            scaleX, 0,translation.x*scaleX,
            0,scaleY, translation.y*scaleY,
            0,0,1
            );
}

vec2_t m3_mul_pos(mat3_t matrix, vec2_t position)
{
    vec2_t result = vec2(
            matrix.m00*position.x + matrix.m10*position.y + matrix.m20,
            matrix.m01*position.x + matrix.m11*position.y + matrix.m21
            );
    float w = matrix.m02 *position.x + matrix.m12*position.y + matrix.m22;
    if(w!=0 && w!=1)
    {
        return vec2(result.x/w, result.y/w);
    }
    else
    {
        return result;
    }
}

void m3_fprintp(FILE* stream, mat3_t matrix, int width, int precision) {
	mat3_t m = matrix;
	int w = width, p = precision;
	for(int r = 0; r < 3; r++) {
		fprintf(stream, "| %*.*f %*.*f %*.*f |\n",
			w, p, m.m[0][r], w, p, m.m[1][r], w, p, m.m[2][r]
		);
	}
}


#endif
