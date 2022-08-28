#ifndef DEFAULT_SYSTEMC_VEC3_HPP
#define DEFAULT_SYSTEMC_VEC3_HPP

struct Vec3 {
    Vec3() { }
    Vec3(float x, float y, float z) : x(x), y(y), z(z) { }

    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    bool operator==(const Vec3 &rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }

    Vec3& operator+=(const Vec3 &v2);
    Vec3& operator-=(const Vec3 &v2);
    Vec3& operator*=(const Vec3 &v2);
    Vec3& operator*=(float t);

    float x, y, z;
};

Vec3 operator+(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z);
}

Vec3 operator-(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z);
}

Vec3 operator*(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(v1.x*v2.x, v1.y*v2.y, v1.z*v2.z);
}

Vec3 operator*(const Vec3 &v, float t) {
    return Vec3(v.x * t, v.y * t, v.z * t);
}

Vec3 operator*(float t, const Vec3 &v) {
    return Vec3(v.x * t, v.y * t, v.z * t);
}

float dot(const Vec3 &v1, const Vec3 &v2) {
    return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vec3 cross(const Vec3 &v1, const Vec3 &v2) {
    return Vec3(v1.y * v2.z - v1.z * v2.y,
                v1.z * v2.x - v1.x * v2.z,
                v1.x * v2.y - v1.y * v2.x);
}

Vec3& Vec3::operator+=(const Vec3 &v2) {
    x += v2.x;
    y += v2.y;
    z += v2.z;
    return *this;
}

Vec3& Vec3::operator-=(const Vec3 &v2) {
    x -= v2.x;
    y -= v2.y;
    z -= v2.z;
    return *this;
}

Vec3& Vec3::operator*=(const Vec3 &v2) {
    x *= v2.x;
    y *= v2.y;
    z *= v2.z;
    return *this;
}

Vec3& Vec3::operator*=(float t) {
    x *= t;
    y *= t;
    z *= t;
    return *this;
}

void sc_trace(sc_trace_file *tf, const Vec3 &v, const std::string &name) {
    sc_trace(tf, v.x, name + ".x");
    sc_trace(tf, v.y, name + ".y");
    sc_trace(tf, v.z, name + ".z");
}

#endif //DEFAULT_SYSTEMC_VEC3_HPP
