#include "Falcor.h"
#include <vector>
#include <dlib/matrix.h>
#include <dlib/optimization.h>
#include <dlib/global_optimization.h>


using namespace Falcor;

class TorusFitting {

public: 
    std::vector<float4> torus_points;
    dlib::matrix<double, 8, 1> minimised_params;
  
    std::vector<float4> getTorusPoints(int n, float r, float R,float3 c, float3 d) {
        std::vector<float4> points;
        for (int i = 0; i < n; i++) {

            float4 p;
            p.x = float(rand())/float(RAND_MAX) * 5 - 2.5;
            p.y = float(rand()) / float(RAND_MAX) * 5 - 2.5;
            p.z = float(rand()) / float(RAND_MAX) * 5 - 2.5;
            p.w = getDistFromTorus(p.xyz, r, R, c, d);
            //p.w = sqrt(dot(p.xyz - c, p.xyz - c)) - 0.5;
            points.push_back(p);

        };

        torus_points = points;
        return points;
    }

    std::vector<float4> getNewPoints(int n) {
        return getTorusPoints(n, 0.1, 0.5,float3(0,0,0),float3(0,0,1));
    }

    float getDistFromTorus(float3 p, float r, float R, float3 c, float3 d) {
        float3 p2 = p - c;
        float3 e2 = normalize(d);
        float3 n = cross(e2, p2);

        float3 e1 = normalize(cross(e2, n));

        float px = dot(e1, p2);
        float py = dot(e2, p2);

        float dist1 = sqrt((px - R) * (px - R) + py * py) - r;
        float dist2 = sqrt((px + R) * (px + R) + py * py) - r;

        if (abs(dist1) <= abs(dist2))
            return dist1;
        else return dist2;
    }

    void fitTorus() {

        dlib::matrix<double, 8, 1> start = { 1,1,0.5,0.5,0.5 ,0,0,1};


        auto fToMinimise = [&](const dlib::matrix<double, 8, 1>& t) {
            
            float R = t(1);
            float r = t(0);
            float3 c = float3(t(2), t(3), t(4));
            float3 d = float3(0,0,1);
            

            double distsum = 0;

            for (int i = 0; i < torus_points.size(); i++) {

                float dist = getDistFromTorus(torus_points[i].xyz, r, R, c, d) - torus_points[i].w;

                distsum += (dist * dist);
            }
            return distsum;
        };


        double min = dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
            dlib::objective_delta_stop_strategy(0.0000000000000001),
            fToMinimise, start, 0.001);
        minimised_params = start;

        for (int j = 0; j < 10; j++) {
            start = { rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,
                rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX ,
                rand() / (float)RAND_MAX,rand() / (float)RAND_MAX,rand() / (float)RAND_MAX };
            double min2 = dlib::find_min_using_approximate_derivatives(dlib::bfgs_search_strategy(),
                dlib::objective_delta_stop_strategy(0.0000000000000001),
                fToMinimise, start, 0.0000001);
            if (min2 < min) {
                min = min2;
                minimised_params = start;
               
            }
            
        }
     
    }

};


