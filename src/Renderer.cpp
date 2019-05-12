//
// Created by James on 2019/4/9.
//

#include "Renderer.h"
#include "scene/Scene.h"
#include "camera/All.h"
#include "alg/All.h"
#include "utils/funcs.hpp"
#include <fstream>
#include <omp.h>

Renderer::~Renderer()
{
	delete scene;
	delete camera;
}

void Renderer::getReady()
{
	if (prev_ppm_path.length() > 0) {	// load from checkpoint
		camera->readPPM(prev_ppm_path, prev_cpt_path);
		printf("loaded previous status from \"%s\"\n", prev_ppm_path.data());
	}
	delete is_edge;
	is_edge = new bool[camera->size];
	memset(is_edge, false, camera->size * sizeof(bool));
}

void Renderer::checkIfReady()
{
	if (scene == nullptr) TERMINATE("Error: scene is not setup yet.")
	if (camera == nullptr) TERMINATE("Error: camera is not setup yet.")
	if (algorithm == nullptr) TERMINATE("Error: algorithm is not setup yet.")
	if (is_edge == nullptr) TERMINATE("Error: is_edge is not setup yet.")

	printf("\n----------------------------------------------------------------\n");
	printf("loaded %ld objects, %ld triangle meshes in total.\n",
		   scene->getSingletonCount(), scene->getMeshCount());
	printf("camera viewpoint at %s  orienting towards %s\n",
		   camera->viewpoint().toString().data(), camera->orientation().toString().data());
	printf("algorithm info: %s", algorithm->info().data());
	printf("\n---------------------- ready to render -------------------------\n\n");
}

void Renderer::renderFrame()
{
	if (verbose_step == 0)
		render();
	else
		renderVerbose();
}

void Renderer::start()
{
	checkIfReady();
	scene->buildKDTree();
	printf("===== rendering start =====\n");

	double since = omp_get_wtime();
	renderFrame();
	auto elapse = lround(omp_get_wtime() - since);

	printf("\n===== rendering finished in %ld min %ld sec =====\n", elapse / 60, elapse % 60);
}

void Renderer::startKinetic(size_t n_frame, void (*motion)())
{
//	if (prev_cpt_path.length() > 0) {
//		TERMINATE("Error: before rendering kinetic images, you should not pre-read from previous image.");
//	}
//	checkIfReady();
//
//	printf("======= kinetic rendering start =======\n");
//
//	double since = omp_get_wtime();
//	for (size_t frame = 0; frame < n_frame; ++frame) {
//		printf("\n===== frame %ld / %ld =====\n", frame + 1, n_frame);
//		renderFrame();
//		motion();    // deal with motion
//		if (checkpoint) {    // save checkpoint for each frame
//			Buffer out_path;
//			sprintf(out_path, "%s/frame - %ld (samps = %ld).ppm", checkpoint_dir.data(), frame + 1, n_epoch);
//			camera->writePPM(out_path);
//		}
//	}
//	auto elapse = lround(omp_get_wtime() - since);
//
//	printf("\n======= kinetic rendering finished in %ld min %ld sec =======\n", elapse / 60, elapse % 60);
}

void Renderer::save() const
{
	if (camera == nullptr) TERMINATE("Error: camera is not setup yet.");
	camera->writePPM(save_ppm_path, save_cpt_path);
	printf("image written to \"%s\" \n", save_ppm_path.data());
}

void Renderer::saveProgress(size_t cur_epoch) const
{
	if (save_step == 0 || cur_epoch % save_step > 0) return;
	camera->writePPM(save_ppm_path, save_cpt_path);
	barInfo("\t progress saved to \"%s\"\n", save_ppm_path.data());
}

#define SUB_D1 0.45
#define SUB_D2 0.05

void Renderer::render()
{
// without progressbar, fast version

//	detectEdges();
	double since = omp_get_wtime();

	for (size_t epoch = 0; epoch < n_epoch; ++epoch) {    // for samples
		auto eta = lround((omp_get_wtime() - since) * (n_epoch - epoch) / epoch);
		barInfo("\r=== epoch %ld / %ld ===  eta: %ld min %ld sec", epoch + 1, n_epoch, eta / 60, eta % 60);

#ifdef __USE_OMP__
#pragma omp parallel for schedule(dynamic, 1)
#endif
		for (size_t j = 0; j < camera->height; ++j) {                // for each pixel
			for (size_t i = 0, rank = j * camera->width; i < camera->width; ++i, ++rank) {
				if (is_edge[rank]) {                                    // MSAA - for 4 subpixels
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i - SUB_D1, j - SUB_D2)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i - SUB_D2, j + SUB_D1)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i + SUB_D2, j - SUB_D1)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i + SUB_D1, j + SUB_D2)));
				}
				else {
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i, j, 0.5)));    // just rand normal - 0.5
				}
			}
		}
		saveProgress(epoch + 1);
	}
	printf("\n");
}

void Renderer::renderVerbose()
{
	// with progressbar

//	detectEdges();
	double since = omp_get_wtime();

	for (size_t epoch = 0; epoch < n_epoch; ++epoch) {    // for samples
		auto eta = lround((omp_get_wtime() - since) * (n_epoch - epoch) / epoch);
		barInfo("=== epoch %ld / %ld ===  eta: %ld min %ld sec\n", epoch + 1, n_epoch, eta / 60, eta % 60);

#ifdef __USE_OMP__
#pragma omp parallel for schedule(dynamic, 1)
#endif

		for (size_t j = 0; j < camera->height; ++j) {                // for each pixel
			if (j % verbose_step == 0) {
				barInfo("\r %.1f %%", j * 100.0 / camera->height);    // progressbar :)
			}
			for (size_t i = 0, rank = j * camera->width; i < camera->width; ++i, ++rank) {
				if (is_edge[rank]) {                                    // MSAA - for 4 subpixels
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i - SUB_D1, j - SUB_D2)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i - SUB_D2, j + SUB_D1)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i + SUB_D2, j - SUB_D1)));
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i + SUB_D1, j + SUB_D2)));
				}
				else {
					camera->render(rank, algorithm->radiance(camera->shootRayAt(i, j, 0.5)));    // just rand normal - 0.5
				}
			}
		}
		saveProgress(epoch + 1);
		barInfo("\n");
	}
}

void Renderer::detectEdges()
{
	printf("detecting edges...\n");
	fflush(stdout);
#ifdef __USE_OMP__
#pragma omp parallel for schedule(dynamic, 1)        // OpenMP
#endif
	for (size_t j = 0; j < camera->height; ++j) {
		barInfo("\r %.1f %%", j * 100.0 / camera->height);    // progressbar :)
		for (size_t i = 0, rank = j * camera->width; i < camera->width; ++i, ++rank) {
			// four sub rays
			const Object
					*hit0 = scene->hitOf(camera->shootRayAt(i - SUB_D1, j - SUB_D2)),
					*hit1 = scene->hitOf(camera->shootRayAt(i - SUB_D2, j + SUB_D1)),
					*hit2 = scene->hitOf(camera->shootRayAt(i + SUB_D2, j - SUB_D1)),
					*hit3 = scene->hitOf(camera->shootRayAt(i + SUB_D1, j + SUB_D2));
			is_edge[rank] = !(hit0 == hit1 && hit0 == hit2 && hit0 == hit3);    // if hit different objs, mark as edge
		}
	}
//#ifdef __DEV_STAGE__
//	std::ofstream fout("is_edge.ppm");
//	if (!fout.is_open()) TERMINATE("not open is_edge");
//	fout << "P3 " << camera->width << " " << camera->height << "\n255\n";
//	for (size_t rank = 0; rank < camera->size; ++rank) {
//		fout << (is_edge[rank] ? "0 0 0 " : "255 255 255 ");
//	}
//	fout.close();
//#endif
}

#undef SUB_D1
#undef SUB_D2
