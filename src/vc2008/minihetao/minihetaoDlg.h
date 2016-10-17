
// minihetaoDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"


// CMiniHetaoDlg �Ի���
class CMiniHetaoDlg : public CDialog
{
// ����
public:
	CMiniHetaoDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_MINIHETAO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();

	afx_msg LRESULT OnClickPopupMenu( WPARAM wParam , LPARAM lParam );
	afx_msg void OnShowWindow();
	afx_msg void OnShowAboutBox();

	DECLARE_MESSAGE_MAP()
public:
	NOTIFYICONDATA m_nid;
	CString m_strWWWRoot;

	afx_msg void OnDestroy();

	afx_msg void OnBnClickedButtonSelectDirectory();
	afx_msg void OnBnClickedButtonRunning();
	afx_msg void OnBnClickedButtonStop();
	afx_msg void OnBnClickedButtonHide();
	afx_msg void OnBnClickedButtonExit();
	HANDLE m_hRunningThread;
	DWORD m_dwThreadId;
	CButton m_ctlRunningButton;
	CButton m_ctlStopButton;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CButton m_ctlRegisteFolderPopupmenu;
	CButton m_ctlUnregisteFolderPopupmenu;
	afx_msg void OnBnClickedButtonRegiste();
	afx_msg void OnBnClickedButtonUnregiste();
	CEdit m_ctlWWWRoot;
	CString m_strStaticRemark;
};
