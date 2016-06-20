#include "stdafx.h"
#include "MSocket.h"
#include "MapDataReceiverDlg.h"
#include "resource.h"


CMSocket::CMSocket() {
	m_map_data = new MapData;
}


CMSocket::~CMSocket() {
	delete m_map_data;
}

void CMSocket::OnReceive(int nErrorCode) {

	static unsigned int counter(0);

	if (0 == nErrorCode) {
		static int i = 0;
		i++;

		int nRead(0);

		volatile int cbLeft(sizeof(MapData)); // 4 Byte
		volatile int cbDataReceived(0);
		int cTimesRead(0);
		do {

			if(cbLeft < 0) AfxMessageBox(_T("WTF occurred"));
			// Determine Socket State
			nRead = Receive(m_map_data + cbDataReceived, cbLeft);

			/****** Print Socket status inorder to debug ****/
			CString msg;
			msg.Format(_T("Rec count: %d, Left count: %d"), nRead, cbLeft);
			m_parent->SetDlgItemTextW(IDC_RECE_STATUS, msg);
			m_parent->UpdateData(false);
			/**********************************************/
			
			if (nRead == SOCKET_ERROR) {
				if (GetLastError() != WSAEWOULDBLOCK) {
					AfxMessageBox(_T("Error occurred : %d"));
					Close();
				}
				break;
			}

			cbLeft -= nRead;
			cbDataReceived += nRead;
			cTimesRead++;
		} while (cbLeft > 0 && cTimesRead < 50);

	}



	
	CListBox* aListBox = (CListBox*)m_parent->GetDlgItem(IDC_SENSOR_DATA);
	aListBox->SetRedraw(false);
	aListBox->ResetContent();
	char buffer[100];

	sprintf_s(buffer, 50, "Resolution: %5.2f",
		m_map_data->info.res);
	aListBox->AddString(CString(buffer));

	sprintf_s(buffer, 50, "Map Height: %4d",
		m_map_data->info.height);
	aListBox->AddString(CString(buffer));

	sprintf_s(buffer, 50, "Map Width: %4d",
		m_map_data->info.width);
	aListBox->AddString(CString(buffer));
	sprintf_s(buffer, 50, "Origin : %5.2f, %5.2f",
		m_map_data->info.origin_x, m_map_data->info.origin_y);
	aListBox->AddString(CString(buffer));
	

	sprintf_s(buffer, 50, "Rotation: %5.2f",
		m_map_data->info.origin_yaw);
	aListBox->AddString(CString(buffer));


	aListBox->SetRedraw(true);
	
	
	// ***********************************
	// Show the Map data to User
	// ***********************************
	/**
	CDC *pDC = m_parent->GetDlgItem(IDC_POS_SHOW)->GetDC();

	CStatic* pPic = (CStatic*)m_parent->GetDlgItem(IDC_POS_SHOW);

	// Transform the raw data to BITMAT format
	BITMAP bitmap;
	HBITMAP hMap;
	hMap = CreateBitmap(MAP_SIZE_X, MAP_SIZE_X, 1,
		sizeof(UINT8), &m_map_data.data);


	// Print the map data to the screen
	pPic->ModifyStyle(0xf, SS_BITMAP | SS_CENTERIMAGE);
	pPic->SetBitmap(hMap);
	*/

	CSocket::OnReceive(nErrorCode);
}

BOOL CMSocket::OnMessagePending() {
	

	return 0;
}


void CMSocket::registerParent(CWnd* _parent) {
	m_parent = _parent;
}
