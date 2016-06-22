
// SensorDataReceiverDlg.h : ���Y��
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include <afxsock.h>
#include "MSocket.h"
#include "CRLSocket.h"
#include "RSocket.h"


// CSensorDataReceiverDlg ��ܤ��
class CMapDataReceiverDlg : public CDialogEx
{
// �غc
public:
	CMapDataReceiverDlg(CWnd* pParent = NULL);	// �зǫغc�禡

// ��ܤ�����
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SENSORDATARECEIVER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV �䴩


// �{���X��@
protected:
	HICON m_hIcon;


	enum TCPEvent {
		CREATE_SOCKET_SUCCESSFUL,
		CREATE_SOCKET_FAIL,
		CONNECT_SUCCESSFUL,
		CONNECT_FAIL,
		DISCONNECT,
		SEND_MESSAGE_SUCCESSFUL,
		SENT_MESSAGE_FAIL
	};
	// ���ͪ��T�������禡
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	CIPAddressCtrl m_socket_ip_c;
	CListBox m_socket_log_c;
	CButton m_socket_connect_c;
	UINT m_socket_map_port;
	UINT m_socket_lrf_port;
	template<size_t LENGTH> void SendSocketMessage(char(&data)[LENGTH]);

public:
	afx_msg void OnBnClickedSocketConnect();
	void DoMapSocketConnect();
	void DoLRFSocketConnect();
private:
	void DoMapSocketDisconnect();
	void DoLRFSocketDisconnect();
	void CMapDataReceiverDlg::ReportSocketStatus(TCPEvent event_, CString &msg = CString(""));
	bool fg_map_connected;
	bool fg_lrf_connected;
	bool fg_cmd_connected;
protected:
	CMSocket m_socket_map;
	CRSocket m_socket_lrf;
	CCRLSocket m_socket_cmd;
public:
	CListBox m_sensor_data_c;
	CString m_pos_data;
	afx_msg void OnBnClickedSaveMap();

	afx_msg void OnBnClickedGetMapList();
	afx_msg void OnBnClickedSelMap();
};
