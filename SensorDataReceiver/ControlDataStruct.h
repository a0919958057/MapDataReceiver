#pragma once

#define REPORT_DATA_SIZE 10

// The Controll function for Map Provider
enum ControlMapMsg {
	GET_MAP_LIST,
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
	int data[REPORT_DATA_SIZE];
};
