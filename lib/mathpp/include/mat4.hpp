#pragma once
#include "vec4.hpp"
#include "mat3.hpp"
namespace mathpp {
    template <typename T>
    struct mat4 {
         vec4<T> col[4];

        mat4(): col{
            vec4<T>(T(1),T(0),T(0),T(0)),
            vec4<T>(T(0),T(1),T(0),T(0)),
            vec4<T>(T(0),T(0),T(1),T(0)),
            vec4<T>(T(0),T(0),T(0),T(1))
        } {}

    };
    template <typename T>
    mat4<T> operator+(const mat4<T>& a, const mat4<T>& b) {
        mat4<T> result;
        result.col[0] = a.col[0] + b.col[0];
        result.col[1] = a.col[1] + b.col[1];
        result.col[2] = a.col[2] + b.col[2];
        result.col[3] = a.col[3] + b.col[3];
        return result;
    }
    template <typename T>
    mat4<T> operator-(const mat4<T>& a, const mat4<T>& b) {
        mat4<T> result;
        result.col[0] = a.col[0] - b.col[0];
        result.col[1] = a.col[1] - b.col[1];
        result.col[2] = a.col[2] - b.col[2];
        result.col[3] = a.col[3] - b.col[3];
        return result;
    }
    template <typename T>
    mat4<T> operator*(const mat4<T>& a, const mat4<T>& b) {
        mat4<T> result;

        for (int col = 0; col < 4; col++) {
            for (int row = 0; row < 4; row++) {
                T sum = T(0);
                for (int k = 0; k < 4; k++) {
                    sum += a.col[k][row] * b.col[col][k];
                }
                result.col[col][row] = sum;
            }
        }
        return result;
    }
    template <typename T>
    mat4<T> transpose(const mat4<T>& m) {
        mat4<T> result;
        for (int c = 0; c < 4; c++) {
            for (int r = 0; r < 4; r++) {
                result.col[r][c] = m.col[c][r];   // note r and c swapped on the left
            }
        }
        return result;
    }

    template <typename T>
mat4<T> inverse(const mat4<T>& m) {
        mat3<T> R(
            vec3<T>(m.col[0][0], m.col[0][1], m.col[0][2]),
            vec3<T>(m.col[1][0], m.col[1][1], m.col[1][2]),
            vec3<T>(m.col[2][0], m.col[2][1], m.col[2][2])
        );

        vec3<T> t(m.col[3][0], m.col[3][1], m.col[3][2]);

        mat3<T> R_inv = inverse(R);           // reuses your tested mat3::inverse
        vec3<T> t_inv = R_inv * (t * T(-1));   // -R_inv * t

        mat4<T> result;
        for (int c = 0; c < 3; c++) {
            result.col[c] = vec4<T>(R_inv.col[c].x, R_inv.col[c].y, R_inv.col[c].z, T(0));
        }
        result.col[3] = vec4<T>(t_inv.x, t_inv.y, t_inv.z, T(1));

        return result;
    }
    template <typename T>
T determinant(const mat4<T>& m) {
        mat3<T> upper(
            vec3<T>(m.col[0][0], m.col[0][1], m.col[0][2]),
            vec3<T>(m.col[1][0], m.col[1][1], m.col[1][2]),
            vec3<T>(m.col[2][0], m.col[2][1], m.col[2][2])
        );
        return determinant(upper);
    }
    template<typename T>
    mat4<T> MakeMat4FromColumnMajorElements(const float m[16]) {
        mat4<T> result;
        result.col[0] = vec4<T>(m[0],  m[1],  m[2],  m[3]);
        result.col[1] = vec4<T>(m[4],  m[5],  m[6],  m[7]);
        result.col[2] = vec4<T>(m[8],  m[9],  m[10], m[11]);
        result.col[3] = vec4<T>(m[12], m[13], m[14], m[15]);
        return result;
    }
    using mat4f = mat4<float>;
    using mat4d = mat4<double>;
}



