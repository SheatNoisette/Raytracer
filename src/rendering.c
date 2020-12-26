#include "rendering.h"
#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "phong_material.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "vec3.h"

int run_renderer(struct rgb_image *image, struct scene *scene,
                 enum runner_type runner, render_mode_f renderer)
{
    for (size_t y = 0; y < image->height; y++)
        for (size_t x = 0; x < image->width; x++)
            renderer(image, scene, x, y);

    return 0;
}
