#ifndef RUN_SINGLE_H
#define RUN_SINGLE_H

#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "rendering.h"
#include "scene.h"
#include "triangle.h"
#include "vec3.h"

int runner_singlethread(struct rgb_image *image, struct scene *scene,
                        render_mode_f renderer);

#endif
