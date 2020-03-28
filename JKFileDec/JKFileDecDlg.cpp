
// JKFileDecDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "JKFileDec.h"
#include "JKFileDecDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CJKFileDecDlg �Ի���

// Ψһ��Ӧ�ó������
#include <io.h>

//�ļ���ϵͳlib
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")

char PROC_PATH_C[512];
/********************************************************************
������������
�õ���������Ŀ¼
������˵����
������ֵ��
NULL��ʧ��  �������ɹ�
�������ߡ� WanHui
���޸��ߡ�
������޸�ʱ�䡿 
********************************************************************/
CHAR *JKCommon_GetProcPath_C()
{
    return PROC_PATH_C;//����
}
void JKCommon_WriteLog(char *strLogFile,UINT lineno, char *spc, char *logs)
{
    //�õ�ϵͳʱ��
    SYSTEMTIME st;//����
    ZeroMemory(&st,sizeof(SYSTEMTIME));//��ʼ��
    GetLocalTime(&st);//�õ���ǰʱ�� 

    //����ļ���
    char strLF[256];
    sprintf(strLF,"%sLogs\\\0",JKCommon_GetProcPath_C());//�������
    if(! PathFileExistsA(strLF))
        MakeSureDirectoryPathExists(strLF);//�½�Ŀ¼
    sprintf(strLF+strlen(strLF),"%s_%04d%02d%02d.log\0",strLogFile, st.wYear,st.wMonth,st.wDay);//�������

    char strTime[16];
    sprintf(strTime, "%02d:%02d:%02d.%03d \0", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    FILE *stream = fopen(strLF,"a+b");//���ļ�
    if (! stream)//�ж�
        return;

    char strLog[256];
    if(strlen(strTime) + 16 + strlen(spc) + strlen(logs) + 2 < sizeof(strLog))
    {
        sprintf(strLog,"%s Line[%d] %s%s\n\0",strTime, lineno, spc, logs);
        fwrite(strLog,1, strlen(strLog),stream);
    }
    else
    {
        fwrite(strTime,1, strlen(strTime),stream);
        fwrite(spc,1, strlen(spc),stream);
        fwrite(logs,1,strlen(logs),stream);//д����
        fwrite("\n",1,1,stream);//д����
    }
    fclose(stream);//�ر��ļ�
}

char * stristr (const char * str1, const char * str2)
{
    char *cp = (char *) str1;
    char *s1, *s2;

    if ( !*str2 )
        return((char *)str1);

    while (*cp)
    {
        s1 = cp;
        s2 = (char *) str2;
        while ( *s1 && *s2 && !(_tolower(*s1) - _tolower(*s2)) )
            s1++, s2++;
        if (!*s2)
            return(cp);
        cp++;
    }
    return(NULL);
}
//�Ӻ���ǰ��
char * strirstr (const char * str1, const char * str2)
{
    char *cp = (char *) str1;
    char *s1, *s2;

    if ( ! *str2 )
        return((char *)str1);

    char *prevpos = NULL;

    while (*cp)
    {
        s1 = cp;
        s2 = (char *) str2;

        while ( *s1 && *s2 && !(_tolower(*s1) - _tolower(*s2)) )
            s1++, s2++;
        if (! *s2)
            prevpos = cp;//��¼���һ��λ��
        //return(cp);
        cp++;
    }

    return(prevpos);
}

bool CJKFileDecDlg::CheckFilterList(char *filename)
{
    char *pData = NULL;
    for(UINT i = 0; i < m_FilterSum; i++)
    {
        pData = strirstr(filename, m_FilterList[i]);
        if(pData != NULL)//�ҵ�ǰ׺
        {
            if(strlen(pData) == strlen(m_FilterList[i]))//��ȫƥ��
                return true;
        }
    }

    return false;
}

bool JKCommon_BuildFolder(char *strPath)
{
    if(! PathFileExistsA(strPath))
        return MakeSureDirectoryPathExists(strPath) == TRUE;//�½����Ŀ¼(������'\\'Ϊβ�ַ�)

    return true;
}

UINT64 JKCommon_GetFileSize_Huge(char *strFileName)
{
    WIN32_FIND_DATAA fileinfo;//��ֵ
    DWORD64 dwSize = 0;//����

    HANDLE hFind = FindFirstFileA(strFileName, &fileinfo);//��ֵ
    if(hFind == INVALID_HANDLE_VALUE)//�ж�
        return 0;//����
    if(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//�ж�
    {
        FindClose(hFind);//�رվ��
        return 0;//����
    }

    dwSize = (fileinfo.nFileSizeHigh * 4294967296) + fileinfo.nFileSizeLow;//�ļ���С(DWORD-->4GB)
    FindClose(hFind);//�رվ��
    return dwSize;//����
}

bool JKCommon_GetClipboardText(char *filename,byte *pData, DWORD *pFileSize)
{
    //�򿪼�����
    if(! ::OpenClipboard(NULL)) // �򿪼�����
    {
        JKCommon_WriteLog("W",__LINE__,"���а��ʧ��>", filename);
        return false;
    }

    /*if (! IsClipboardFormatAvailable(CF_TEXT))
    {
        CloseClipboard();
        printf("���а����ݸ�ʽ����CF_TEXT\n");
        return false;
    }*/

    /*UINT  uFormat = EnumClipboardFormats(NULL);
    if (uFormat < 1)
    {   
    CloseClipboard();
    return false;
    }*/

    HANDLE hData = GetClipboardData(CF_TEXT);
    char * buffer = (char*)GlobalLock(hData);

    if(buffer == NULL)
    {
        CloseClipboard();
        JKCommon_WriteLog("W",__LINE__,"���а�Ϊ��>", filename);
        return false;
    }
    DWORD buffLen = GlobalSize(hData);
    
    if(*pFileSize < buffLen)
    {
        //GlobalUnlock(hData);
        //CloseClipboard();
        //printf("��%d����%d�� %s\n", *pFileSize, buffLen, filename);
        //return false;

        printf("��%d����%d�� %s\n", *pFileSize, buffLen, filename);
        memcpy(pData, buffer, *pFileSize);
        GlobalUnlock(hData);
        CloseClipboard();

        JKCommon_WriteLog("W",__LINE__,"�������ܳ���>", filename);
        return true;
    }
    else
    {
        //��������
        memcpy(pData, buffer, buffLen);
        *pFileSize = buffLen;
        GlobalUnlock(hData);
        CloseClipboard();
        return true;
    }
}

bool JKCommon_ReadFile(char *strFileName,DWORD *pFileSize,byte* pData)
{
    /*::OpenClipboard(NULL);
    ::EmptyClipboard();
    ::CloseClipboard();*/

    char sys[1024];
    sprintf(sys, "notepad.exe %s\0", strFileName);

    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi;

    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = TRUE;
    BOOL bRet = ::CreateProcess (
        NULL,        sys,
        NULL,        NULL,        FALSE,        CREATE_NEW_CONSOLE,
        NULL,        NULL,        &si,        &pi);

    if(! bRet )
    {
        JKCommon_WriteLog("W",__LINE__,"�½�����ʧ��>", strFileName);
        return false;
    }
    
    //ִ�а�������
    ::Sleep(800);

    //ctrl+a
    keybd_event(VK_CONTROL,0,0,0);//����Shift��
    keybd_event('A',0,0,0);//����a��
    keybd_event('A',0,KEYEVENTF_KEYUP,0);//�ɿ�a��
    keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);//�ɿ�Shift��
    //::Sleep(200);
    //����
    keybd_event(VK_CONTROL,0,0,0);//����Shift��
    keybd_event('C',0,0,0);//����a��
    keybd_event('C',0,KEYEVENTF_KEYUP,0);//�ɿ�a��
    keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);//�ɿ�Shift��

    ::Sleep(800);//�����а�ʱ��
    //�򿪼��а�
    if(! JKCommon_GetClipboardText(strFileName, pData, pFileSize))
    {
        system("TASKKILL /F /IM notepad.exe");
        return false;
    }

    system("TASKKILL /F /IM notepad.exe");

    return true;//����
}

bool JKCommon_WriteFile(char *strFileName,DWORD uFileSize,byte* pData)
{
    //���ļ������������½�
    HANDLE hFile = ::CreateFileA(strFileName, GENERIC_WRITE, 0, 
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)//�ж�
    {
        JKCommon_WriteLog("W",__LINE__,"д�ļ���ʧ��>", strFileName);
        return false;//����
    }

    DWORD nRet;//���ؽ�� 
    if(WriteFile(hFile,pData, uFileSize, &nRet,NULL) == FALSE)//дʧ��
    {
        ::CloseHandle(hFile);//�رվ�� 
        JKCommon_WriteLog("W",__LINE__,"д�ļ�ʧ��>", strFileName);
        return false;//����
    }

    ::CloseHandle(hFile);//�رվ�� 
    return true;//����
}

int JKCommon_Char2Num(char c)
{
    if(c >= '0' && c <= '9')
        return (int)(c - 0x30);
    else if(c >= 'a' && c <= 'f')
        return (int)(c - 0x61 + 10);
    else if(c >= 'A' && c <= 'F')
        return (int)(c - 0x41 + 10) ;
    else
        return 0;
}

int SuffixEncrpyt(char *srcfile, char *destfile, size_t dpSize)
{
    char *pSrcTail = strrchr(srcfile, '.');
    if(pSrcTail == NULL)
        return 0;

    char *pDestTail = strrchr(destfile, '.');
    if(pDestTail == NULL)
        return 0;

    char tmpStr[64];
    ZeroMemory(tmpStr, sizeof(tmpStr));
    pSrcTail++;//����
    if(strlen(pSrcTail) >= sizeof(tmpStr) /2)
    {
        //printf("error: ��׺������32���ַ�!!!\n");

        JKCommon_WriteLog("W",__LINE__,"��׺������>", srcfile);
        return 1;
    }

    for(int i = 0,j = 0; i < strlen(pSrcTail); i++, j += 2)
    {
        sprintf(tmpStr + j, "%02X", (byte *)*(pSrcTail + i));
    }

    pDestTail++;
    *pDestTail = '_';//ǰ׺
    pDestTail++;

    memcpy(pDestTail, tmpStr, strlen(tmpStr));

    return 0;
}

int SuffixDescrypt(char *srcfile, char *destfile)
{
    char *pSrcTail = strirstr(srcfile, "._");//�ҵ����һ��
    if(pSrcTail == NULL)
        return 2;

    char tmpStr[64];
    ZeroMemory(tmpStr, sizeof(tmpStr));
    pSrcTail += 2;//����

    if(strlen(pSrcTail) >= sizeof(tmpStr) /2)
    {
        JKCommon_WriteLog("W",__LINE__,"��׺������>", srcfile);
        return 1;
    }

    int bTmp = 0;
    for(int i = 0,j = 0; i < strlen(pSrcTail); i++, j += 2)
    {
        bTmp = (int)(JKCommon_Char2Num(*(pSrcTail + j))) * 16 + 
            (int)JKCommon_Char2Num(*(pSrcTail + j + 1));

        sprintf(tmpStr + i, "%c",  bTmp);
    }
    memcpy(destfile, srcfile, pSrcTail - srcfile - 1);//ȥ��'_'
    sprintf(destfile + strlen(destfile), "%s\0", tmpStr);
    return 0;
}


int CF_Self(char *srcfile, char *destfile)
{
    DWORD oldfilelen = JKCommon_GetFileSize_Huge(srcfile);
    if(oldfilelen < 1)
    {
        JKCommon_WriteLog("W",__LINE__,"�ļ�Ϊ��>", srcfile);
        return 1;
    }

    byte *fileData = (byte *)GlobalAlloc(GPTR, oldfilelen);
    if(fileData == NULL)
    {
        JKCommon_WriteLog("W",__LINE__,"����ռ�ʧ��>", srcfile);
        return 2;
    }

    if(! JKCommon_ReadFile(srcfile, &oldfilelen, fileData))
    {
        GlobalFree(fileData);
        //JKCommon_WriteLog("W",__LINE__,"���ļ�ʧ��>", srcfile);
        return 3;
    }

    if(! JKCommon_WriteFile(destfile, oldfilelen, fileData))
    {
        GlobalFree(fileData);
        //printf("д�ļ�ʧ�ܣ�%s\n", destfile);
        return 4;
    }

    GlobalFree(fileData);

    //�ж��ļ���С
    DWORD newfilelen = JKCommon_GetFileSize_Huge(destfile);
    if(newfilelen < 1 || newfilelen < oldfilelen)
    {
        JKCommon_WriteLog("W",__LINE__,"�ļ�Ϊ�ջ򳤶Ȳ��� > ", destfile);
        return 1;
    }
    
    if((newfilelen - oldfilelen) >= 5)
    {
        JKCommon_WriteLog("Q",__LINE__,"����ԭ�ļ�5Byte���� > ", destfile);
        return 0;
    }
    return 0;
}

int BuildFile(CJKFileDecDlg *pCJKFDD, int type, char *srcpath, char *destpath)
{
    char strLog[256];
    //�����ļ���
    BOOL bFound = FALSE;//��ֵ

    char tmpPath[512];//����
    ZeroMemory(tmpPath, sizeof(tmpPath));//��ʼ��
    sprintf(tmpPath,"%s*.*\0", srcpath);//�������

    char SrcSubPath[512];
    char DestSubPath[512];
    //---------�����ļ�-------------------//
    _finddatai64_t findData;//����
    intptr_t handle = _findfirsti64(tmpPath, &findData);// ����Ŀ¼�еĵ�һ���ļ�
    if (handle == -1)//�ж�
    {
        return -1;//����
    }

    do
    {
        //ֻ��һ��Ŀ¼
        if (findData.attrib & _A_SUBDIR)// �Ƿ�����Ŀ¼���Ҳ�Ϊ"."��".."
        {
            if(! strcmp(findData.name, ".") == 0 && ! strcmp(findData.name, "..") == 0)
            {
                ZeroMemory(SrcSubPath, sizeof(SrcSubPath));//��ʼ��
                sprintf(SrcSubPath, "%s%s\\\0", srcpath, findData.name);//�������

                if(type == 1)//����
                {
                    ZeroMemory(DestSubPath, sizeof(DestSubPath));//��ʼ��
                    sprintf(DestSubPath, "%s%s\\\0", destpath, findData.name);//�������
                    JKCommon_BuildFolder(DestSubPath);//�½��ļ���

                    if(BuildFile(pCJKFDD, type, SrcSubPath, DestSubPath) == -99)//�������ļ���
                        return -99;
                }
                else
                {
                    if(BuildFile(pCJKFDD, type, SrcSubPath, "") == -99)//�������ļ���
                        return -99;
                }
            }
        }
        else
        {
            //�����ļ�����ȡ��
            char srcfile[512];
            ZeroMemory(srcfile, sizeof(srcfile));
            sprintf(srcfile, "%s%s\0", srcpath, findData.name);

            if(type == 1)
            {
                char destfile[512];
                ZeroMemory(destfile, sizeof(destfile));
                sprintf(destfile, "%s%s\0", destpath, findData.name);

                if(pCJKFDD->CheckFilterList(srcfile))
                {
                    if(SuffixEncrpyt(srcfile, destfile, sizeof(destfile)) != 0)
                    {
                        sprintf(strLog, "��׺���쳣����ϸ����־[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    if(CF_Self(srcfile, destfile) != 0)
                    {
                        sprintf(strLog, " ���а渴���쳣����ϸ����־[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    pCJKFDD->m_sta_hint.SetWindowTextA(destfile);
                }
                else
                {
                    if(! CopyFile(srcfile, destfile, false))
                    {
                        sprintf(strLog, " �Ǽ��ܸ����쳣����ϸ����־[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }
                    else
                    {
                        //printf("�����ơ���%s\n",destfile);
                    }
                }
            }
            else
            {
                char destfile[512];
                ZeroMemory(destfile, sizeof(destfile));

                int iRet = SuffixDescrypt (srcfile, destfile);
                if(iRet == 1)//���ݴ���
                    return -99;
                else if(iRet == 0)
                {
                    if(rename(srcfile, destfile) != 0)
                    {
                        sprintf(strLog, " ������ʧ���쳣����ϸ����־[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    //printf("�����ܡ���%s\n",destfile);
                }
            }
        }
    } while (_findnexti64(handle, &findData) == 0);// ����Ŀ¼�е���һ���ļ�

    _findclose(handle); // �ر��������
    return 0;//����
}

void GetSelPath(char *path, size_t size)
{
    /*BROWSEINFO bi;  
    ZeroMemory(&bi, sizeof(BROWSEINFO));
    bi.hwndOwner = NULL;  
    bi.ulFlags   = BIF_RETURNONLYFSDIRS;
    LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
    if (pidl)
    {
    if (SHGetPathFromIDList(pidl, path))
    {
    strncat(path, "\\",1 );
    }

    IMalloc *pMalloc = NULL;  
    if (SUCCEEDED(SHGetMalloc(&pMalloc)) && pMalloc)  
    {
    pMalloc->Free(pidl);
    pMalloc->Release();
    }  
    }  */


    CFolderPickerDialog fd(NULL, 0, NULL, 0);
    if (fd.DoModal() == IDOK)
    {
        CString des = fd.GetPathName();
        char *pData = des.GetBuffer(size);;
        memcpy(path, pData, strlen(pData));
        des.ReleaseBuffer();
    }
}


CJKFileDecDlg::CJKFileDecDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CJKFileDecDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CJKFileDecDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_EDT_OLDPATH, m_edt_oldpath);
    DDX_Control(pDX, IDC_EDT_NEWPATH, m_edt_newpath);
    DDX_Control(pDX, IDC_BTN_OLDPATH, m_btn_oldpath);
    DDX_Control(pDX, IDC_BTN_NEWPATH, m_btn_newpath);
    DDX_Control(pDX, IDC_CHK_ADDSUBPATH, m_chk_addsubpath);
    DDX_Control(pDX, IDC_STA_HINT, m_sta_hint);
    DDX_Control(pDX, IDC_PRO, m_pro);
    DDX_Control(pDX, IDC_BTN_RUNDES, m_btn_rundes);
    DDX_Control(pDX, IDC_BTN_STOPDES, m_btn_stopdes);
    DDX_Control(pDX, IDC_BTN_RESTORE, m_btn_restore);
    DDX_Control(pDX, IDC_CHK_SUFFIX_CPP, m_chk_suffix_cpp);
    DDX_Control(pDX, IDC_CHK_SUFFIX_C, m_chk_suffix_c);
    DDX_Control(pDX, IDC_CHK_SUFFIX_H, m_chk_suffix_h);
    DDX_Control(pDX, IDC_CHK_SUFFIX_CS, m_chk_suffix_cs);
    DDX_Control(pDX, IDC_CHK_SUFFIX_JAVA, m_chk_suffix_java);
    DDX_Control(pDX, IDC_CHK_SUFFIX_ALL, m_chk_suffix_all);
    DDX_Control(pDX, IDC_CHK_SUFFIX_HPP, m_chk_suffix_hpp);
    DDX_Control(pDX, IDC_CHK_SUFFIX_CC, m_chk_suffix_cc);
}

BEGIN_MESSAGE_MAP(CJKFileDecDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_BTN_OLDPATH, &CJKFileDecDlg::OnBnClickedBtnOldpath)
    ON_BN_CLICKED(IDC_BTN_NEWPATH, &CJKFileDecDlg::OnBnClickedBtnNewpath)
    ON_BN_CLICKED(IDC_CHK_ADDSUBPATH, &CJKFileDecDlg::OnBnClickedChkAddsubpath)
    ON_BN_CLICKED(IDC_BTN_RUNDES, &CJKFileDecDlg::OnBnClickedBtnRundes)
    ON_BN_CLICKED(IDC_BTN_RESTORE, &CJKFileDecDlg::OnBnClickedBtnRestore)
    ON_BN_CLICKED(IDC_CHK_SUFFIX_ALL, &CJKFileDecDlg::OnBnClickedChkSuffixAll)
END_MESSAGE_MAP()


// CJKFileDecDlg ��Ϣ�������

BOOL CJKFileDecDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
    ZeroMemory(m_FilterList,sizeof(m_FilterList));//���
    m_FilterSum = 0;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CJKFileDecDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CJKFileDecDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


UINT WINAPI ThdDes(LPVOID lpParam)
{
    CJKFileDecDlg *pCJKFDD = (CJKFileDecDlg *)lpParam;

    pCJKFDD->m_btn_rundes.EnableWindow(TRUE);
    pCJKFDD->m_btn_stopdes.EnableWindow(FALSE);


    return 0;
}


void CJKFileDecDlg::OnBnClickedBtnOldpath()
{
    char path[MAX_PATH];
    ZeroMemory(path, sizeof(path));
    GetSelPath(path, sizeof(path));
    m_edt_oldpath.SetWindowTextA(path);
}

void CJKFileDecDlg::OnBnClickedBtnNewpath()
{
    char path[MAX_PATH];
    ZeroMemory(path, sizeof(path));
    GetSelPath(path, sizeof(path));
    m_edt_newpath.SetWindowTextA(path);
}


void CJKFileDecDlg::OnBnClickedChkAddsubpath()
{
    
}


void CJKFileDecDlg::OnBnClickedBtnRundes()
{
    //���Ӻ�������

    if(m_chk_suffix_cpp.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".cpp", 4);
        m_FilterSum++;
    }

    if(m_chk_suffix_c.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".c", 2);
        m_FilterSum++;
    }

    if(m_chk_suffix_h.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".h", 2);
        m_FilterSum++;
    }

    if(m_chk_suffix_hpp.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".hpp", 4);
        m_FilterSum++;
    }

    if(m_chk_suffix_cc.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".cc", 3);
        m_FilterSum++;
    }

    if(m_chk_suffix_cs.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".cs", 3);
        m_FilterSum++;
    }

    if(m_chk_suffix_java.GetCheck() == BST_CHECKED)
    {
        memcpy(m_FilterList[m_FilterSum], ".java", 5);
        m_FilterSum++;
    }


    //�����߳�
    ZeroMemory(m_OldPath, sizeof(m_OldPath));
    m_edt_oldpath.GetWindowTextA(m_OldPath, sizeof(m_OldPath));

    ZeroMemory(m_NewPath, sizeof(m_NewPath));
    m_edt_newpath.GetWindowTextA(m_NewPath, sizeof(m_NewPath));

    char subpath[MAX_PATH];
    ZeroMemory(subpath, sizeof(subpath));

    if(strlen(m_OldPath) < 1 || strlen(m_NewPath) < 1)
    {
        MessageBoxA("����Ŀ¼������Ŀ¼������Ϊ��", "����", MB_OK | MB_ICONERROR);
        return;
    }

    if(m_OldPath[strlen(m_OldPath)] != '\\')
        strncat(m_OldPath, "\\", 1);
    if(m_NewPath[strlen(m_NewPath)] != '\\')
        strncat(m_NewPath, "\\", 1);

    if(m_chk_addsubpath.GetCheck() == BST_CHECKED)
    {
        //�ж������Ŀ¼�Ƿ���ͬ
        char *pOldEnd = NULL;
        char *pNewEnd = NULL;

        pOldEnd = strrchr(m_OldPath, '\\');//�ҵ����һ��
        pNewEnd = strrchr(m_NewPath, '\\');//�ҵ����һ��
        
        if(pOldEnd != NULL && pNewEnd != NULL)
        {
            char *pOldPrev = NULL;
            char *pNewPrev = NULL;

            *pOldEnd = '|';
            *pNewEnd = '|';

            pOldPrev = strrchr(m_OldPath, '\\');//�ҵ����һ��
            pNewPrev = strrchr(m_NewPath, '\\');//�ҵ����һ��

            *pOldEnd = '\\';
            *pNewEnd = '\\';

            if(pOldPrev != NULL && pNewPrev != NULL)
            {
                if(strcmp(pOldPrev, pNewPrev))//���ݲ�ͬ
                {
                    memcpy(subpath, pOldPrev + 1, pOldEnd - pOldPrev);//�õ�����
                }
                else
                {
                    if(IDYES != MessageBoxA("���һ����Ŀ¼��ͬ������������ԭ���ݣ��Ƿ����","��ʾ",
                        MB_YESNO | MB_ICONQUESTION))
                    {
                        return;
                    }
                }
            }
        }
    }

    if(strlen(subpath) > 0)
    {
        strncat(m_NewPath, subpath, strlen(subpath));
    }

    int ret = BuildFile(this, 1, this->m_OldPath, this->m_NewPath);
    if(ret != 0)
    {
        MessageBoxA("���������ܡ�ʧ��", "����", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxA("���������ܡ��ɹ�", "����", MB_OK | MB_ICONINFORMATION);
    }

    ////�����߳�
    //HANDLE hnd = (HANDLE)_beginthreadex(NULL,0,ThdDes, (void *)this,CREATE_SUSPENDED,NULL);

    //if(hnd != NULL)
    //{
    //    m_ThdFlag_Des = 1;//�����ɹ�

    //    m_btn_rundes.EnableWindow(FALSE);
    //    m_btn_stopdes.EnableWindow(TRUE);

    //    ResumeThread(hnd);//�����߳�
    //    CloseHandle(hnd);
    //}
    //else
    //{
    //    m_btn_rundes.EnableWindow(TRUE);
    //    m_btn_stopdes.EnableWindow(FALSE);
    //    m_sta_hint.SetWindowTextA("�����߳�����ʧ��");
    //    return;
    //}
}


void CJKFileDecDlg::OnBnClickedBtnRestore()
{
    ZeroMemory(m_NewPath, sizeof(m_NewPath));
    m_edt_newpath.GetWindowTextA(m_NewPath, sizeof(m_NewPath));

    int ret = BuildFile(this, 0, this->m_NewPath,"");
    if(ret != 0)
    {
        MessageBoxA("�����ܻ�ԭ��ʧ��", "����", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxA("�����ܻ�ԭ���ɹ�", "����", MB_OK | MB_ICONINFORMATION);
    }
}

void CJKFileDecDlg::OnBnClickedChkSuffixAll()
{
    int flag = m_chk_suffix_all.GetCheck();
    m_chk_suffix_cpp.SetCheck(flag);
    m_chk_suffix_c.SetCheck(flag);
    m_chk_suffix_h.SetCheck(flag);
    m_chk_suffix_cs.SetCheck(flag);
    m_chk_suffix_java.SetCheck(flag);
    m_chk_suffix_cc.SetCheck(flag);
    m_chk_suffix_hpp.SetCheck(flag);

}
 