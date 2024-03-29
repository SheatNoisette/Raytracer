#include <err.h>
#include <stdlib.h>
#include <string.h>

#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "phong_material.h"
#include "rendering.h"
#include "runners/run_multi.h"
#include "runners/run_single.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "vec3.h"

// Number of runners
#define RUNNERS_NB 4

/*
** Get runner type from options parser
*/
enum runner_type get_runner_opt(char *input)
{
    if (strcmp(input, "=realtime") == 0)
        return RUNNER_REALTIME;
    else if (strcmp(input, "=mt") == 0)
        return RUNNER_MULTITHREADED;
    else if (strcmp(input, "=single") == 0)
        return RUNNER_SINGLETHREADED;

    // Wrong option
    return RUNNER_UNKNOWN;
}

/*
** Get the name of the runner
*/
static char *runner_str(enum runner_type runner)
{
    char *runners[RUNNERS_NB]
        = {"SINGLETHREADED", "MULTI-THREADED", "REALTIME", "UNKNOWN"};
    return runners[(int)runner];
}

/*
** Run the renderer
*/
int run_renderer(struct rgb_image *image, struct scene *scene,
                 enum runner_type runner, render_mode_f renderer,
                 size_t threads)
{
    // Logging
    warnx("Using '%s' runner.", runner_str(runner));

    if (runner == RUNNER_SINGLETHREADED)
        return runner_singlethread(image, scene, renderer);
    else if (runner == RUNNER_MULTITHREADED)
        return runner_multithread(image, scene, renderer, threads);
    else if (runner == RUNNER_REALTIME)
        warnx("REALTIME NOT IMPLEMENTED!");

    // Unknown runner
    warnx("Unknown runner detected, please check your options.");
    return 1;
}
