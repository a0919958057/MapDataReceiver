
// SensorDataReceiver.h : PROJECT_NAME ���ε{�����D�n���Y��
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�� PCH �]�t���ɮ׫e���]�t 'stdafx.h'"
#endif

#include "resource.h"		// �D�n�Ÿ�


// CSensorDataReceiverApp: 
// �аѾ\��@�����O�� SensorDataReceiver.cpp
//

class CSensorDataReceiverApp : public CWinApp
{
public:
	CSensorDataReceiverApp();

// �мg
public:
	virtual BOOL InitInstance();

// �{���X��@

	DECLARE_MESSAGE_MAP()
};

extern CSensorDataReceiverApp theApp;