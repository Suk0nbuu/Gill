#pragma once
#include "vec3.hpp"
#include "vec4.hpp"
#include "mat3.hpp"
#include "mat4.hpp"
#include "matrix_transform.hpp"

namespace mathpp {
    template <typename T>
    vec3<T> operator*(const mat3<T>& m, const vec3<T>& v) {
        return vec3<T>(
            m.col[0][0]*v.x + m.col[1][0]*v.y + m.col[2][0]*v.z,
            m.col[0][1]*v.x + m.col[1][1]*v.y + m.col[2][1]*v.z,
            m.col[0][2]*v.x + m.col[1][2]*v.y + m.col[2][2]*v.z
        );
    }
    template <typename T>
    vec4<T> operator*(const mat4<T>& m, const vec4<T>& v) {
        return vec4<T>(
            m.col[0][0]*v.x + m.col[1][0]*v.y + m.col[2][0]*v.z + m.col[3][0]*v.w,
            m.col[0][1]*v.x + m.col[1][1]*v.y + m.col[2][1]*v.z + m.col[3][1]*v.w,
            m.col[0][2]*v.x + m.col[1][2]*v.y + m.col[2][2]*v.z + m.col[3][2]*v.w,
            m.col[0][3]*v.x + m.col[1][3]*v.y + m.col[2][3]*v.z + m.col[3][3]*v.w
        );
    }
    template<typename T>
   mat3<T> UpperMatrix(const mat4<T>& model) {
        mat3<T> upper(
            vec3<T>(model.col[0][0], model.col[0][1], model.col[0][2]),
            vec3<T>(model.col[1][0], model.col[1][1], model.col[1][2]),
            vec3<T>(model.col[2][0], model.col[2][1], model.col[2][2])
        );
        return upper;
    }

    template <typename T>
    mat3<T> normal_matrix(const mat4<T>& model) {
        mat3<T> upper = UpperMatrix(model);
        return transpose(inverse(upper));
    }


    template<typename T>
    vec3<T> TranslateFromMat4(const mat4<T>& model) {
        vec3<T> result;
        result[0] = model.col[3][0];
        result[1] = model.col[3][1];
        result[2] = model.col[3][2];
        return result;
    }
    template <typename T>
   mat3<T> Mat3CleanScale(const mat3<T>& m) {
        mat3<T> result;
        T scaleX = std::sqrt(m.col[0][0]*m.col[0][0] + m.col[0][1]*m.col[0][1] + m.col[0][2]*m.col[0][2]);
        T scaleY = std::sqrt(m.col[1][0]*m.col[1][0] + m.col[1][1]*m.col[1][1] + m.col[1][2]*m.col[1][2]);
        T scaleZ = std::sqrt(m.col[2][0]*m.col[2][0] + m.col[2][1]*m.col[2][1] + m.col[2][2]*m.col[2][2]);


        result.col[0][0] = m.col[0][0] / scaleX;
        result.col[1][0] = m.col[1][0] / scaleY;
        result.col[2][0] = m.col[2][0] / scaleZ;


        result.col[0][1] = m.col[0][1] / scaleX;
        result.col[1][1] = m.col[1][1] / scaleY;
        result.col[2][1] = m.col[2][1] / scaleZ;


        result.col[0][2] = m.col[0][2] / scaleX;
        result.col[1][2] = m.col[1][2] / scaleY;
        result.col[2][2] = m.col[2][2] / scaleZ;

        return result;
    }
    template <typename T>
    vec3<T> ScaleFromMat4(const mat4<T>& model) {
        vec3<T> result;
        result[0] = length3v(vec3<T>{model.col[0][0], model.col[0][1], model.col[0][2]});
        result[1] = length3v(vec3<T>{model.col[1][0], model.col[1][1], model.col[1][2]});
        result[2] = length3v(vec3<T>{model.col[2][0], model.col[2][1], model.col[2][2]});
        return result;
    }





}
