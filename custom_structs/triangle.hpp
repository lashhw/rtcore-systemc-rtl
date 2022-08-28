#ifndef DEFAULT_SYSTEMC_TRIANGLE_HPP
#define DEFAULT_SYSTEMC_TRIANGLE_HPP

#include "vec3.hpp"
#include "bounding_box.hpp"

struct Triangle {
    Triangle() { }
    Triangle(const Vec3 &p0, const Vec3 &p1, const Vec3 &p2)
        : p0(p0), e1(p0-p1), e2(p2-p0), n(cross(e1, e2)) { }

    Vec3 p1() const { return p0 - e1; }
    Vec3 p2() const { return p0 + e2; }
    Vec3 center() const { return (p0 + p1() + p2()) * (1.f / 3.f); }
    BoundingBox bounding_box() const;

    Vec3 p0, e1, e2, n;
};

BoundingBox Triangle::bounding_box() const {
    BoundingBox bbox;

    Vec3 p1_ = p1();
    Vec3 p2_ = p2();

    bbox.bounds[0] = fminf(p0.x, fminf(p1_.x, p2_.x));
    bbox.bounds[2] = fminf(p0.y, fminf(p1_.y, p2_.y));
    bbox.bounds[4] = fminf(p0.z, fminf(p1_.z, p2_.z));

    bbox.bounds[1] = fmaxf(p0.x, fmaxf(p1_.x, p2_.x));
    bbox.bounds[3] = fmaxf(p0.y, fmaxf(p1_.y, p2_.y));
    bbox.bounds[5] = fmaxf(p0.z, fmaxf(p1_.z, p2_.z));

    return bbox;
}

#endif //DEFAULT_SYSTEMC_TRIANGLE_HPP
