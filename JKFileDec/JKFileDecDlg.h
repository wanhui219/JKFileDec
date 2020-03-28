
// JKFileDecDlg.h : ͷ�ļ�
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"

#define GFL_MAX 64
// CJKFileDecDlg �Ի���
class CJKFileDecDlg : public CDialogEx
{
// ����
public:
	CJKFileDecDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_JKFILEDEC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
    int m_ThdFlag_Des;
    char m_OldPath[MAX_PATH];
    char m_NewPath[MAX_PATH];
    
    char m_FilterList[GFL_MAX][16];
    UINT m_FilterSum;//����

    CEdit m_edt_oldpath;
    CEdit m_edt_newpath;
    CButton m_btn_oldpath;
    CButton m_btn_newpath;
    CButton m_chk_addsubpath;
    CStatic m_sta_hint;
    CProgressCtrl m_pro;
    CButton m_btn_rundes;
    CButton m_btn_stopdes;
    CButton m_btn_restore;
    afx_msg void OnBnClickedBtnOldpath();
    afx_msg void OnBnClickedBtnNewpath();
    afx_msg void OnBnClickedChkAddsubpath();
    afx_msg void OnBnClickedBtnRundes();
    afx_msg void OnBnClickedBtnRestore();
    CButton m_chk_suffix_cpp;
    CButton m_chk_suffix_c;
    CButton m_chk_suffix_h;
    CButton m_chk_suffix_cs;
    CButton m_chk_suffix_java;
    CButton m_chk_suffix_all;
    afx_msg void OnBnClickedChkSuffixAll();


    bool CheckFilterList(char *filename);
    CButton m_chk_suffix_hpp;
    CButton m_chk_suffix_cc;
};
