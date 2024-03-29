#include <err.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "antialias.h"
#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "phong_material.h"
#include "rendering.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "vec3.h"

/*
** The color of a light is encoded inside a float, from 0 to +inf,
** where 0 is no light, and +inf a lot more light. Unfortunately,
** regular images can't hold such a huge range, and each color channel
** is usualy limited to [0,255]. This function does the (lossy) translation
** by mapping the float [0,1] range to [0,255]
*/
static inline uint8_t translate_light_component(double light_comp)
{
    if (light_comp < 0.)
        light_comp = 0.;
    if (light_comp > 1.)
        light_comp = 1.;

    return light_comp * 255;
}

/*
** Converts an rgb floating point light color to 24 bit rgb.
*/
struct rgb_pixel rgb_color_from_light(const struct vec3 *light)
{
    struct rgb_pixel res;
    res.r = translate_light_component(light->x);
    res.g = translate_light_component(light->y);
    res.b = translate_light_component(light->z);
    return res;
}

#if 0
static void build_test_scene(struct scene *scene, double aspect_ratio)
{
    // create a sample red material
    struct phong_material *red_material = zalloc(sizeof(*red_material));
    phong_material_init(red_material);
    red_material->surface_color = (struct vec3){0.75, 0.125, 0.125};
    red_material->diffuse_Kn = 0.2;
    red_material->spec_n = 10;
    red_material->spec_Ks = 0.2;
    red_material->ambient_intensity = 0.1;

    // create a single sphere with the above material, and add it to the scene
    struct sphere *sample_sphere
        = sphere_create((struct vec3){0, 10, 0}, 4, &red_material->base);
    object_vect_push(&scene->objects, &sample_sphere->base);

    // go the same with a triangle
    // points are listed counter-clockwise
    //     a
    //    /|
    //   / |
    //  b--c
    struct vec3 points[3] = {
        {6, 10, 1}, // a
        {5, 10, 0}, // b
        {6, 10, 0}, // c
    };

    struct triangle *sample_triangle
        = triangle_create(points, &red_material->base);
    object_vect_push(&scene->objects, &sample_triangle->base);

    // setup the scene lighting
    scene->light_intensity = 5;
    scene->light_color = (struct vec3){1, 1, 0}; // yellow
    scene->light_direction = (struct vec3){-1, 1, -1};
    vec3_normalize(&scene->light_direction);

    // setup the camera
    double cam_width = 10;
    double cam_height = cam_width / aspect_ratio;

    scene->camera = (struct camera){
        .center = {0, 0, 0},
        .forward = {0, 1, 0},
        .up = {0, 0, 1},
        .width = cam_width,
        .height = cam_height,
        .focal_distance = focal_distance_from_fov(cam_width, 80),
    };

    // release the reference to the material
    material_put(&red_material->base);
}
#endif

static void build_obj_scene(struct scene *scene, double aspect_ratio,
                            double fov)
{
    // setup the scene lighting
    scene->light_intensity = 5;
    scene->light_color = (struct vec3){1, 1, 1};
    scene->light_direction = (struct vec3){-1, -1, -1};
    vec3_normalize(&scene->light_direction);

    // setup the camera
    double cam_width = 2;
    double cam_height = cam_width / aspect_ratio;

    // for some reason the object points in the z axis,
    // with its up on y
    scene->camera = (struct camera){
        .center = {0, 1, 2},
        .forward = {0, -1, -2},
        .up = {0, 1, 0},
        .width = cam_width,
        .height = cam_height,
        .focal_distance = focal_distance_from_fov(cam_width, fov),
    };

    vec3_normalize(&scene->camera.forward);
    vec3_normalize(&scene->camera.up);
}

static struct ray image_cast_ray(const struct rgb_image *image,
                                 const struct scene *scene, size_t x, size_t y)
{
    // find the position of the current pixel in the image plane
    // camera_cast_ray takes camera relative positions, from -0.5 to 0.5 for
    // both axis
    double cam_x = ((double)x / image->width) - 0.5;
    double cam_y = ((double)y / image->height) - 0.5;

    // find the starting point and direction of this ray
    struct ray ray;
    camera_cast_ray(&ray, &scene->camera, cam_x, cam_y);
    return ray;
}

double scene_intersect_ray(struct object_intersection *closest_intersection,
                           struct scene *scene, struct ray *ray)
{
    // we will now try to find the closest object in the scene
    // intersecting this ray
    double closest_intersection_dist = INFINITY;

    for (size_t i = 0; i < object_vect_size(&scene->objects); i++)
    {
        struct object *obj = object_vect_get(&scene->objects, i);
        struct object_intersection intersection;
        // if there's no intersection between the ray and this object, skip it
        double intersection_dist = obj->intersect(&intersection, obj, ray);
        if (intersection_dist >= closest_intersection_dist)
            continue;

        closest_intersection_dist = intersection_dist;
        *closest_intersection = intersection;
    }

    return closest_intersection_dist;
}

/* For all the pixels of the image, try to find the closest object
** intersecting the camera ray. If an object is found, shade the pixel to
** find its color.
*/
static void render_shaded(struct rgb_image *image, struct scene *scene,
                          size_t x, size_t y)
{
    struct ray ray = image_cast_ray(image, scene, x, y);

    struct object_intersection closest_intersection;
    double closest_intersection_dist
        = scene_intersect_ray(&closest_intersection, scene, &ray);

    // if the intersection distance is infinite, do not shade the pixel
    if (isinf(closest_intersection_dist))
        return;

    struct material *mat = closest_intersection.material;
    struct vec3 pix_color
        = mat->shade(mat, &closest_intersection.location, scene, &ray, 0);
    rgb_image_set(image, x, y, rgb_color_from_light(&pix_color));
}

/* For all the pixels of the image, try to find the closest object
** intersecting the camera ray. If an object is found, shade the pixel to
** find its color.
*/
static void render_normals(struct rgb_image *image, struct scene *scene,
                           size_t x, size_t y)
{
    struct ray ray = image_cast_ray(image, scene, x, y);

    struct object_intersection closest_intersection;
    double closest_intersection_dist
        = scene_intersect_ray(&closest_intersection, scene, &ray);

    // if the intersection distance is infinite, do not shade the pixel
    if (isinf(closest_intersection_dist))
        return;

    struct material *mat = closest_intersection.material;
    struct vec3 pix_color = normal_material.shade(
        mat, &closest_intersection.location, scene, &ray, 0);
    rgb_image_set(image, x, y, rgb_color_from_light(&pix_color));
}

/* For all the pixels of the image, try to find the closest object
** intersecting the camera ray. If an object is found, shade the pixel to
** find its color.
*/
static void render_distances(struct rgb_image *image, struct scene *scene,
                             size_t x, size_t y)
{
    struct ray ray = image_cast_ray(image, scene, x, y);

    struct object_intersection closest_intersection;
    double closest_intersection_dist
        = scene_intersect_ray(&closest_intersection, scene, &ray);

    // if the intersection distance is infinite, do not shade the pixel
    if (isinf(closest_intersection_dist))
        return;

    assert(closest_intersection_dist > 0);

    double depth_repr = 1 / (closest_intersection_dist + 1);
    uint8_t depth_intensity = translate_light_component(depth_repr);
    struct rgb_pixel pix_color
        = {depth_intensity, depth_intensity, depth_intensity};
    rgb_image_set(image, x, y, pix_color);
}

int main(int argc, char *argv[])
{
    // Return code of the application
    int return_code;
    // Out scene
    struct scene scene;
    // The final image
    struct rgb_image *image;
    render_mode_f renderer;
    // Type of runner for rendering
    enum runner_type runner;
    // Type of anti-aliasing used
    enum aa_type aalias_type = ANTIALIAS_NONE;
    // Aspect ratio for the camera
    double aspect_ratio;
    // Size of the image - Default is 100x100
    size_t width = 100;
    size_t height = 100;
    // Number of threads used
    size_t threads = 4;

    // Check if we have the minimum of arguments
    if (argc < 3)
    {
        errx(1, "Usage: SCENE.obj OUTPUT.bmp [--normals] [--distances] "
                "[--runner=mt/single] [--width=100] [--height=100] "
                "[--threads=4] [--aa=none/ssaa2x/ssaa4x]");
    }

    // Create the scene
    scene_init(&scene);

    // Options variables
    renderer = render_shaded;
    // By default, the runner is single threaded
    runner = RUNNER_SINGLETHREADED;

    // Parse options
    for (int i = 3; i < argc; i++)
    {
        if (strcmp(argv[i], "--normals") == 0)
            renderer = render_normals;
        else if (strcmp(argv[i], "--distances") == 0)
            renderer = render_distances;
        else if (strncmp(argv[i], "--runner", 8) == 0)
            runner = get_runner_opt(argv[i] + 8);
        else if (strncmp(argv[i], "--aa", 4) == 0)
            aalias_type = select_alias_opt(argv[i] + 4);
        else if (strncmp(argv[i], "--width", 7) == 0)
            width = atoi(argv[i] + 8);
        else if (strncmp(argv[i], "--height", 8) == 0)
            height = atoi(argv[i] + 9);
        else if (strncmp(argv[i], "--threads", 9) == 0)
            threads = atoi(argv[i] + 10);
        else
            warnx("Unknown option '%s'", argv[i]);
    }
    // Check the image size
    if (width == 0 || height == 0)
        errx(3, "Invalid size - width: %li height: %li", width, height);

    // initialize the frame buffer (the buffer that will store the result of the
    // rendering)
    warnx("Initializing base frame buffer width: %li height: %li...", width,
          height);
    image = rgb_image_alloc(width, height);

    // Apply necessary modifications for anti aliasing techniques
    preprocess_antialias(aalias_type, &image);

    // set all the pixels of the image to black
    struct rgb_pixel bg_color = {0};
    rgb_image_clear(image, &bg_color);

    // Get the aspect ratio
    aspect_ratio = (double)image->width / image->height;

    // build the scene
    build_obj_scene(&scene, aspect_ratio, 90);

    // Check if we can't load the model
    if (load_obj(&scene, argv[1]))
        return 41;

    // Run the renderer and use the runner selected
    if (run_renderer(image, &scene, runner, renderer, threads))
        errx(2, "Rendering failed!");

    // Apply a post processing anti aliasing
    postprocess_antialias(aalias_type, &image);

    // write the rendered image to a bmp file
    FILE *fp = fopen(argv[2], "w");
    if (fp == NULL)
        err(1, "failed to open the output file");

    return_code = bmp_write(image, ppm_from_ppi(80), fp);
    fclose(fp);

    // release resources
    scene_destroy(&scene);
    free(image);
    return return_code;
}
