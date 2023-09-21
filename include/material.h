#ifndef MATERIAL_H
#define MATERIAL_H

#include "vec3.h"

class material {
public:
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float shininess;
};

#endif