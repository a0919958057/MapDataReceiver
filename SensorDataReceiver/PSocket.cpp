#include "stdafx.h"
#include "PSocket.h"
#include "MapDataReceiverDlg.h"

CPSocket::CPSocket() {

	memset(&m_pose_data, 0, sizeof(PoseData));
}


CPSocket::~CPSocket() {}

void CPSocket::OnReceive(int nErrorCode) {
	static int counter(0);
	counter++;

	if (0 == nErrorCode) {

		static int i = 0;
		i++;

		int nRead(0);

		volatile int cbLeft(sizeof(PoseData)); // 4 Byte
		volatile int cbDataReceived(0);
		int cTimesRead(0);
		do {


			// Determine Socket State
			nRead = Receive(&m_pose_data + cbDataReceived, cbLeft);

			if (nRead == SOCKET_ERROR) {
				if (GetLastError() != WSAEWOULDBLOCK) {
					//AfxMessageBox(_T("Error occurred"));
					Close();
					// Trying to reconnect
					((CMapDataReceiverDlg*)m_parent)->DoPoseSocketConnect();
					return;
				}
				break;
			}


			cbLeft -= nRead;
			cbDataReceived += nRead;
			cTimesRead++;
		} while (cbLeft > 0 && cTimesRead < 50);
	}
	CSocket::OnReceive(nErrorCode);
}

BOOL CPSocket::OnMessagePending() {


	return 0;
}

void CPSocket::registerParent(CWnd* _parent) {
	m_parent = _parent;
}