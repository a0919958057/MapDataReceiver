#pragma once
#include "afxsock.h"
#include "../server/include/PoseDataStruct.h"

class CPSocket :
	public CSocket {
public:
	CPSocket();
	virtual ~CPSocket();
	virtual void OnReceive(int nErrorCode);
	virtual BOOL OnMessagePending();
	void registerParent(CWnd* _parent);
	PoseData m_pose_data;
	CWnd* m_parent;
};