#pragma once

namespace mathpp {
    template <typename T>
    struct vec2 {
        T x,y;
        vec2(T x, T y) : x(x), y(y) {}
        vec2() : x(0), y(0) {}

        vec2& operator+=(const vec2& rhs) {
            x += rhs.x;
            y += rhs.y;
            return *this;
        }
    };

    template <typename T>
    vec2<T> operator+(const vec2<T> &lhs, const vec2<T> &rhs){return vec2<T>(lhs.x + rhs.x, lhs.y + rhs.y);}
    template <typename T>
    vec2<T> operator-(const vec2<T> & lhs, const vec2<T>&rhs){return vec2<T>(lhs.x - rhs.x, lhs.y - rhs.y);}
    template <typename T>
    vec2<T> operator*(const vec2<T>& lhs, const T &s){return vec2<T>(lhs.x * s, lhs.y * s);}
    template<typename T>
    vec2<T> operator*(const T &s, const vec2<T>&rhs){return vec2<T>(s * rhs.x, s * rhs.y);}

    using vec2f = vec2<float>;
    using vec2d = vec2<double>;
    using vec2i = vec2<int>;

}