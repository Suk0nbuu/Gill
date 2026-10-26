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
quat<T> v1c = v1;
        T cosTheta =  dot(v0, v1);
        if (dot >= T(0.9995)) { return Nlerp(t,v0,v1); }
     if(dot < 0)
       { dot = - dot;
v1c = {-v1.w,-v1.x,-v1.y, -v1.z};
}
     

T theta std::acos(clamp(cosTheta,T(-1.0),T(1.0)));
        T upper1= std::sin((T(1.0)- t)* theta);
        T lower = std::sin(theta);
        quat<T> result1 = upper1 * v0 /lower;
        T upper2 = std::sin(t* theta);
        quat<T> result2 = upper2 * v1c /lower;
        return result1 + result2;
    }
}