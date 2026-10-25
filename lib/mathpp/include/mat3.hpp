#pragma once
#include <assert.h>

#include "vec3.hpp"
#include "vec_ops.hpp"
#include "constants.hpp"
#include <cmath>



namespace mathpp {
    template <typename T>
    struct mat3 {
         vec3<T> col[3];

        mat3()
            : col{ vec3<T>(T(1), T(0), T(0)),
                   vec3<T>(T(0), T(1), T(0)),
                   vec3<T>(T(0), T(0), T(1)) } {}

        mat3(const vec3<T>& c0, const vec3<T>& c1, const vec3<T>& c2)
            : col{ c0, c1, c2 } {}

        vec3<T>& operator[](int i) {
            switch (i) {
                case 0: return col[0];
                case 1: return col[1];
                default: return col[2];
            }
        }

        const  vec3<T>& operator[](int i) const {
            switch (i) {
                case 0: return col[0];
                case 1: return col[1];
                default: return col[2];
            }
        }
    };
    template <typename T>
    mat3<T> operator*(const mat3<T>& a, const mat3<T>& b) {
        mat3<T> result;
        for (int col = 0; col < 3; col++) {
            for (int row = 0; row < 3; row++) {
                T sum = T(0);
                for (int k = 0; k < 3; k++) {
                    sum += a.col[k][row] * b.col[col][k];
                }
                result.col[col][row] = sum;
            }
        }
        return result;
    }
    template <typename T>
    mat3<T> operator-(const mat3<T>& a, const mat3<T>& b) {return  mat3<T>(a.col[0]-b.col[0], a.col[1]-b.col[1], a.col[2]-b.col[2]); }
    template <typename T>
    mat3<T> operator+(const mat3<T>& a, const mat3<T>& b) {
        return mat3<T>(a.col[0]+b.col[0], a.col[1]+b.col[1], a.col[2]+b.col[2]);
    }

    template <typename T>
    mat3<T> transpose(const mat3<T>& m) {
        mat3<T> result;
        for (int c = 0; c < 3; c++) {
            for (int r = 0; r < 3; r++) {
                result.col[r][c] = m.col[c][r];   // note r and c swapped on the left
            }
        }
        return result;
    }
    template <typename T>
   mat3<T> inverse(const mat3<T>& m) {
        vec3<T> r0 = cross(m.col[1], m.col[2]);
        vec3<T> r1 = cross(m.col[2], m.col[0]);
        vec3<T> r2 = cross(m.col[0], m.col[1]);

        T det = dot(m.col[0], r0);
        if (std::abs(det) < epsilon<T>) {
            assert(false && "mat3::inverse: matrix is singular, cannot invert");
            return mat3<T>();
        }

        T inv_det = T(1) / det;

        return mat3<T>(
            vec3<T>(r0.x, r1.x, r2.x) * inv_det,
            vec3<T>(r0.y, r1.y, r2.y) * inv_det,
            vec3<T>(r0.z, r1.z, r2.z) * inv_det
        );
    }
    template <typename T>
     T determinant(const mat3<T>& m) {return dot(m.col[0],cross(m.col[1],m.col[2]));}

    using mat3f = mat3<float>;
    using mat3d = mat3<double>;

}
