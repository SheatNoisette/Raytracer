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

    struct vec3 pix_color = {0};
    pix_color = vec3_add(&pix_color, &ambient_contribution);
    pix_color = vec3_add(&pix_color, &diffuse_contribution);
    pix_color = vec3_add(&pix_color, &specular_contribution);

    // Create a new ray that has ben reflected
    struct ray reflexion
        = {.source = inter->point,
           .direction = vec3_reflect(&ray->direction, &inter->normal)};

    // The new ray intersection
    struct object_intersection new_intersection;

    // Check the recursion depth and check if our new intersection isn't sent
    // To the **VOID**. If so, return the last valid material color.
    if (depth == MAX_DEPTH
        || isinf(scene_intersect_ray(&new_intersection, scene, &reflexion)))
    {
        return pix_color;
    }

    // Compute the material color for the reflected ray recursively
    struct vec3 bounce = phong_metarial_shade(new_intersection.material,
                                              &new_intersection.location, scene,
                                              &reflexion, depth + 1);

    // Get the phong material from the new material of which the ray bounced
    const struct phong_material *mat_bounce
        = (const struct phong_material *)new_intersection.material;

    // Set the reflexion Ks to the material which new ray touched
    struct vec3 bounce_ks = vec3_mul(&bounce, mat_bounce->spec_Ks);
    // Add the reflexion material color to the previous material color
    struct vec3 final_pixel = vec3_add(&bounce_ks, &pix_color);

    return final_pixel;
}
