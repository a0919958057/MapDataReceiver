#pragma once
#include "afxsock.h"
#include <sys/types.h>

#define MAP_SIZE_X (400)
#define MAP_SIZE_Y (400)

#define SIZE_MAP ( MAP_SIZE_X * MAP_SIZE_Y )

struct MapMetaInfo {
	float res;
	unsigned int height;
	unsigned int width;
	float origin_x;
	float origin_y;
	float origin_yaw;
};

struct MapData {
	struct MapMetaInfo info;
	UINT8 data[SIZE_MAP];
};

class CMSocket :
	public CSocket {
public:
	CMSocket();
	virtual ~CMSocket();
	virtual void OnReceive(int nErrorCode);
	virtual BOOL OnMessagePending();
	struct MapData* m_map_data;
	void registerParent(CWnd* _parent);

	CWnd* m_parent;
};

