#pragma once
#include "constants.hpp"
#include "base.hpp"
#include "mat3.hpp"
#include "mat4.hpp"
#include "vec3.hpp"
#include "vec4.hpp"
#include "vec_ops.hpp"
#include <cmath>


namespace mathpp {
    template <typename T>
    mat4<T> rotate(const mat4<T>& a, const vec3<T>& axis, T angle) {
        vec3<T> bnorm = normalize(axis);
        T rad = to_radians(angle);
        T c = std::cos(rad);
        T s = std::sin(rad);
        T c1 = T(1) - c;

        mat4<T> r; // identity
        r.col[0] = vec4<T>(c + bnorm.x*bnorm.x*c1,
                            bnorm.y*bnorm.x*c1 + bnorm.z*s,
                            bnorm.z*bnorm.x*c1 - bnorm.y*s,
                            T(0));
        r.col[1] = vec4<T>(bnorm.x*bnorm.y*c1 - bnorm.z*s,
                            c + bnorm.y*bnorm.y*c1,
                            bnorm.z*bnorm.y*c1 + bnorm.x*s,
                            T(0));
        r.col[2] = vec4<T>(bnorm.x*bnorm.z*c1 + bnorm.y*s,
                            bnorm.y*bnorm.z*c1 - bnorm.x*s,
                            c + bnorm.z*bnorm.z*c1,
                            T(0));

        return a * r;
    }

    template<typename T>
    mat4<T> scale(const mat4<T>& a, const vec3<T>& s) {
        mat4<T> result = a;
        result.col[0] = result.col[0] * s.x;
        result.col[1] = result.col[1] * s.y;
        result.col[2] = result.col[2] * s.z;
        return result;
    }

    template<typename T>
    mat4<T> translate(const mat4<T>& a, const vec3<T>& b) {
        mat4<T> t; // identity
        t.col[3] = vec4<T>(b.x, b.y, b.z, T(1));
        return a * t;
    }

    template <typename T>
    mat4<T> look_at(const vec3<T>& eye, const vec3<T>& target, const vec3<T>& up) {
        vec3<T> f = normalize(target - eye);      // forward

        vec3<T> upRef = up;
        if (std::abs(dot(f, up)) > T(0.999)) {
            upRef = vec3<T>(T(0), T(0), T(1)); // f is nearly parallel to the given up — fall back to avoid a degenerate cross product
        }

        vec3<T> r = normalize(cross(f, upRef));   // right
        vec3<T> u = cross(r, f);                  // recomputed up (already unit length, no need to normalize)

        mat4<T> result;
        result.col[0] = vec4<T>(r.x, u.x, -f.x, T(0));
        result.col[1] = vec4<T>(r.y, u.y, -f.y, T(0));
        result.col[2] = vec4<T>(r.z, u.z, -f.z, T(0));
        result.col[3] = vec4<T>(-dot(r, eye), -dot(u, eye), dot(f, eye), T(1));

        return result;
    }

    template <typename T>
    mat4<T> perspective(T fov_degrees, T aspect, T near_plane, T far_plane) {
        T fov_rad = to_radians(fov_degrees);
        T tan_half_fov = std::tan(fov_rad / T(2));

        mat4<T> result; // start from identity, then overwrite
        result.col[0] = vec4<T>(T(1) / (aspect * tan_half_fov), T(0), T(0), T(0));
        result.col[1] = vec4<T>(T(0), T(1) / tan_half_fov, T(0), T(0));
        result.col[2] = vec4<T>(T(0), T(0), -(far_plane + near_plane) / (far_plane - near_plane), T(-1));
        result.col[3] = vec4<T>(T(0), T(0), -(T(2) * far_plane * near_plane) / (far_plane - near_plane), T(0));

        return result;
    }

}