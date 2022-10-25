#include "Falcor.h"
#include <vector>
#include <dlib/matrix.h>
#include <dlib/optimization.h>
#include <dlib/global_optimization.h>


using namespace Falcor;

class TorusFitting {

public: 
    std::vector<float3> torus_points;

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

        torus_points = points;

        return points;
    }

    std::vector<float3> getNewPoints(int n) {
        return getTorusPoints(n, 0.3f, 1.0f);
    }

    dlib::matrix<float, 14, 14> getPMatrix(std::vector<float3> points) {
        dlib::matrix<float, 14, 1> P;

        const size_t s = points.size();

        dlib::matrix<float, 14, 14> Psum;
        for (int i = 0; i < s; i++) {
            float3 p = points[i];
            P(0) = (p.x * p.x + p.y * p.y + p.z * p.z) * (p.x * p.x + p.y * p.y + p.z * p.z);
            P(1) = 4 * p.x * (p.x * p.x + p.y * p.y + p.z * p.z);
            P(2) = 4 * p.y * (p.x * p.x + p.y * p.y + p.z * p.z);
            P(3) = 4 * p.z * (p.x * p.x + p.y * p.y + p.z * p.z);
            P(4) = p.x * p.x;
            P(5) = p.y * p.y;
            P(6) = p.z * p.z;
            P(7) = 2 * p.x * p.y;
            P(8) = 2 * p.x * p.z;
            P(9) = 2 * p.y * p.z;
            P(10) = 2 * p.x;
            P(11) = 2 * p.y;
            P(12) = 2 * p.z;
            P(13) = 1;

            Psum += P * trans(P);
        }
        return Psum;
    }

    double fToMinimise(dlib::matrix<float, 9, 1> t) {
        dlib::matrix<float, 14, 0> s;
        s(0) = t(3);
        s(1) = -t(3) * t(4);
        s(2) = -t(3) * t(5);
        s(3) = -t(3) * t(6);
        s(4) = 4 * t(3) * t(4) * t(4) + t(0) * t(0) + t(7);
        s(5) = 4 * t(3) * t(5) * t(5) + t(1) * t(1) + t(7);
        s(6) = 4 * t(3) * t(6) * t(6) + t(2) * t(2) + t(7);
        s(7) = 4 * t(3) * t(4) * t(5) + t(0) * t(1);
        s(8) = 4 * t(3) * t(4) * t(6) + t(0) * t(2);
        s(9) = 4 * t(3) * t(5) * t(6) + t(1) * t(2);
        s(10) = -t(4) * t(7);
        s(11) = -t(5) * t(7);
        s(12) = -t(6) * t(7);
        s(13) = t(8);

        dlib::matrix<float, 14, 14> Psum = getPMatrix(torus_points);

        return trans(s) * Psum * s;
    }

    void fitTorus() {

        dlib::matrix<float, 9, 1> start(0);

        /*dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
            dlib::objective_delta_stop_strategy(1e-7),
            &TorusFitting::fToMinimise, start, -1);*/
    }

};


