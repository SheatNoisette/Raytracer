#ifndef RENDERING_H
#define RENDERING_H

#include <stddef.h>

#include "image.h"
#include "scene.h"

/*
** Used to define the type of renderer we want
*/
typedef void (*render_mode_f)(struct rgb_image *, struct scene *, size_t x,
                              size_t y);

/*
** This define the type of runner used for rendering the image (Multithreaded or
** not,...)
*/
enum runner_type
{
    RUNNER_MULTITHREADED,
    RUNNER_SINGLETHREADED,
    RUNNER_REALTIME,
    RUNNER_UNKNOWN
};

/*
** Run if it's single threaded, multi-threaded or realtime
*/
int run_renderer(struct rgb_image *image, struct scene *scene,
                 enum runner_type runner, render_mode_f renderer);

/*
** Get runner type from options parser
*/
enum runner_type get_runner_opt(char *input);

#endif
