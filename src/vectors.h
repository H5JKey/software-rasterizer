#ifndef VECTORS_H
#define VECTORS_H

// Минимальный набор векторных типов, нужный утилитам шаблона.
// Остальную математику (операции над векторами, матрицы) вы пишете сами.

typedef struct
{
    union
    {
        struct
        {
            float x;
            float y;
        };
        float M[2];
    };
} vec2;
typedef struct
{
    union
    {
        struct
        {
            float x;
            float y;
            float z;
        };
        float M[3];
    };
} vec3;

static inline vec2 make2(float x, float y)
{
    vec2 v;
    v.x = x;
    v.y = y;
    return v;
}

static inline vec3 make3(float x, float y, float z)
{
    vec3 v;
    v.x = x;
    v.y = y;
    v.z = z;
    return v;
}

#endif
