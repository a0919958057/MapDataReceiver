#pragma once

#define REPORT_DATA_SIZE 10

// The Controll function for Map Provider
enum ControlMapMsg {
	SET_INIT_POSE,
	GET_MAP_LIST,
	GET_MAP,
	SEL_MAP,
	LOAD_MAP
};

// The Controll function for Laser Finder
enum ControlLRFMsg {
	START_LRF_STREAM,
	STOP_LRF_STREAM
};

// The main communication package
struct ControlMsg {
	ControlMapMsg map_msg;
	ControlLRFMsg lrf_msg;
	// position X, Y, Z
	// (x, y, z)
	double init_pose_position[3];

	// orientation yaw, pitch, row
	// (rotation about X axis, rotation about Y axis, rotation about Z axis)
	double init_pose_orientation[3];

	//(x, y, z, rotation about X axis, rotation about Y axis, rotation about Z axis)
	double init_pose_cov[36];

	int data[REPORT_DATA_SIZE];
};
