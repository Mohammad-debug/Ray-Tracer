#ifndef SPHERE_H
#define SPHERE_H

#include "vec3.h"
#include "hittable.h"
#include "interval.h"
#include "color.h"

class sphere : public hittable {
private:
    point3 center;
    double radius;
    color  colorOfoject;
    bool glaze;
public:
	sphere(point3 _center, double _radius, color _colorOfoject, bool _glaze) : center(_center), radius(_radius), colorOfoject(_colorOfoject), glaze(_glaze){}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 oc = r.origin() - center;
        auto a = r.direction().length_squared();
        auto half_b = dot(oc, r.direction());
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        // Find the nearest root that lies in the acceptable range.
        auto root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.col = colorOfoject;
        rec.glaze = glaze;
        return true;
    }
};

#endif