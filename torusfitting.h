#include "Falcor.h"
#include <vector>
//#include <dlib/image_processing.h>


using namespace Falcor;


std::vector<float3> getTorusPoints(int n, float r, float R) {
    std::vector<float3> points;
    for (int i = 0; i < n; i++) {
        float u = (float)((float)rand() / (float)(RAND_MAX) * 2.0f * M_PI);
        float v = (float)((float)rand() / (float)(RAND_MAX) * 2.0f * M_PI);
      
        points.push_back(float3(
            (R + r * cos(u)) * cos(v),
            r * sin(u),
            (R + r * cos(u)) * sin(v)
        ));
    };
    return points;
}

std::vector<float3> getNewPoints(int n) {
    return getTorusPoints(n, 0.3f, 1.0f);
}
