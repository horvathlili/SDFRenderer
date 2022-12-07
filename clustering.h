#include "Falcor.h"
#include <vector>
#include <dlib/matrix.h>
#include <dlib/clustering.h>
#include "torusfitting.h"


using namespace Falcor;

class Clustering {

public:

    std::vector<float4> points_pos;
    std::vector<float4> points_data;
    std::vector<int> cluster;
    std::vector<float4> c1;
    std::vector<float4> c2;
    int right_cluster;
    int wrong_cluster;

    std::vector<float4> getPoints(int mode,float3 p, float box, int res) {

        std::vector<float4> points_p;
        std::vector<float4> points_d;
        cluster.clear();


        int res2 = res - 1;

        for (int i = 0; i < res; i++) {
            for (int j = 0; j < res; j++) {
                for (int l = 0; l < res; l++) {
                    float4 px = float4(p - float3(box / 2.0), 0);
                    px.x += box / (float)res2 * (float)i;
                    px.y += box / (float)res2 * (float)j;
                    px.z += box / (float)res2 * (float)l;
                        float4 pxx;
                        if (mode == 1) {
                            pxx = twoCircleDistAndNormals(px.xyz, float3(-0.3), 0.4, float3(0.3), 0.2);
                        }
                        if (mode == 2) {
                            pxx =  twoCircleDistAndNormals(px.xyz, float3(0), 0.4, float3(0, 0.1, 0.5), 0.2);
                        }
                        if (mode == 3) {
                            pxx = twoCircleDistAndNormals(px.xyz, float3(0), 0.7, float3(0.7), 0.1);
                        }
                        
                        px.w = pxx.x;
                        points_p.push_back(px);
                        points_d.push_back(pxx);
                        std::cout << pxx.x << " " << pxx.y << " " << pxx.z << "    " << pxx.w << std::endl;
                    }
                }
            }

        points_pos = points_p;
        points_data = points_d;
        return points_d;
    }

    float4 twoCircleDistAndNormals(float3 p,float3 c1,float r1, float3 c2, float r2) {


        float d1 = length(p-c1) - r1;
        float d2 = length(p - c2) - r2;

        float3 n1 = float3(0);
        if (p != c1) {
           n1 = normalize(p - c1);
        }
        float3 n2 = float3(0);
        if (p != c2) {
            n2 = normalize(p - c2);
        }
        

        if (abs(d1) < abs(d2)) {
            cluster.push_back(0);
            return float4(d1, n1);
        }
        else {
            cluster.push_back(1);
            return float4(d2, n2);
        }


    }


    void clusterPoints() {

        typedef dlib::matrix<double, 3, 1> sample_type;
        typedef dlib::radial_basis_kernel<sample_type> kernel_type;

        dlib::kcentroid<kernel_type> kc(kernel_type(0.1), 0.05, 80);
        dlib::kkmeans<kernel_type> test(kc);

        test.set_number_of_centers(2);

        std::vector<sample_type> initial_centers;
        std::vector<sample_type> samples;

        for (int i = 0; i < points_data.size(); i++) {
            sample_type s;
            s(0) = points_pos[i].x - points_data[i].x*points_data[i].y;
            s(1) = points_pos[i].y - points_data[i].x * points_data[i].z;
            s(2) = points_pos[i].z - points_data[i].x * points_data[i].w;
            samples.push_back(s);
           // std::cout << s << std::endl;
        }

        pick_initial_centers(2, initial_centers, samples, test.get_kernel());

        //std::cout << initial_centers[0]<< initial_centers[1]<< std::endl;

        test.train(samples, initial_centers);

        c1.clear();
        c2.clear();

        right_cluster = 0;
        wrong_cluster = 0;

        for (int i = 0; i < points_data.size(); i++) {
            std::cout << cluster[i] << " "<<test(samples[i]) << std::endl;
            if (cluster[i] == test(samples[i])) {
                right_cluster++;
            }
            else {
                wrong_cluster++;
            }
            if (test(samples[i]) == 0) {
                c1.push_back(points_pos[i]);
            }
            else {
                c2.push_back(points_pos[i]);
            }
        }

        


    }
};
