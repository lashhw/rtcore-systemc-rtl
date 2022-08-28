#ifndef DEFAULT_SYSTEMC_RAY_HPP
#define DEFAULT_SYSTEMC_RAY_HPP

#include <cfloat>
#include "vec3.hpp"

struct Ray {
    Ray() { }
    Ray(const Vec3 &origin,
        const Vec3 &unit_d,
        float tmax = FLT_MAX)
        : origin(origin), unit_d(unit_d), tmax(tmax) { }
    bool operator==(const Ray &rhs) const;

    Vec3 origin;
    Vec3 unit_d;
    float tmax;
};

bool Ray::operator==(const Ray &rhs) const {
    return origin == rhs.origin && unit_d == rhs.unit_d && tmax == rhs.tmax;
}

std::ostream& operator<<(std::ostream& os, const Ray&) {
    os << "[NOT IMPLEMENTED]";
    return os;
}

void sc_trace(sc_trace_file *tf, const Ray &ray, const std::string &name) {
    sc_trace(tf, ray.origin, name + ".origin");
    sc_trace(tf, ray.unit_d, name + ".unit_d");
    sc_trace(tf, ray.tmax, name + ".tmax");
}

#endif //DEFAULT_SYSTEMC_RAY_HPP
