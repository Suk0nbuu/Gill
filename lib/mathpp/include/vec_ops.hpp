#pragma once
#include <cmath>
#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"
#include "constants.hpp"

namespace mathpp {

    template <typename T>
    T dot(const vec3<T>& a, const vec3<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }

    template <typename T>
    T dot(const vec4<T>& a, const vec4<T>& b) { return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w; }

    template <typename T>
    vec3<T> cross(const vec3<T>& a, const vec3<T>& b) {
        return vec3<T>(a.y * b.z - a.z * b.y,
                       a.z * b.x - a.x * b.z,
                       a.x * b.y - a.y * b.x);
    }

    template <typename T>
    vec4<T> to_point(const vec3<T>& a) { return vec4<T>(a.x, a.y, a.z, T(1)); }

    template <typename T>
    vec4<T> to_direction(const vec3<T>& a) { return vec4<T>(a.x, a.y, a.z, T(0)); }

    template <typename T>
    vec2<T> normalize(const vec2<T>& a) {
        T len = std::sqrt(a.x * a.x + a.y * a.y);
        if (len < epsilon<T>) { return vec2<T>(T(0), T(0)); }
        T inv_len = T(1) / len;
        return vec2<T>(a.x * inv_len, a.y * inv_len);
    }

    template <typename T>
    vec3<T> normalize(const vec3<T>& a) {
        T len = std::sqrt(dot(a, a));
        if (len < epsilon<T>) { return vec3<T>(T(0), T(0), T(0)); }
        T inv_len = T(1) / len;
        return a * inv_len;
    }

    template <typename T>
    vec4<T> normalize(const vec4<T>& a) {
        T len = std::sqrt(dot(a, a));
        if (len < epsilon<T>) { return vec4<T>(T(0), T(0), T(0), T(0)); }
        T inv_len = T(1) / len;
        return a * inv_len;
    }

    template <typename T>
    vec3<T> operator/(const vec4<T>& a, T b) {
        return vec3<T>(a.x / b, a.y / b, a.z / b);
    }

    template <typename T>
    T length3v(const vec3<T>& v) { return std::sqrt(dot(v, v)); }

    template <typename T>
    T length4v(const vec4<T>& v) { return std::sqrt(dot(v, v)); }

}