#ifndef RUN_MULTI_H
#define RUN_MULTI_H

#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "rendering.h"
#include "scene.h"
#include "triangle.h"
#include "vec3.h"

struct mt_worker_args
{
    // Image to write to
    struct rgb_image *image;
    // The scene
    struct scene *scene;
    // Renderer used
    render_mode_f renderer;
    // Compute image pixels between y_from to y_to
    size_t y_from;
    size_t y_to;
};

int runner_multithread(struct rgb_image *image, struct scene *scene,
                       render_mode_f renderer);

#endif
