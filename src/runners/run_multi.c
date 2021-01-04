#include <err.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include "bmp.h"
#include "camera.h"
#include "image.h"
#include "normal_material.h"
#include "obj_loader.h"
#include "phong_material.h"
#include "rendering.h"
#include "runners/run_multi.h"
#include "scene.h"
#include "sphere.h"
#include "triangle.h"
#include "utils/alloc.h"
#include "vec3.h"

/*
** Split task for threads
*/
int mt_split_tasks(struct rgb_image *image, struct scene *scene,
                   render_mode_f renderer, struct mt_worker_args **args_lists,
                   size_t threads)
{
    // Init arguments structs
    for (size_t arg = 0; arg < threads; arg++)
        args_lists[arg] = xcalloc(1, sizeof(struct mt_worker_args));

    // Divide tasks

    // Number of line per thread
    size_t nb_line_thread = image->height / threads;
    // If it exist a remainder (The image height can't be perfectly splitted
    // Between threads)
    size_t nb_line_thread_remain = image->height % threads;

    // Initialize every struct with correct data
    for (size_t arg = 0; arg < threads; arg++)
    {
        args_lists[arg]->image = image;
        args_lists[arg]->renderer = renderer;
        args_lists[arg]->scene = scene;
        args_lists[arg]->y_from = arg * nb_line_thread;
        args_lists[arg]->y_to = arg * nb_line_thread + nb_line_thread;
    }

    // For now, only the last thread have more work
    args_lists[threads - 1]->y_to += nb_line_thread_remain;

    return 0;
}

/*
** Task given to thread
*/
void *worker(void *arg)
{
    // Get arguments
    struct mt_worker_args *worker_data = (struct mt_worker_args *)arg;

    // Logging
    printf("MT: FROM %li TO %li\n", worker_data->y_from, worker_data->y_to);
    fflush(stdout);

    // Apply the renderer to some pixels of the image
    for (size_t y = worker_data->y_from; y < worker_data->y_to; y++)
        for (size_t x = 0; x < worker_data->image->width; x++)
            worker_data->renderer(worker_data->image, worker_data->scene, x, y);

    return NULL;
}

/*
** Multithreaded runner
*/
int runner_multithread(struct rgb_image *image, struct scene *scene,
                       render_mode_f renderer, size_t threads_requested)
{
    size_t thread_number
        = threads_requested; /* Number of thread(s) requested */
    pthread_t *threads; /* Pthread ID */
    size_t current_thread;
    // This is the structure given to the threads as data
    struct mt_worker_args **thread_data;

    // Check the number of thread
    if (thread_number == 0)
        warnx("Invalid number of threads - Got %li, expected at least 1",
              thread_number);
    else
        warnx("MULTI-THREADED RUNNER - Using %li threads", thread_number);

    // Create a array of thread id
    threads = xalloc(thread_number * sizeof(pthread_t));

    // Create the thread argument list
    thread_data = xalloc(thread_number * sizeof(struct mt_worker_args));

    // Set arguments array and divide task for threads
    mt_split_tasks(image, scene, renderer, thread_data, thread_number);

    // Spawn the threads and give them data
    for (current_thread = 0; current_thread < thread_number; current_thread++)
    {
        // Spawn a thread and set the data for each of them
        if (pthread_create(&threads[current_thread], NULL, worker,
                           thread_data[current_thread])
            != 0)
        {
            // The thread creation failed
            warnx("Failed thread creation");
            return 1;
        }
    }

    // Join thread(s) when task is finished
    for (current_thread = 0; current_thread < thread_number; current_thread++)
    {
        pthread_join(threads[current_thread], NULL);
    }

    // Free thread list
    free(threads);

    // Free thread data
    for (size_t i = 0; i < thread_number; i++)
        free(thread_data[i]);

    // Free the thread data array containet
    free(thread_data);

    // Logging - Completed render
    warnx("MT - Complete");

    // Success!
    return 0;
}
