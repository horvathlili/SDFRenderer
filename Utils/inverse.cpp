#include "inverse.h"

float4x4 sqr(float4x4 m) {
    float4x4 s;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            s[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                s[i][j] += m[i][k] * m[k][j];
            }
        }
    }

    return s;
}

float4x4 cube(float4x4 m) {
    float4x4 ms = sqr(m);

    float4x4 s;

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            s[i][j] = 0;
            for (int k = 0; k < 4; k++) {
                s[i][j] += ms[i][k] * m[k][j];
            }
        }
    }

    return s;
}

float trace(float4x4 m) {
    float tr = 0;
    for (int i = 0; i < 4; i++) {
        tr += m[i][i];
    }
    return tr;
}

float det2(float a, float b, float c, float d) {
    return a * d - b * c;
}

float det3(float3x3 m) {
    return m[0][0] * det2(m[1][1], m[1][2], m[2][1], m[2][2])
        - m[0][1] * det2(m[1][0], m[1][2], m[2][0], m[2][2])
        + m[0][2] * det2(m[1][0], m[1][1], m[2][0], m[2][1]);
}


float det(float4x4 m) {
    float m1 = m[0][0] * det3(float3x3{ m[1][1],m[1][2],m[1][3],m[2][1],m[2][2],m[2][3],m[3][1],m[3][2],m[3][3] });
    float m2 = m[0][1] * det3(float3x3{ m[1][0],m[1][2],m[1][3],m[2][0],m[2][2],m[2][3],m[3][0],m[3][2],m[3][3] });
    float m3 = m[0][2] * det3(float3x3{ m[1][0],m[1][1],m[1][3],m[2][0],m[2][1],m[2][3],m[3][0],m[3][1],m[3][3] });
    float m4 = m[0][3] * det3(float3x3{ m[1][0],m[1][1],m[1][2],m[2][0],m[2][1],m[2][2],m[3][0],m[3][1],m[3][2] });

    return m1 - m2 + m3 - m4;
}

float4x4 inverse(float4x4 m) {

    float4x4 i = { 1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1 };

    return 1 / det(m) *
        (1 / 6.0f * (trace(m) * trace(m) * trace(m) - 3 * trace(m) * trace(sqr(m)) + 2 * trace(cube(m))) * i
            - 0.5f * m * (trace(m) * trace(m) - trace(sqr(m))) + sqr(m) * trace(m) - cube(m));
}
