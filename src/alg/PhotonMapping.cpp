//
// Created by James on 2019/6/7.
//

#include "PhotonMapping.h"

PhotonMapping::PhotonMapping(const Scene &scene_, Camera &camera_,
							 size_t n_iteration_, size_t n_photon_per_iter_, size_t max_depth_) :
		Algorithm(scene_, camera_),
		n_iteration(n_iteration_), n_photon_per_iter(n_photon_per_iter_), max_depth(max_depth_)
{

}

PhotonMapping::~PhotonMapping()
{

}

String PhotonMapping::info() const
{
	return "<Photon Mapping>";
}

Color PhotonMapping::radiance(const Ray &ray) const
{

}
