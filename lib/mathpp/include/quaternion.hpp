#pragma once
#include "mat_ops.hpp"

namespace mathpp {
    template <typename T>
    struct quat {
        T w,x,y,z;

        quat() : w(1),x(0),y(0),z(0) {}
        quat(T w, T x, T y,T z): w(w),x(x),y(y),z(z) {}
    };

    template<typename T>
    quat<T> QuatFromAxisAngle(T angle,const  vec3<T>& axis) {
        quat<T> q;
        vec3<T> normAxis = normalize(axis);
        T rad = to_radians(angle);
        q.w = cos(rad/static_cast<T>(2));
        T halfSin = sin(rad / static_cast<T>(2));
        q.x = normAxis.x * halfSin;
        q.y = normAxis.y * halfSin;
        q.z = normAxis.z * halfSin;
        return q;
    }
    template<typename T>
    quat<T> QuatFromEulerAngles(const vec3<T>& euler) {
        quat<T> q;
        T cy = cos(to_radians(euler.y)/static_cast<T>(2));
        T sy = sin(to_radians(euler.y)/static_cast<T>(2));
        T cx = cos(to_radians(euler.x)/static_cast<T>(2));
        T sx = sin(to_radians(euler.x)/static_cast<T>(2));
        T cz = cos(to_radians(euler.z)/static_cast<T>(2));
        T sz = sin(to_radians(euler.z)/static_cast<T>(2));

        q.w = cy * cx * cz + sy * sx * sz;
        q.x = cy * sx * cz + sy * cx * sz;
        q.y = sy * cx * cz - cy * sx * sz;
        q.z = cy * cx * sz - sy * sx * cz;
        return q;
    }
    template<typename T>
    vec3<T> QuatToEulerAngles(const quat<T>& q) {
        vec3<T> r;
        r.x = to_degrees(atan2(T(2)*(q.w*q.x + q.y*q.z), T(1) - T(2)*(q.x*q.x + q.y*q.y)));
        r.z = to_degrees(atan2(T(2)*(q.w*q.z + q.x*q.y), T(1) - T(2)*(q.y*q.y + q.z*q.z)));
        T sinPitch = T(2)*(q.w*q.y - q.x*q.z);
        sinPitch = clamp(sinPitch, T(-1), T(1));
        r.y = to_degrees(asin(sinPitch));
        return r;
    }



    template<typename T>
vec3<T> RotateVector(const quat<T>& q, const vec3<T>& v) {
        vec3<T> qv{q.x, q.y, q.z};
        vec3<T> t = cross(qv, v) * static_cast<T>(2);
        return v + t * q.w + cross(qv, t);
    }

    template<typename T>
    quat<T> normalize(const quat<T>& q) {
        T divisor = std::sqrt(q.w*q.w + q.x*q.x + q.y*q.y + q.z*q.z );
        if (divisor < epsilon<T>) {
           return quat<T>();
        }
        return quat<T>( q.w / divisor , q.x / divisor , q.y/ divisor, q.z / divisor);
    }

    template<typename T>
    quat<T> operator*(const quat<T>& lhs, const quat<T>& rhs) {
        quat<T> q;
        q.w = lhs.w * rhs.w - lhs.x * rhs.x - lhs.y * rhs.y - lhs.z * rhs.z;
        q.x = lhs.w * rhs.x + lhs.x * rhs.w + lhs.y * rhs.z - lhs.z * rhs.y;
        q.y = lhs.w * rhs.y - lhs.x * rhs.z + lhs.y * rhs.w + lhs.z * rhs.x;
        q.z = lhs.w * rhs.z + lhs.x * rhs.y - lhs.y * rhs.x + lhs.z * rhs.w;
        return q;
    }
        template<typename T>
        quat<T> operator*(const quat<T>& lhs, T s) {
            return quat<T>(lhs.w* s,lhs.x * s,lhs.y * s, lhs.z* s);
        }
        template<typename T>
        quat<T> operator*(T s, const quat<T>& rhs) {
            return quat<T>(rhs.w* s,rhs.x * s,rhs.y * s, rhs.z* s);
        }
    template<typename T>
    quat<T> operator/(const quat<T>& lhs, T s) {
        return quat<T>(lhs.w/s,lhs.x/s,lhs.y/s,lhs.z/s);
    }
    template<typename T>
    quat<T> operator/(T s, const quat<T>& rhs) {
        return quat<T>(rhs.w/s,rhs.x/s,rhs.y/s,rhs.z/s);
    }
    template<typename T>
    quat<T> operator+(const quat<T>& lhs, const quat<T>& rhs) {
        return quat<T>(lhs.w + rhs.w,lhs.x + rhs.x, lhs.y + rhs.y,lhs.z + rhs.z );
    }
    template<typename T>
    quat<T> operator-(const quat<T>& lhs, const quat<T>& rhs) {
        return quat<T>(lhs.w - rhs.w,lhs.x - rhs.x, lhs.y - rhs.y,lhs.z - rhs.z );
    }
    template<typename T>
    mat4<T> QuatToMat4(const quat<T>& q) {
        float xx = q.x*q.x, yy = q.y*q.y, zz = q.z*q.z;
        float xy = q.x*q.y, xz = q.x*q.z, yz = q.y*q.z;
        float wx = q.w*q.x, wy = q.w*q.y, wz = q.w*q.z;

        mat4<T> m;
        m.col[0] = vec4<T>(1 - 2*(yy+zz), 2*(xy+wz),     2*(xz-wy),     T(0.0));
        m.col[1] = vec4<T>(2*(xy-wz),     1 - 2*(xx+zz), 2*(yz+wx),     T(0.0));
        m.col[2] = vec4<T>(2*(xz+wy),     2*(yz-wx),     1 - 2*(xx+yy), T(0.0));
        m.col[3] = vec4<T>(0.0f,          0.0f,          0.0f,         T(1.0));
        return m;
    }
    template <typename T>
    quat<T> QuatFromMat4(const mat4<T>& m) {
        //Shepperd Method
        quat<T> q;


        mat3<T> mt3 = Mat3CleanScale(UpperMatrix(m));

        T trace = mt3.col[0][0] + mt3.col[1][1] + mt3.col[2][2];
        if (trace > T(0.0)) {
            // Case 1: qw is the largest
            T s = std::sqrt(trace + T(1.0)) * T(2.0); // s = 4 * qw
            q.w = 0.25f * s;
            q.x = (mt3.col[1][2] - mt3.col[2][1]) / s;
            q.y = (mt3.col[2][0] - mt3.col[0][2]) / s;
            q.z = (mt3.col[0][1] - mt3.col[1][0]) / s;
        }
        else if ((mt3.col[0][0] > mt3.col[1][1]) && (mt3.col[0][0] > mt3.col[2][2])) {
            // Case 2: qx is the largest
            T s = std::sqrt(1.0f + mt3.col[0][0] - mt3.col[1][1] - mt3.col[2][2]) * T(2.0); // s = 4 * qx
            q.w = (mt3.col[1][2] - mt3.col[2][1]) / s;
            q.x = T(0.25) * s;
            q.y = (mt3.col[1][0] + mt3.col[0][1]) / s;
            q.z = (mt3.col[0][2] + mt3.col[2][0]) / s;
        }
        else if (mt3.col[1][1] > mt3.col[2][2]) {
            // Case 3: qy is the largest
            T s = std::sqrt(T(1.0) - mt3.col[0][0] + mt3.col[1][1] - mt3.col[2][2]) * T(2.0); // s = 4 * qy
            q.w = (mt3.col[2][0] - mt3.col[0][2]) / s;
            q.x = (mt3.col[1][0] + mt3.col[0][1]) / s;
            q.y = T(0.25) * s;
            q.z = (mt3.col[2][1] + mt3.col[1][2]) / s;
        }
        else {
            // Case 4: qz is the largest
            T s = std::sqrt(T(1.0) - mt3.col[0][0] - mt3.col[1][1] + mt3.col[2][2]) * T(2.0); // s = 4 * qz
            q.w = (mt3.col[0][1] - mt3.col[1][0]) / s;
            q.x = (mt3.col[0][2] + mt3.col[2][0]) / s;
            q.y = (mt3.col[2][1] + mt3.col[1][2]) / s;
            q.z = T(0.25) * s;
        }
        q = normalize(q);
        return q;
    }

    template <typename T>
    quat<T> ConjugateQuat(const quat<T>& q) {
        return quat<T>(q.w, -q.x, -q.y, -q.z);
    }

    template<typename T>
    T dot (const quat<T>& a, const quat<T>& b) {
        return a.w * b.w + a.x * b.x + a.y * b.y + a.z * b.z;
    }


    using quatf = quat<float>;
    using quatd = quat<double>;

}