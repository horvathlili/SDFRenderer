#pragma once
#include "Falcor.h"

using namespace Falcor;

float4x4 sqr(float4x4 m);

float4x4 cube(float4x4 m);

float trace(float4x4 m);

float det2(float a, float b, float c, float d);
float det3(float3x3 m);
float det(float4x4 m);

float4x4 inverse(float4x4 m);
