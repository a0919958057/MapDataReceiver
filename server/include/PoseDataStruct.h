#pragma once

struct PoseData {
	// position X, Y, Z
	// (x, y, z)
	double pose_position[3];

	// orientation yaw, pitch, row
	// (rotation about X axis, rotation about Y axis, rotation about Z axis)
	double pose_orientation[3];

	//(x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
	double pose_cov[36];
};