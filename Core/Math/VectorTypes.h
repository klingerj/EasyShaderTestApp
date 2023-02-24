#pragma once

#include "CoreDefines.h"

#include <float.h>
#include <string.h>

namespace Tk
{
namespace Core
{

// Vector classes

template <typename T>
struct vec2
{
    enum
    {
        numEles = 2,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            T x;
            T y;
        };
    };

    vec2()
    {
        this->x = {};
        this->y = {};
    }

    vec2(const vec2& v)
    {
        this->x = v.x;
        this->y = v.y;
    }

    vec2(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    vec2(const T& x)
    {
        this->x = x;
        this->y = x;
    }

    vec2(const T& x, const T& y)
    {
        this->x = x;
        this->y = y;
    }

    const T& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    T& operator[](size_t index)
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    vec2<T> operator+(const vec2<T>& other) const
    {
        return { x + other.x, y + other.y };
    }

    vec2<T> operator+(const T& other) const
    {
        return *this + vec2<T>(other);
    }

    vec2<T> operator-(const vec2<T>& other) const
    {
        return { x - other.x, y - other.y };
    }

    vec2<T> operator-(const T& other) const
    {
        return *this - vec2<T>(other);
    }

    vec2<T> operator-() const
    {
        return { -this->x, -this->y };
    }

    vec2<T> operator*(const vec2<T>& other) const
    {
        return { x * other.x, y * other.y };
    }

    vec2<T> operator*(const T& other) const
    {
        return *this * vec2<T>(other);
    }

    vec2<T> operator/(const vec2<T>& other) const
    {
        return { x / other.x, y / other.y };
    }

    vec2<T> operator/(const T& other) const
    {
        return *this / vec2<T>(other);
    }

    void operator+=(const vec2<T>& other)
    {
        x += other.x;
        y += other.y;
    }

    void operator-=(const vec2<T>& other)
    {
        x -= other.x;
        y -= other.y;
    }

    void operator*=(const vec2<T>& other)
    {
        x *= other.x;
        y *= other.y;
    }

    void operator/=(const vec2<T>& other)
    {
        x /= other.x;
        y /= other.y;
    }
};

template <typename T>
bool operator==(const vec2<T>& lhs, const vec2<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

template <typename T>
bool operator!=(const vec2<T>& lhs, const vec2<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
struct vec3
{
    enum
    {
        numEles = 3,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            T x;
            T y;
            T z;
        };
    };

    vec3()
    {
        this->x = {};
        this->y = {};
        this->z = {};
    }

    vec3(const vec3& v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
    }

    vec3(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    vec3(const T& x)
    {
        this->x = x;
        this->y = x;
        this->z = x;
    }

    vec3(const T& x, const T& y, const T& z)
    {
        this->x = x;
        this->y = y;
        this->z = z;
    }

    const T& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    T& operator[](size_t index)
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    vec3<T> operator+(const vec3<T>& other) const
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    vec3<T> operator+(const T& other) const
    {
        return *this + vec3<T>(other);
    }

    vec3<T> operator-(const vec3<T>& other) const
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    vec3<T> operator-(const T& other) const
    {
        return *this - vec3<T>(other);
    }

    vec3<T> operator-() const
    {
        return { -this->x, -this->y, -this->z };
    }

    vec3<T> operator*(const vec3<T>& other) const
    {
        return { x * other.x, y * other.y, z * other.z };
    }

    vec3<T> operator*(const T& other) const
    {
        return *this * vec3<T>(other);
    }

    vec3<T> operator/(const vec3<T>& other) const
    {
        return { x / other.x, y / other.y, z / other.z };
    }

    vec3<T> operator/(const T& other) const
    {
        return *this / vec3<T>(other);
    }

    void operator+=(const vec3<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
    }

    void operator-=(const vec3<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
    }

    void operator*=(const vec3<T>& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
    }

    void operator/=(const vec3<T>& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
    }
};

template <typename T>
bool operator==(const vec3<T>& lhs, const vec3<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z;
}

template <typename T>
bool operator!=(const vec3<T>& lhs, const vec3<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
struct vec4
{
    enum
    {
        numEles = 4,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            T x;
            T y;
            T z;
            T w;
        };
    };

    vec4()
    {
        this->x = {};
        this->y = {};
        this->z = {};
        this->w = {};
    }

    vec4(const vec4& v)
    {
        this->x = v.x;
        this->y = v.y;
        this->z = v.z;
        this->w = v.w;
    }

    vec4(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    vec4(const T& x)
    {
        this->x = x;
        this->y = x;
        this->z = x;
        this->w = x;
    }

    vec4(const T& x, const T& y, const T& z, const T& w)
    {
        this->x = x;
        this->y = y;
        this->z = z;
        this->w = w;
    }

    vec4(const vec3<T>& xyz, const T& w)
    {
        this->x = xyz.x;
        this->y = xyz.y;
        this->z = xyz.z;
        this->w = w;
    }

    const T& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    T& operator[](size_t index)
    {
        TINKER_ASSERT(index < numEles);
        return m_data[index];
    }

    vec4<T> operator+(const vec4<T>& other) const
    {
        return { x + other.x, y + other.y, z + other.z, w + other.w };
    }

    vec4<T> operator+(const T& other) const
    {
        return *this + vec4<T>(other);
    }

    vec4<T> operator-(const vec4<T>& other) const
    {
        return { x - other.x, y - other.y, z - other.z, w - other.w };
    }

    vec4<T> operator-(const T& other) const
    {
        return *this - vec4<T>(other);
    }

    vec4<T> operator-() const
    {
        return { -this->x, -this->y, -this->z, -this->w };
    }

    vec4<T> operator*(const vec4<T>& other) const
    {
        return { x * other.x, y * other.y, z * other.z, w * other.w };
    }

    vec4<T> operator*(const T& other) const
    {
        return *this * vec4<T>(other);
    }

    vec4<T> operator/(const vec4<T>& other) const
    {
        return { x / other.x, y / other.y, z / other.z, w / other.w };
    }

    vec4<T> operator/(const T& other) const
    {
        return *this / vec4<T>(other);
    }

    void operator+=(const vec4<T>& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        w += other.w;
    }

    void operator-=(const vec4<T>& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        w -= other.w;
    }

    void operator*=(const vec4<T>& other)
    {
        x *= other.x;
        y *= other.y;
        z *= other.z;
        w *= other.w;
    }

    void operator/=(const vec4<T>& other)
    {
        x /= other.x;
        y /= other.y;
        z /= other.z;
        w /= other.w;
    }

    vec3<T> xyz() const
    {
        return { x, y, z };
    }
};

template <typename T>
bool operator==(const vec4<T>& lhs, const vec4<T>& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y && lhs.z == rhs.z && lhs.w == rhs.w;
}

template <typename T>
bool operator!=(const vec4<T>& lhs, const vec4<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
inline vec3<T> Cross(const vec3<T>& a, const vec3<T>& b)
{
    return vec3<T>(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

template <typename T>
inline T Dot(const vec2<T>& a, const vec2<T>& b)
{
    return a.x * b.x + a.y * b.y;
}

template <typename T>
inline T Dot(const vec3<T>& a, const vec3<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

template <typename T>
inline T Dot(const vec4<T>& a, const vec4<T>& b)
{
    return a.x * b.x + a.y * b.y + a.z * b.z + a.w * b.w;
}

// Matrix classes

template <typename T>
struct mat2
{
    enum
    {
        numEles = 4,
        numCols = 2,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            vec2<T> m_cols[numCols];
        };
    };

    mat2()
    {
        for (uint8 i = 0; i < numEles; ++i) m_data[i] = {};
    }

    mat2(const mat2<T>& other)
    {
        memcpy(m_data, other.m_data, numEleBytes);
    }

    mat2(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    mat2(const T& x)
    {
        // Initialize diagonal
        m_data[0] = x;
        m_data[1] = {};
        m_data[2] = {};
        m_data[3] = x;
    }

    mat2(const vec2<T>& a, const vec2<T>& b)
    {
        // Column-major
        m_data[0] = a.x; m_data[2] = b.x;
        m_data[1] = a.y; m_data[3] = b.y;
    }

    mat2(const T& x00, const T& x01, const T& x10, const T& x11)
    {
        // Column-major
        m_data[0] = x00; m_data[2] = x10;
        m_data[1] = x01; m_data[3] = x11;
    }

    const vec2<T>& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    vec2<T>& operator[](size_t index)
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    mat2<T> operator+(const mat2<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] + other.m_data[i];
        }
        return { newData };
    }

    mat2<T> operator-(const mat2<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] - other.m_data[i];
        }
        return { newData };
    }

    mat2<T> operator*(const mat2<T>& other) const
    {
        mat2<T> newMat;

        for (uint8 dstCol = 0; dstCol < numCols; ++dstCol)
        {
            for (uint8 dstRow = 0; dstRow < numCols; ++dstRow)
            {
                const vec2<T> row = vec2<T>((*this)[0][dstRow], (*this)[1][dstRow]);
                const vec2<T>& col = other[dstCol];
                newMat[dstCol][dstRow] = Dot(row, col);
            }
        }

        return newMat;
    }

    mat2<T> operator/(const mat2<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] / other.m_data[i];
        }
        return { newData };
    }

    void operator+=(const mat2<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] += other.m_data[i];
        }
    }

    void operator-=(const mat2<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] -= other.m_data[i];
        }
    }

    void operator*=(const mat2<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] *= other.m_data[i];
        }
    }

    void operator/=(const mat2<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] /= other.m_data[i];
        }
    }
};

template <typename T>
vec2<T> operator*(const mat2<T>& m, const vec2<T>& v)
{
    return { m[0][0] * v.x + m[1][0] * v.y, m[0][1] * v.x + m[1][1] * v.y };
}

template <typename T>
bool operator==(const mat2<T>& lhs, const mat2<T>& rhs)
{
    bool equal = true;
    for (uint8 i = 0; i < 4; ++i)
    {
        equal &= lhs.m_data[i] == rhs.m_data[i];
    }
    return equal;
}

template <typename T>
bool operator!=(const mat2<T>& lhs, const mat2<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
struct mat3
{
    enum
    {
        numEles = 9,
        numCols = 3,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            vec3<T> m_cols[numCols];
        };
    };

    mat3()
    {
        for (uint8 i = 0; i < numEles; ++i) m_data[i] = {};
    }

    mat3(const mat3<T>& other)
    {
        memcpy(m_data, other.m_data, numEleBytes);
    }

    mat3(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    mat3(const T& x)
    {
        // Initialize diagonal
        m_data[0] = x;
        m_data[1] = {};
        m_data[2] = {};

        m_data[3] = {};
        m_data[4] = x;
        m_data[5] = {};

        m_data[6] = {};
        m_data[7] = {};
        m_data[8] = x;
    }

    mat3(const vec3<T>& a, const vec3<T>& b, const vec3<T>& c)
    {
        // Column-major
        m_data[0] = a.x; m_data[3] = b.x; m_data[6] = c.x;
        m_data[1] = a.y; m_data[4] = b.y; m_data[7] = c.y;
        m_data[2] = a.z; m_data[5] = b.z; m_data[8] = c.z;
    }

    mat3(const T& x00, const T& x01, const T& x02,
        const T& x10, const T& x11, const T& x12,
        const T& x20, const T& x21, const T& x22)
    {
        // Column-major
        m_data[0] = x00; m_data[3] = x10; m_data[6] = x20;
        m_data[1] = x01; m_data[4] = x11; m_data[7] = x21;
        m_data[2] = x02; m_data[5] = x12; m_data[8] = x22;
    }

    const vec3<T>& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    vec3<T>& operator[](size_t index)
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    mat3<T> operator+(const mat3<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] + other.m_data[i];
        }
        return { newData };
    }

    mat3<T> operator-(const mat3<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] - other.m_data[i];
        }
        return { newData };
    }

    mat3<T> operator*(const mat3<T>& other) const
    {
        mat3<T> newMat;

        for (uint8 dstCol = 0; dstCol < numCols; ++dstCol)
        {
            for (uint8 dstRow = 0; dstRow < numCols; ++dstRow)
            {
                const vec3<T> row = vec3<T>((*this)[0][dstRow], (*this)[1][dstRow], (*this)[2][dstRow]);
                const vec3<T>& col = other[dstCol];
                newMat[dstCol][dstRow] = Dot(row, col);
            }
        }

        return newMat;
    }

    mat3<T> operator/(const mat3<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] / other.m_data[i];
        }
        return { newData };
    }

    void operator+=(const mat3<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] += other.m_data[i];
        }
    }

    void operator-=(const mat3<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] -= other.m_data[i];
        }
    }

    void operator*=(const mat3<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] *= other.m_data[i];
        }
    }

    void operator/=(const mat3<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] /= other.m_data[i];
        }
    }
};

template <typename T>
vec3<T> operator*(const mat3<T>& m, const vec3<T>& v)
{
    return {
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z
    };
}

template <typename T>
bool operator==(const mat3<T>& lhs, const mat3<T>& rhs)
{
    bool equal = true;
    for (uint8 i = 0; i < 9; ++i)
    {
        equal &= lhs.m_data[i] == rhs.m_data[i];
    }
    return equal;
}

template <typename T>
bool operator!=(const mat3<T>& lhs, const mat3<T>& rhs)
{
    return !(lhs == rhs);
}

template <typename T>
struct mat4
{

    enum
    {
        numEles = 16,
        numCols = 4,
        numEleBytes = numEles * sizeof(T)
    };

public:
    union
    {
        T m_data[numEles];
        struct
        {
            vec4<T> m_cols[numCols];
        };
    };

    mat4()
    {
        for (uint8 i = 0; i < numEles; ++i) m_data[i] = {};
    }

    mat4(const mat4<T>& other)
    {
        memcpy(m_data, other.m_data, numEleBytes);
    }

    mat4(const T* data)
    {
        memcpy(m_data, data, numEleBytes);
    }

    mat4(const T& x)
    {
        // Initialize diagonal
        m_data[0] = x;
        m_data[1] = {};
        m_data[2] = {};
        m_data[3] = {};

        m_data[4] = {};
        m_data[5] = x;
        m_data[6] = {};
        m_data[7] = {};

        m_data[8] = {};
        m_data[9] = {};
        m_data[10] = x;
        m_data[11] = {};

        m_data[12] = {};
        m_data[13] = {};
        m_data[14] = {};
        m_data[15] = x;
    }

    mat4(const vec4<T>& a, const vec4<T>& b, const vec4<T>& c, const vec4<T>& d)
    {
        // Column-major
        m_data[0] = a.x; m_data[4] = b.x; m_data[8] = c.x; m_data[12] = d.x;
        m_data[1] = a.y; m_data[5] = b.y; m_data[9] = c.y; m_data[13] = d.y;
        m_data[2] = a.z; m_data[6] = b.z; m_data[10] = c.z; m_data[14] = d.z;
        m_data[3] = a.w; m_data[7] = b.w; m_data[11] = c.w; m_data[15] = d.w;
    }

    mat4(const T& x00, const T& x01, const T& x02, const T& x03,
        const T& x10, const T& x11, const T& x12, const T& x13,
        const T& x20, const T& x21, const T& x22, const T& x23,
        const T& x30, const T& x31, const T& x32, const T& x33)
    {
        // Column-major
        m_data[0] = x00; m_data[4] = x10; m_data[8] = x20; m_data[12] = x30;
        m_data[1] = x01; m_data[5] = x11; m_data[9] = x21; m_data[13] = x31;
        m_data[2] = x02; m_data[6] = x12; m_data[10] = x22; m_data[14] = x32;
        m_data[3] = x03; m_data[7] = x13; m_data[11] = x23; m_data[15] = x33;
    }

    const vec4<T>& operator[](size_t index) const
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    vec4<T>& operator[](size_t index)
    {
        TINKER_ASSERT(index < numCols);
        return m_cols[index];
    }

    mat4<T> operator+(const mat4<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] + other.m_data[i];
        }
        return { newData };
    }

    mat4<T> operator-(const mat4<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] - other.m_data[i];
        }
        return { newData };
    }

    mat4<T> operator*(const mat4<T>& other) const
    {
        mat4<T> newMat;

        for (uint8 dstCol = 0; dstCol < numCols; ++dstCol)
        {
            for (uint8 dstRow = 0; dstRow < numCols; ++dstRow)
            {
                const vec4<T> row = vec4<T>((*this)[0][dstRow], (*this)[1][dstRow], (*this)[2][dstRow], (*this)[3][dstRow]);
                const vec4<T>& col = other[dstCol];
                newMat[dstCol][dstRow] = Dot(row, col);
            }
        }

        return newMat;
    }

    mat4<T> operator/(const mat4<T>& other) const
    {
        T newData[numEles];
        for (uint8 i = 0; i < numEles; ++i)
        {
            newData[i] = m_data[i] / other.m_data[i];
        }
        return { newData };
    }

    void operator+=(const mat4<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] += other.m_data[i];
        }
    }

    void operator-=(const mat4<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] -= other.m_data[i];
        }
    }

    void operator*=(const mat4<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] *= other.m_data[i];
        }
    }

    void operator/=(const mat4<T>& other)
    {
        for (uint8 i = 0; i < numEles; ++i)
        {
            m_data[i] /= other.m_data[i];
        }
    }
};

template <typename T>
inline vec4<T> operator*(const mat4<T>& m, const vec4<T>& v)
{
    return {
        m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w,
        m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w,
        m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w,
        m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w
    };
}

template <typename T>
vec3<T> operator*(const T& scale, const vec3<T>& v)
{
    return { v.x * scale, v.y * scale, v.z * scale };
}

template <typename T>
bool operator==(const mat4<T>& lhs, const mat4<T>& rhs)
{
    bool equal = true;
    for (uint8 i = 0; i < 16; ++i)
    {
        equal &= lhs.m_data[i] == rhs.m_data[i];
    }
    return equal;
}

template <typename T>
bool operator!=(const mat4<T>& lhs, const mat4<T>& rhs)
{
    return !(lhs == rhs);
}

inline float Length(const vec3<float>& v)
{
    float len2 = v.x * v.x + v.y * v.y + v.z * v.z;
    // TODO: custom sqrt impl
    return sqrtf(len2);
}

inline void Normalize(vec3<float>& v)
{
    float denom = Length(v);
    v /= denom;
}

template <typename T>
inline mat4<T> RotationMatrix_AxisAngle(const vec3<T>& axis, float angleRads)
{
    float cosT = cosf(angleRads);
    float sinT = sinf(angleRads);
    float oneMinusCosT = 1 - cosT;

    mat4<T> mat;
    mat[0][0] = cosT + axis.x * axis.x * oneMinusCosT;          mat[1][0] = axis.x * axis.y * oneMinusCosT - axis.z * sinT; mat[2][0] = axis.x * axis.z * oneMinusCosT + axis.y * sinT; mat[3][0] = 0;
    mat[0][1] = axis.y * axis.x * oneMinusCosT + axis.z * sinT; mat[1][1] = cosT + axis.y * axis.y * oneMinusCosT;          mat[2][1] = axis.y * axis.z * oneMinusCosT - axis.x * sinT; mat[3][1] = 0;
    mat[0][2] = axis.z * axis.x * oneMinusCosT - axis.y * sinT; mat[1][2] = axis.z * axis.y * oneMinusCosT + axis.x * sinT; mat[2][2] = cosT + axis.z * axis.z * oneMinusCosT;          mat[3][2] = 0;
    mat[0][3] = 0;                                              mat[1][3] = 0;                                              mat[2][3] = 0;                                              mat[3][3] = 1;
    return mat;
}

struct AABB3D
{
    vec3<float> minExt;
    vec3<float> maxExt;

    void InitInvalidMinMax()
    {
        for (uint32 i = 0; i < 3; ++i) { minExt[i] = FLT_MAX; }
        for (uint32 i = 0; i < 3; ++i) { maxExt[i] = -FLT_MAX; }
    }

    void ExpandTo(const vec3<float>& point)
    {
        for (uint32 i = 0; i < 3; ++i) { minExt[i] = Min(minExt[i], point[i]); }
        for (uint32 i = 0; i < 3; ++i) { maxExt[i] = Max(maxExt[i], point[i]); }
    }

    bool Intersects(const AABB3D& other) const
    {
        return false;
    }
};

// Vector Ops
#include <smmintrin.h>
#include <xmmintrin.h>

// mulps - SSE1
// mulloepi32 - SSE4.1

namespace VectorOps
{
    inline void Mul_SIMD(const vec2<float>* RESTRICT v, const mat2<float>* RESTRICT m, vec2<float>* RESTRICT out)
    {
        // NOTE: mm_stream for __m64 type is not available on AMD.
        // We probably don't need a SIMD mul for v2f * m2f. We could write one that multiplies
        // two v2f's * m2f and use mm_stream for __m128, but the inputs would have to be contiguous (otherwise we use mm_setr)
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        /*__m128 vx = _mm_setr_ps(v->x, v->x, 0.0f, 0.0f);
        __m128 vy = _mm_setr_ps(v->y, v->y, 0.0f, 0.0f);
        __m128 mcol1 = _mm_setr_ps((*m)[0][0], (*m)[0][1], 0.0f, 0.0f);
        __m128 mcol2 = _mm_setr_ps((*m)[1][0], (*m)[1][1], 0.0f, 0.0f);

        __m128 xProd = _mm_mul_ps(mcol1, vx);
        __m128 yProd = _mm_mul_ps(mcol2, vy);

        __m128 sum = _mm_add_ps(xProd, yProd);

        _mm_stream_pi((__m64*)out, *(__m64*)&sum);*/
        *out = *m * *v;
    }

    inline void Mul_SIMD(const vec2<int32>* RESTRICT v, const mat2<int32>* RESTRICT m, vec2<int32>* RESTRICT out)
    {
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        /*__m128i vx = _mm_setr_epi32(v->x, v->x, 0, 0);
        __m128i vy = _mm_setr_epi32(v->y, v->y, 0, 0);
        __m128i mcol1 = _mm_setr_epi32((*m)[0][0], (*m)[0][1], 0, 0);
        __m128i mcol2 = _mm_setr_epi32((*m)[1][0], (*m)[1][1], 0, 0);

        __m128i xProd = _mm_mullo_epi32(mcol1, vx);
        __m128i yProd = _mm_mullo_epi32(mcol2, vy);

        __m128i sum = _mm_add_epi32(xProd, yProd);

        _mm_stream_pi((__m64*)out, *(__m64*)&sum);*/
        *out = *m * *v;
    }

    inline void Mul_SIMD(const vec2<uint32>* RESTRICT v, const mat2<uint32>* RESTRICT m, vec2<uint32>* RESTRICT out)
    {
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        /*__m128i vx = _mm_setr_epi32(v->x, v->x, 0, 0);
        __m128i vy = _mm_setr_epi32(v->y, v->y, 0, 0);
        __m128i mcol1 = _mm_setr_epi32((*m)[0][0], (*m)[0][1], 0, 0);
        __m128i mcol2 = _mm_setr_epi32((*m)[1][0], (*m)[1][1], 0, 0);

        __m128i xProd = _mm_mullo_epi32(mcol1, vx);
        __m128i yProd = _mm_mullo_epi32(mcol2, vy);

        __m128i sum = _mm_add_epi32(xProd, yProd);

        _mm_stream_pi((__m64*)out, *(__m64*)&sum);*/
        *out = *m * *v;
    }

    inline void Mul_SIMD(const vec4<float>* RESTRICT v, const mat4<float>* m, vec4<float>* RESTRICT out)
    {
        // All parameters must be 16-byte aligned
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        __m128 vec = _mm_load_ps((v->m_data));
        __m128 vx = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(0, 0, 0, 0));
        __m128 vy = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(1, 1, 1, 1));
        __m128 vz = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(2, 2, 2, 2));
        __m128 vw = _mm_shuffle_ps(vec, vec, _MM_SHUFFLE(3, 3, 3, 3));

        __m128 mcol1 = _mm_load_ps((*m)[0].m_data);
        __m128 mcol2 = _mm_load_ps((*m)[1].m_data);
        __m128 mcol3 = _mm_load_ps((*m)[2].m_data);
        __m128 mcol4 = _mm_load_ps((*m)[3].m_data);

        __m128 xProd = _mm_mul_ps(mcol1, vx);
        __m128 yProd = _mm_mul_ps(mcol2, vy);
        __m128 zProd = _mm_mul_ps(mcol3, vz);
        __m128 wProd = _mm_mul_ps(mcol4, vw);

        __m128 sum1 = _mm_add_ps(xProd, yProd);
        __m128 sum2 = _mm_add_ps(zProd, wProd);
        __m128 sum = _mm_add_ps(sum1, sum2);

        _mm_stream_ps((float*)out, sum);
    }

    inline void Mul_SIMD(const vec4<int32>* RESTRICT v, const mat4<int32>* m, vec4<int32>* RESTRICT out)
    {
        // All parameters must be 16-byte aligned
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        __m128i vec = *(__m128i*)((v->m_data));
        __m128i vx = _mm_shuffle_epi32(vec, _MM_SHUFFLE(0, 0, 0, 0));
        __m128i vy = _mm_shuffle_epi32(vec, _MM_SHUFFLE(1, 1, 1, 1));
        __m128i vz = _mm_shuffle_epi32(vec, _MM_SHUFFLE(2, 2, 2, 2));
        __m128i vw = _mm_shuffle_epi32(vec, _MM_SHUFFLE(3, 3, 3, 3));

        __m128i mcol1 = *(__m128i*)((*m)[0].m_data);
        __m128i mcol2 = *(__m128i*)((*m)[1].m_data);
        __m128i mcol3 = *(__m128i*)((*m)[2].m_data);
        __m128i mcol4 = *(__m128i*)((*m)[3].m_data);

        __m128i xProd = _mm_mullo_epi32(mcol1, vx);
        __m128i yProd = _mm_mullo_epi32(mcol2, vy);
        __m128i zProd = _mm_mullo_epi32(mcol3, vz);
        __m128i wProd = _mm_mullo_epi32(mcol4, vw);

        __m128i sum1 = _mm_add_epi32(xProd, yProd);
        __m128i sum2 = _mm_add_epi32(zProd, wProd);
        __m128i sum = _mm_add_epi32(sum1, sum2);

        _mm_stream_si128((__m128i*)out, sum);
    }

    inline void Mul_SIMD(const vec4<uint32>* RESTRICT v, const mat4<uint32>* m, vec4<uint32>* RESTRICT out)
    {
        // All parameters must be 16-byte aligned
        TINKER_ASSERT(!(((size_t)v) & 15));
        TINKER_ASSERT(!(((size_t)m) & 15));
        TINKER_ASSERT(!(((size_t)out) & 15));

        __m128i vec = *(__m128i*)((v->m_data));
        __m128i vx = _mm_shuffle_epi32(vec, _MM_SHUFFLE(0, 0, 0, 0));
        __m128i vy = _mm_shuffle_epi32(vec, _MM_SHUFFLE(1, 1, 1, 1));
        __m128i vz = _mm_shuffle_epi32(vec, _MM_SHUFFLE(2, 2, 2, 2));
        __m128i vw = _mm_shuffle_epi32(vec, _MM_SHUFFLE(3, 3, 3, 3));

        __m128i mcol1 = *(__m128i*)((*m)[0].m_data);
        __m128i mcol2 = *(__m128i*)((*m)[1].m_data);
        __m128i mcol3 = *(__m128i*)((*m)[2].m_data);
        __m128i mcol4 = *(__m128i*)((*m)[3].m_data);

        __m128i xProd = _mm_mullo_epi32(mcol1, vx);
        __m128i yProd = _mm_mullo_epi32(mcol2, vy);
        __m128i zProd = _mm_mullo_epi32(mcol3, vz);
        __m128i wProd = _mm_mullo_epi32(mcol4, vw);

        __m128i sum1 = _mm_add_epi32(xProd, yProd);
        __m128i sum2 = _mm_add_epi32(zProd, wProd);
        __m128i sum = _mm_add_epi32(sum1, sum2);

        _mm_stream_si128((__m128i*)out, sum);
    }
}

}
}

typedef Tk::Core::vec2<uint32> v2ui;
typedef Tk::Core::vec3<uint32> v3ui;
typedef Tk::Core::vec4<uint32> v4ui;
typedef Tk::Core::vec2<int32>  v2i;
typedef Tk::Core::vec3<int32>  v3i;
typedef Tk::Core::vec4<int32>  v4i;
typedef Tk::Core::vec2<float>  v2f;
typedef Tk::Core::vec3<float>  v3f;
typedef Tk::Core::vec4<float>  v4f;

typedef Tk::Core::mat2<uint32> m2ui;
typedef Tk::Core::mat3<uint32> m3ui;
typedef Tk::Core::mat4<uint32> m4ui;
typedef Tk::Core::mat2<int32>  m2i;
typedef Tk::Core::mat3<int32>  m3i;
typedef Tk::Core::mat4<int32>  m4i;
typedef Tk::Core::mat2<float>  m2f;
typedef Tk::Core::mat3<float>  m3f;
typedef Tk::Core::mat4<float>  m4f;
