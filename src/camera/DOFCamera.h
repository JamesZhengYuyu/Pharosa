//
// Created by James on 2019/4/2.
//

#ifndef PHAROSA_DOFCAMERA_H
#define PHAROSA_DOFCAMERA_H

#include "Camera.h"

// depth of field camera
class DOFCamera : public Camera
{
public:
	const real length, focus, aperture;	// length of camera, distance to focus plane, random circle range
	// cache:
	const real f_l;	// focus / length
	const Pos pos_ez_length, ez_focus;	// c + ez * length, ez * focus

	DOFCamera(const Json &json);

	Ray shootRayAt(real i, real j) const override;
};

#endif //PHAROSA_DOFCAMERA_H
