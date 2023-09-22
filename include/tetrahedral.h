#ifndef TETRAHEDRAL_H
#define TETRAHEDRAL_H

#include "vec3.h"
#include "hittable.h"
#include "interval.h"
#include "color.h"
class tetrahedron : public hittable
{
private:
    point3 v0, v1, v2, v3; // Vertices of the tetrahedron
    color colorOfObject;
    bool glaze;

public:
    tetrahedron(point3 _v0, point3 _v1, point3 _v2, point3 _v3, color _colorOfObject, bool _glaze)
        : v0(_v0), v1(_v1), v2(_v2), v3(_v3), colorOfObject(_colorOfObject), glaze(_glaze) {}

    bool hit(const ray &r, interval ray_t, hit_record &rec) const override
    {
        // Calculate the plane equations for each of the four triangular faces
        vec3 normal1 = unit_vector(cross(v1 - v0, v2 - v0));
        vec3 normal2 = unit_vector(cross(v2 - v1, v3 - v2));
        vec3 normal3 = unit_vector(cross(v2 - v0, v3 - v0));
        vec3 normal4 = unit_vector(cross(v0 - v1, v0 - v3));

        // Check intersection with each of the four triangular faces
        double t;
        hit_record temp_rec;
        bool hit_anything = false;

        if (triangle_intersection(r, v1, v3, v2, normal2, t, temp_rec))
        {
            if (ray_t.surrounds(t))
            {
                rec = temp_rec;
                ray_t.max = t;
                hit_anything = true;
            }
        }
        if (triangle_intersection(r, v0, v1, v2, normal1, t, temp_rec))
        {
            if (ray_t.surrounds(t))
            {
                rec = temp_rec;
                ray_t.max = t;
                hit_anything = true;
            }
        }



        if (triangle_intersection(r, v0, v2, v3, normal3, t, temp_rec))
        {
            if (ray_t.surrounds(t))
            {
                rec = temp_rec;
                ray_t.max = t;
                hit_anything = true;
            }
        }

        if (triangle_intersection(r, v0, v3, v1, normal4, t, temp_rec))
        {
            if (ray_t.surrounds(t))
            {
                rec = temp_rec;
                ray_t.max = t;
                hit_anything = true;
            }
        }

        return hit_anything;
    }

private:
    // Helper function to check intersection with a triangle
    bool triangle_intersection(const ray &r, const point3 &v0, const point3 &v1, const point3 &v2,
                               vec3 normal, double &t, hit_record &rec) const
    {

        // Calculate the plane of the triangle
       
        double d = dot(normal, v0 - r.origin());

        // Compute the denominator of the ray-plane intersection formula
        double denom = dot(normal, r.direction());

        // Check if the ray and plane are parallel (denominator close to zero)
        if (std::fabs(denom) < 0.0001)
        {
            return false;
        }

        // Calculate the parameter 't' at which the ray intersects the plane
        t = d / denom;

        // Check if the intersection point is behind the ray's origin
        // if (t < 0)
        // {
        //     return false;
        // }

        // Calculate the intersection point
        point3 intersection = r.at(t);

        // Check if the intersection point is inside the triangle
        vec3 edge0 = v1 - v0;
        vec3 edge1 = v2 - v1;
        vec3 edge2 = v0 - v2;

        vec3 c0 = intersection - v0;
        vec3 c1 = intersection - v1;
        vec3 c2 = intersection - v2;

        vec3 normal0 = cross(edge0, c0);
        vec3 normal1 = cross(edge1, c1);
        vec3 normal2 = cross(edge2, c2);

        // Check if the normals have the same direction as the triangle's normal
        if (dot(normal, normal0) >= 0 && dot(normal, normal1) >= 0 && dot(normal, normal2) >= 0)
        {
            // Intersection is inside the triangle
            rec.t = t;
            rec.p = intersection;
            rec.set_face_normal(r, normal);
            rec.col = colorOfObject;
            rec.glaze = glaze;
            return true;
        }

        return false;
      
    }
};

#endif