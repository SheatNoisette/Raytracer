#include "phong_material.h"
#include "scene.h"

#define MAX_DEPTH 2

double scene_intersect_ray(struct object_intersection *closest_intersection,
                           const struct scene *scene, struct ray *ray);

struct vec3 phong_metarial_shade(const struct material *base_material,
                                 const struct intersection *inter,
                                 const struct scene *scene,
                                 const struct ray *ray, size_t depth)
{
    const struct phong_material *mat
        = (const struct phong_material *)base_material;

    // a coefficient teaking how much diffuse light to add
    struct vec3 light = vec3_mul(&scene->light_color, scene->light_intensity);
    struct vec3 diffuse_light_color = vec3_mul_vec(&light, &mat->surface_color);

    // compute the diffuse lighting contribution by applying the cosine
    // law
    double diffuse_intensity
        = -vec3_dot(&inter->normal, &scene->light_direction);
    if (diffuse_intensity < 0)
        diffuse_intensity = 0;

    struct vec3 diffuse_contribution
        = vec3_mul(&diffuse_light_color, diffuse_intensity * mat->diffuse_Kn);

    // compute the specular reflection contribution

    struct vec3 light_reflection_dir
        = vec3_reflect(&scene->light_direction, &inter->normal);
    struct vec3 specular_contribution = {0};
    // computes how much the reflection goes in the direction of the
    // camera
    double light_reflection_proj
        = -vec3_dot(&light_reflection_dir, &ray->direction);
    if (light_reflection_proj < 0.0)
        light_reflection_proj = 0.0;
    else
    {
        double spec_coeff
            = pow(light_reflection_proj, mat->spec_n) * mat->spec_Ks;
        specular_contribution = vec3_mul(&scene->light_color, spec_coeff);
    }

    double ambient_intensity = 0.2;
    struct vec3 ambient_contribution
        = vec3_mul(&mat->surface_color, ambient_intensity);

    // Reflexion
    struct ray reflexion
        = {.source = inter->point,
           .direction = vec3_reflect(&ray->direction, &inter->normal)};

    // Object new intersection
    struct object_intersection new_intersection;

    struct vec3 pix_color = {0};
    pix_color = vec3_add(&pix_color, &ambient_contribution);
    pix_color = vec3_add(&pix_color, &diffuse_contribution);
    pix_color = vec3_add(&pix_color, &specular_contribution);

    // Check the recursion depth
    if (depth == MAX_DEPTH
        || isinf(scene_intersect_ray(&new_intersection, scene, &reflexion)))
    {
        return pix_color;
    }

    struct vec3 bounce = phong_metarial_shade(new_intersection.material,
                                              &new_intersection.location, scene,
                                              &reflexion, depth + 1);

    // Make the average between the old color and the new (bounced ray shaded)
    struct vec3 final_pixel = vec3_add(&bounce, &pix_color);
    final_pixel = vec3_mul(&final_pixel, .5f);

    return final_pixel;
}
