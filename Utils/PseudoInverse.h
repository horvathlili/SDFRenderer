#pragma once

#include <vector>
#include "../Eigen/Dense"
#include "Falcor.h"

using namespace Falcor;

std::vector<float> getPseudoInverse1(int boundingbox, int res) {

    Eigen::MatrixXf m(27, 4);

    std::vector<float> ret;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {

                float3 x = float3(i, j, k) * (float)boundingbox / (float)res * 0.6f;

                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 0) = x.x;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 1) = x.y;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 2) = x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 3) = 1.0;

            }
        }
    }



    Eigen::MatrixXf result = (m.transpose() * m).inverse() * m.transpose();

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 27; j++) {
            ret.push_back(result(i, j));
        }
    }

    return ret;
}

std::vector<float> getPseudoInverse2(int boundingbox, int res) {

    Eigen::MatrixXf m(27, 10);

    std::vector<float> ret;

    for (int i = -1; i <= 1; i++) {
        for (int j = -1; j <= 1; j++) {
            for (int k = -1; k <= 1; k++) {

                float3 x = float3(i, j, k) * (float)boundingbox / (float)res * 0.6f;

                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 0) = x.x;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 1) = x.y;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 2) = x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 3) = x.x * x.x;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 4) = x.y * x.y;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 5) = x.z * x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 6) = x.x*x.y;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 7) = x.x*x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 8) = x.y*x.z;
                m((i + 1) * 9 + (j + 1) * 3 + (k + 1), 9) = 1.0;

            }
        }
    }



    Eigen::MatrixXf result = (m.transpose() * m).inverse() * m.transpose();

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 27; j++) {
            ret.push_back(result(i, j));
        }
    }

    return ret;
}

