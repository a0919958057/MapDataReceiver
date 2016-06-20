
// SensorDataReceiverDlg.cpp : 實作檔
//

#include "stdafx.h"
#include "MapDataReceiver.h"
#include "MapDataReceiverDlg.h"
#include "afxdialogex.h"

// CSocket 的標頭檔
#include "MSocket.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 對 App About 使用 CAboutDlg 對話方塊

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 對話方塊資料
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支援

// 程式碼實作
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CSensorDataReceiverDlg 對話方塊



CMapDataReceiverDlg::CMapDataReceiverDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_SENSORDATARECEIVER_DIALOG, pParent)
	, fg_connected(false), m_socket_port(0), m_pos_data(_T("")) {
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMapDataReceiverDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SOCKET_IP, m_socket_ip_c);
	DDX_Control(pDX, IDC_SOCKET_LOG, m_socket_log_c);
	DDX_Control(pDX, IDC_SOCKET_CONNECT, m_socket_connect_c);
	DDX_Control(pDX, IDC_SENSOR_DATA, m_sensor_data_c);
	DDX_Text(pDX, IDC_POS_DATA, m_pos_data);
	DDX_Text(pDX, IDC_SOCKET_PORT, m_socket_port);
}

BEGIN_MESSAGE_MAP(CMapDataReceiverDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_SOCKET_CONNECT, &CMapDataReceiverDlg::OnBnClickedSocketConnect)
	ON_BN_CLICKED(IDC_SAVE_MAP, &CMapDataReceiverDlg::OnBnClickedSaveMap)
END_MESSAGE_MAP()


// CSensorDataReceiverDlg 訊息處理常式

BOOL CMapDataReceiverDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 將 [關於...] 功能表加入系統功能表。

	// IDM_ABOUTBOX 必須在系統命令範圍之中。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 設定此對話方塊的圖示。當應用程式的主視窗不是對話方塊時，
	// 框架會自動從事此作業
	SetIcon(m_hIcon, TRUE);			// 設定大圖示
	SetIcon(m_hIcon, FALSE);		// 設定小圖示

	// TODO: 在此加入額外的初始設定
	AfxSocketInit();
	m_socket.registerParent(this);
	m_socket_ip_c.SetAddress(127, 0, 0, 1);
	m_socket_port = 25651;

	for (int i = 0; i < SIZE_MAP; i += SIZE_MAP / 20) {
		m_sensor_data_c.AddString(CString(""));
	}
	UpdateData(false);

	return TRUE;  // 傳回 TRUE，除非您對控制項設定焦點
}

void CMapDataReceiverDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果將最小化按鈕加入您的對話方塊，您需要下列的程式碼，
// 以便繪製圖示。對於使用文件/檢視模式的 MFC 應用程式，
// 框架會自動完成此作業。

void CMapDataReceiverDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 繪製的裝置內容

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 將圖示置中於用戶端矩形
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 描繪圖示
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 當使用者拖曳最小化視窗時，
// 系統呼叫這個功能取得游標顯示。
HCURSOR CMapDataReceiverDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CMapDataReceiverDlg::OnBnClickedSocketConnect() {

	if (fg_connected) {
		DoSocketDisconnect();
	} else {
		DoSocketConnect();
	}
	m_socket_connect_c.SetCheck(fg_connected);


}



void CMapDataReceiverDlg::DoSocketConnect() {

	// Sync the Panel data to the model
	UpdateData(true);

	// Read the TCP/IP address setting from User
	byte aIpAddressUnit[4];
	m_socket_ip_c.GetAddress(
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);
	CString aStrIpAddress;
	aStrIpAddress.Format(_T("%d.%d.%d.%d"),
		aIpAddressUnit[0], aIpAddressUnit[1], aIpAddressUnit[2], aIpAddressUnit[3]);

	// Create a TCP Socket for transfer Camera data
	// m_tcp_socket.Create(m_tcp_ip_port, 1, aStrIpAddress);
	if (!m_socket.Create()) {

		// If Socket Create Fail Report Message
		TCHAR szMsg[1024] = { 0 };
		wsprintf(szMsg, _T("create faild: %d"), m_socket.GetLastError());
		ReportSocketStatus(TCPEvent::CREATE_SOCKET_FAIL);
		AfxMessageBox(szMsg);
	} else {

		ReportSocketStatus(TCPEvent::CREATE_SOCKET_SUCCESSFUL);
		// Connect to the Server ( Raspberry Pi Server )
		fg_connected = m_socket.Connect(aStrIpAddress, m_socket_port);

		//fg_tcp_ip_read = true;
		//m_tcp_ip_IOHandle_thread = AfxBeginThread(TcpIODataHandler, LPVOID(this));

		//For Test
		m_socket.Send("Test from here", 14);
	}

	if (fg_connected)
		ReportSocketStatus(TCPEvent::CONNECT_SUCCESSFUL, aStrIpAddress);
	else {
		ReportSocketStatus(TCPEvent::CONNECT_FAIL, aStrIpAddress);
		m_socket.Close();
	}



}


void CMapDataReceiverDlg::DoSocketDisconnect() {
	// Setup Connect-staus flag
	fg_connected = false;
	//fg_tcp_ip_read = false;

	// Close the TCP/IP Socket
	m_socket.Close();

	// Report TCP/IP connect status
	CString tmp_log; tmp_log.Format(_T("I/O event: %s"), _T("Close Socket"));
	m_socket_log_c.AddString(tmp_log);
}

void CMapDataReceiverDlg::ReportSocketStatus(TCPEvent event_, CString &msg) {
	CString tmp_log;
	switch (event_) {
		case CREATE_SOCKET_SUCCESSFUL:
			tmp_log.Format(_T("I/O event: %s"),
				_T("Create Socket Successful"));
			break;
		case CREATE_SOCKET_FAIL:
			tmp_log.Format(_T("I/O event: %s"),
				_T("Create Socket Fail"));
			break;
		case CONNECT_SUCCESSFUL:
			tmp_log.Format(_T("I/O event: %s%s%s"),
				_T("Connect "), msg, _T(" Successful"));
			break;
		case CONNECT_FAIL:
			tmp_log.Format(_T("I/O event: %s%s%s"),
				_T("Connect "), msg, _T(" Fail"));
			break;
		case DISCONNECT:
			tmp_log.Format(_T("I/O event: %s"),
				_T("Disconnect"));
			break;
		case SEND_MESSAGE_SUCCESSFUL:
			tmp_log.Format(_T("I/O event: %s%s%s"),
				_T("Sent Message"), msg, _T("Successful"));
			break;
		case SENT_MESSAGE_FAIL:
			tmp_log.Format(_T("I/O event: %s%s%s"),
				_T("Sent Message"), msg, _T("Fail"));
			break;
	}
	m_socket_log_c.AddString(tmp_log);
}

template<size_t LENGTH>
void CMapDataReceiverDlg::SendSocketMessage(char(&data)[LENGTH]) {

	// Send Message
	if (fg_tcp_ip_connected) {
		m_tcp_socket.Send(data, LENGTH);
		ReportTCPStatus(TCPEvent::SEND_MESSAGE_SUCCESSFUL, CString(data));
	}
}


void CMapDataReceiverDlg::OnBnClickedSaveMap() {
	// TODO: 在此加入控制項告知處理常式程式碼
	CString filter;
	filter = "所有文件(*.bmp,*.jpg,*.gif,*tiff)|*.bmp;*.jpg;*.gif;*.tiff| BMP(*.bmp)|*.bmp| JPG(*.jpg)|*.jpg| GIF(*.gif)|*.gif| TIFF(*.tiff)|*.tiff||";
	CFileDialog fdlg(FALSE, NULL, NULL, OFN_HIDEREADONLY, filter, NULL);

	if (fdlg.DoModal() == IDOK) {
	}
}
