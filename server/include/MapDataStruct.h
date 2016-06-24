#pragma once
#include<inttypes.h>
#include <cmath>

// Define the map size
#define MAP_SIZE_X (576)
#define MAP_SIZE_Y (576)
#define SIZE_MAP ( MAP_SIZE_X * MAP_SIZE_Y )

// Define the atom data size  (bytes)
#define MAP_PER_CUT_SIZE (400)

#define MAP_PART_COUNT ((int)(std::ceil(SIZE_MAP/MAP_PER_CUT_SIZE)))

/********* 分批傳輸時的Meta資訊，用於辨識這是第幾個物件，組裝時使用 ******/
struct MapMetaInfoPart {
	int map_stamp;
	int part_stamp;
	float res;
	unsigned int height;
	unsigned int width;
	float origin_x;
	float origin_y;
	float origin_yaw;
};

struct MapDataPart {
	struct MapMetaInfoPart info;
	uint8_t data[MAP_PER_CUT_SIZE];
};
/****************************************************************/

/*******************放置已經組合好的資料*********************/
struct MapMetaInfo {
	int map_stamp;
	float res;
	unsigned int height;
	unsigned int width;
	float origin_x;
	float origin_y;
	float origin_yaw;
};

struct MapData {
	struct MapMetaInfo info;
	uint8_t data[SIZE_MAP];
};
/****************************************************************/
