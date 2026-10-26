#pragma once
#include "vec3.hpp"
#include "quaternion.hpp"

namespace mathpp {

    template <typename T>
    vec3<T> Lerp( T t, const vec3<T>& v0,  const vec3<T>& v1 ) {
        return  v0 + (v1 - v0)*t;
    }

    template <typename T>
    quat<T> Lerp( T t, const quat<T>& v0, const quat<T>&  v1 ) {
        return  v0 + (v1 - v0)*t;
    }

    template <typename T>
    quat<T> Nlerp(T t,  const quat<T>& v0, const quat<T>& v1) {
        quat<T> r = Lerp(t,v0,v1);
        return  r / std::sqrt(r.w * r.w + r.x * r.x + r.y * r.y + r.z * r.z);
    }
    template <typename T>
    quat<T> Slerp(T t,  const quat<T>& v0, const quat<T>& v1 ) {
        T dot =  dot(v0, v1);
        if (dot >= T(1.0)) { return Nlerp(t,v0,v1); }
        T theta = std::acos(clamp(dot),T(0.0),T(1.0));
        T upper1= std::sin(to_radians((T(1.0)- t))* theta);
        T lower = std::sin(theta);
        quat<T> result1 = upper1 * v0 /lower;
        T upper2 = std::sin(to_radians(t)* theta);
        quat<T> result2 = upper2 * v1 /lower;
        return result1 + result2;
    }
}