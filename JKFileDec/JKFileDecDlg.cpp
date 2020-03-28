
// JKFileDecDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "JKFileDec.h"
#include "JKFileDecDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CJKFileDecDlg 对话框

// 唯一的应用程序对象
#include <io.h>

//文件夹系统lib
#include <imagehlp.h>
#pragma comment(lib, "imagehlp.lib")

char PROC_PATH_C[512];
/********************************************************************
【功能描述】
得到程序运行目录
【参数说明】
【返回值】
NULL：失败  其它：成功
【开发者】 WanHui
【修改者】
【最后修改时间】 
********************************************************************/
CHAR *JKCommon_GetProcPath_C()
{
    return PROC_PATH_C;//返回
}
void JKCommon_WriteLog(char *strLogFile,UINT lineno, char *spc, char *logs)
{
    //得到系统时间
    SYSTEMTIME st;//变量
    ZeroMemory(&st,sizeof(SYSTEMTIME));//初始化
    GetLocalTime(&st);//得到当前时间 

    //组合文件名
    char strLF[256];
    sprintf(strLF,"%sLogs\\\0",JKCommon_GetProcPath_C());//组合数据
    if(! PathFileExistsA(strLF))
        MakeSureDirectoryPathExists(strLF);//新建目录
    sprintf(strLF+strlen(strLF),"%s_%04d%02d%02d.log\0",strLogFile, st.wYear,st.wMonth,st.wDay);//组合数据

    char strTime[16];
    sprintf(strTime, "%02d:%02d:%02d.%03d \0", st.wHour, st.wMinute, st.wSecond, st.wMilliseconds);

    FILE *stream = fopen(strLF,"a+b");//打开文件
    if (! stream)//判断
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
        fwrite(logs,1,strlen(logs),stream);//写数据
        fwrite("\n",1,1,stream);//写数据
    }
    fclose(stream);//关闭文件
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
//从后往前找
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
            prevpos = cp;//记录最后一个位置
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
        if(pData != NULL)//找到前缀
        {
            if(strlen(pData) == strlen(m_FilterList[i]))//完全匹配
                return true;
        }
    }

    return false;
}

bool JKCommon_BuildFolder(char *strPath)
{
    if(! PathFileExistsA(strPath))
        return MakeSureDirectoryPathExists(strPath) == TRUE;//新建多层目录(必须以'\\'为尾字符)

    return true;
}

UINT64 JKCommon_GetFileSize_Huge(char *strFileName)
{
    WIN32_FIND_DATAA fileinfo;//赋值
    DWORD64 dwSize = 0;//返回

    HANDLE hFind = FindFirstFileA(strFileName, &fileinfo);//赋值
    if(hFind == INVALID_HANDLE_VALUE)//判断
        return 0;//返回
    if(fileinfo.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)//判断
    {
        FindClose(hFind);//关闭句柄
        return 0;//返回
    }

    dwSize = (fileinfo.nFileSizeHigh * 4294967296) + fileinfo.nFileSizeLow;//文件大小(DWORD-->4GB)
    FindClose(hFind);//关闭句柄
    return dwSize;//返回
}

bool JKCommon_GetClipboardText(char *filename,byte *pData, DWORD *pFileSize)
{
    //打开剪贴板
    if(! ::OpenClipboard(NULL)) // 打开剪贴板
    {
        JKCommon_WriteLog("W",__LINE__,"剪切板打开失败>", filename);
        return false;
    }

    /*if (! IsClipboardFormatAvailable(CF_TEXT))
    {
        CloseClipboard();
        printf("剪切板数据格式不是CF_TEXT\n");
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
        JKCommon_WriteLog("W",__LINE__,"剪切板为空>", filename);
        return false;
    }
    DWORD buffLen = GlobalSize(hData);
    
    if(*pFileSize < buffLen)
    {
        //GlobalUnlock(hData);
        //CloseClipboard();
        //printf("【%d】【%d】 %s\n", *pFileSize, buffLen, filename);
        //return false;

        printf("【%d】【%d】 %s\n", *pFileSize, buffLen, filename);
        memcpy(pData, buffer, *pFileSize);
        GlobalUnlock(hData);
        CloseClipboard();

        JKCommon_WriteLog("W",__LINE__,"超过加密长度>", filename);
        return true;
    }
    else
    {
        //复制数据
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
        JKCommon_WriteLog("W",__LINE__,"新建进程失败>", strFileName);
        return false;
    }
    
    //执行按键命令
    ::Sleep(800);

    //ctrl+a
    keybd_event(VK_CONTROL,0,0,0);//按下Shift键
    keybd_event('A',0,0,0);//按下a键
    keybd_event('A',0,KEYEVENTF_KEYUP,0);//松开a键
    keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);//松开Shift键
    //::Sleep(200);
    //复制
    keybd_event(VK_CONTROL,0,0,0);//按下Shift键
    keybd_event('C',0,0,0);//按下a键
    keybd_event('C',0,KEYEVENTF_KEYUP,0);//松开a键
    keybd_event(VK_CONTROL,0,KEYEVENTF_KEYUP,0);//松开Shift键

    ::Sleep(800);//给剪切板时间
    //打开剪切板
    if(! JKCommon_GetClipboardText(strFileName, pData, pFileSize))
    {
        system("TASKKILL /F /IM notepad.exe");
        return false;
    }

    system("TASKKILL /F /IM notepad.exe");

    return true;//返回
}

bool JKCommon_WriteFile(char *strFileName,DWORD uFileSize,byte* pData)
{
    //打开文件，不存在则新建
    HANDLE hFile = ::CreateFileA(strFileName, GENERIC_WRITE, 0, 
        NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)//判断
    {
        JKCommon_WriteLog("W",__LINE__,"写文件打开失败>", strFileName);
        return false;//返回
    }

    DWORD nRet;//返回结果 
    if(WriteFile(hFile,pData, uFileSize, &nRet,NULL) == FALSE)//写失败
    {
        ::CloseHandle(hFile);//关闭句柄 
        JKCommon_WriteLog("W",__LINE__,"写文件失败>", strFileName);
        return false;//返回
    }

    ::CloseHandle(hFile);//关闭句柄 
    return true;//返回
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
    pSrcTail++;//跳过
    if(strlen(pSrcTail) >= sizeof(tmpStr) /2)
    {
        //printf("error: 后缀名超过32个字符!!!\n");

        JKCommon_WriteLog("W",__LINE__,"后缀名超长>", srcfile);
        return 1;
    }

    for(int i = 0,j = 0; i < strlen(pSrcTail); i++, j += 2)
    {
        sprintf(tmpStr + j, "%02X", (byte *)*(pSrcTail + i));
    }

    pDestTail++;
    *pDestTail = '_';//前缀
    pDestTail++;

    memcpy(pDestTail, tmpStr, strlen(tmpStr));

    return 0;
}

int SuffixDescrypt(char *srcfile, char *destfile)
{
    char *pSrcTail = strirstr(srcfile, "._");//找到最后一个
    if(pSrcTail == NULL)
        return 2;

    char tmpStr[64];
    ZeroMemory(tmpStr, sizeof(tmpStr));
    pSrcTail += 2;//跳过

    if(strlen(pSrcTail) >= sizeof(tmpStr) /2)
    {
        JKCommon_WriteLog("W",__LINE__,"后缀名超长>", srcfile);
        return 1;
    }

    int bTmp = 0;
    for(int i = 0,j = 0; i < strlen(pSrcTail); i++, j += 2)
    {
        bTmp = (int)(JKCommon_Char2Num(*(pSrcTail + j))) * 16 + 
            (int)JKCommon_Char2Num(*(pSrcTail + j + 1));

        sprintf(tmpStr + i, "%c",  bTmp);
    }
    memcpy(destfile, srcfile, pSrcTail - srcfile - 1);//去掉'_'
    sprintf(destfile + strlen(destfile), "%s\0", tmpStr);
    return 0;
}


int CF_Self(char *srcfile, char *destfile)
{
    DWORD oldfilelen = JKCommon_GetFileSize_Huge(srcfile);
    if(oldfilelen < 1)
    {
        JKCommon_WriteLog("W",__LINE__,"文件为空>", srcfile);
        return 1;
    }

    byte *fileData = (byte *)GlobalAlloc(GPTR, oldfilelen);
    if(fileData == NULL)
    {
        JKCommon_WriteLog("W",__LINE__,"申请空间失败>", srcfile);
        return 2;
    }

    if(! JKCommon_ReadFile(srcfile, &oldfilelen, fileData))
    {
        GlobalFree(fileData);
        //JKCommon_WriteLog("W",__LINE__,"读文件失败>", srcfile);
        return 3;
    }

    if(! JKCommon_WriteFile(destfile, oldfilelen, fileData))
    {
        GlobalFree(fileData);
        //printf("写文件失败：%s\n", destfile);
        return 4;
    }

    GlobalFree(fileData);

    //判断文件大小
    DWORD newfilelen = JKCommon_GetFileSize_Huge(destfile);
    if(newfilelen < 1 || newfilelen < oldfilelen)
    {
        JKCommon_WriteLog("W",__LINE__,"文件为空或长度不对 > ", destfile);
        return 1;
    }
    
    if((newfilelen - oldfilelen) >= 5)
    {
        JKCommon_WriteLog("Q",__LINE__,"超过原文件5Byte以上 > ", destfile);
        return 0;
    }
    return 0;
}

int BuildFile(CJKFileDecDlg *pCJKFDD, int type, char *srcpath, char *destpath)
{
    char strLog[256];
    //遍历文件名
    BOOL bFound = FALSE;//赋值

    char tmpPath[512];//变量
    ZeroMemory(tmpPath, sizeof(tmpPath));//初始化
    sprintf(tmpPath,"%s*.*\0", srcpath);//组合数据

    char SrcSubPath[512];
    char DestSubPath[512];
    //---------遍历文件-------------------//
    _finddatai64_t findData;//变量
    intptr_t handle = _findfirsti64(tmpPath, &findData);// 查找目录中的第一个文件
    if (handle == -1)//判断
    {
        return -1;//返回
    }

    do
    {
        //只有一层目录
        if (findData.attrib & _A_SUBDIR)// 是否是子目录并且不为"."或".."
        {
            if(! strcmp(findData.name, ".") == 0 && ! strcmp(findData.name, "..") == 0)
            {
                ZeroMemory(SrcSubPath, sizeof(SrcSubPath));//初始化
                sprintf(SrcSubPath, "%s%s\\\0", srcpath, findData.name);//组合数据

                if(type == 1)//加密
                {
                    ZeroMemory(DestSubPath, sizeof(DestSubPath));//初始化
                    sprintf(DestSubPath, "%s%s\\\0", destpath, findData.name);//组合数据
                    JKCommon_BuildFolder(DestSubPath);//新建文件夹

                    if(BuildFile(pCJKFDD, type, SrcSubPath, DestSubPath) == -99)//进入子文件夹
                        return -99;
                }
                else
                {
                    if(BuildFile(pCJKFDD, type, SrcSubPath, "") == -99)//进入子文件夹
                        return -99;
                }
            }
        }
        else
        {
            //复制文件（读取）
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
                        sprintf(strLog, "后缀名异常，详细看日志[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    if(CF_Self(srcfile, destfile) != 0)
                    {
                        sprintf(strLog, " 剪切版复制异常，详细看日志[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    pCJKFDD->m_sta_hint.SetWindowTextA(destfile);
                }
                else
                {
                    if(! CopyFile(srcfile, destfile, false))
                    {
                        sprintf(strLog, " 非加密复制异常，详细看日志[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }
                    else
                    {
                        //printf("【复制】：%s\n",destfile);
                    }
                }
            }
            else
            {
                char destfile[512];
                ZeroMemory(destfile, sizeof(destfile));

                int iRet = SuffixDescrypt (srcfile, destfile);
                if(iRet == 1)//数据错误
                    return -99;
                else if(iRet == 0)
                {
                    if(rename(srcfile, destfile) != 0)
                    {
                        sprintf(strLog, " 重命名失败异常，详细看日志[%s]\0", destfile);
                        pCJKFDD->m_sta_hint.SetWindowTextA(strLog);
                        return -99;
                    }

                    //printf("【解密】：%s\n",destfile);
                }
            }
        }
    } while (_findnexti64(handle, &findData) == 0);// 查找目录中的下一个文件

    _findclose(handle); // 关闭搜索句柄
    return 0;//返回
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


// CJKFileDecDlg 消息处理程序

BOOL CJKFileDecDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
    ZeroMemory(m_FilterList,sizeof(m_FilterList));//清空
    m_FilterSum = 0;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CJKFileDecDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
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
    //增加后续类型

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


    //启动线程
    ZeroMemory(m_OldPath, sizeof(m_OldPath));
    m_edt_oldpath.GetWindowTextA(m_OldPath, sizeof(m_OldPath));

    ZeroMemory(m_NewPath, sizeof(m_NewPath));
    m_edt_newpath.GetWindowTextA(m_NewPath, sizeof(m_NewPath));

    char subpath[MAX_PATH];
    ZeroMemory(subpath, sizeof(subpath));

    if(strlen(m_OldPath) < 1 || strlen(m_NewPath) < 1)
    {
        MessageBoxA("加密目录及解密目录不允许为空", "错误", MB_OK | MB_ICONERROR);
        return;
    }

    if(m_OldPath[strlen(m_OldPath)] != '\\')
        strncat(m_OldPath, "\\", 1);
    if(m_NewPath[strlen(m_NewPath)] != '\\')
        strncat(m_NewPath, "\\", 1);

    if(m_chk_addsubpath.GetCheck() == BST_CHECKED)
    {
        //判断最后子目录是否相同
        char *pOldEnd = NULL;
        char *pNewEnd = NULL;

        pOldEnd = strrchr(m_OldPath, '\\');//找到最后一个
        pNewEnd = strrchr(m_NewPath, '\\');//找到最后一个
        
        if(pOldEnd != NULL && pNewEnd != NULL)
        {
            char *pOldPrev = NULL;
            char *pNewPrev = NULL;

            *pOldEnd = '|';
            *pNewEnd = '|';

            pOldPrev = strrchr(m_OldPath, '\\');//找到最后一个
            pNewPrev = strrchr(m_NewPath, '\\');//找到最后一个

            *pOldEnd = '\\';
            *pNewEnd = '\\';

            if(pOldPrev != NULL && pNewPrev != NULL)
            {
                if(strcmp(pOldPrev, pNewPrev))//内容不同
                {
                    memcpy(subpath, pOldPrev + 1, pOldEnd - pOldPrev);//得到数据
                }
                else
                {
                    if(IDYES != MessageBoxA("最后一级子目录相同，继续将覆盖原数据，是否继续","提示",
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
        MessageBoxA("【启动解密】失败", "错误", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxA("【启动解密】成功", "错误", MB_OK | MB_ICONINFORMATION);
    }

    ////启动线程
    //HANDLE hnd = (HANDLE)_beginthreadex(NULL,0,ThdDes, (void *)this,CREATE_SUSPENDED,NULL);

    //if(hnd != NULL)
    //{
    //    m_ThdFlag_Des = 1;//启动成功

    //    m_btn_rundes.EnableWindow(FALSE);
    //    m_btn_stopdes.EnableWindow(TRUE);

    //    ResumeThread(hnd);//唤醒线程
    //    CloseHandle(hnd);
    //}
    //else
    //{
    //    m_btn_rundes.EnableWindow(TRUE);
    //    m_btn_stopdes.EnableWindow(FALSE);
    //    m_sta_hint.SetWindowTextA("解密线程启动失败");
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
        MessageBoxA("【解密还原】失败", "错误", MB_OK | MB_ICONERROR);
    }
    else
    {
        MessageBoxA("【解密还原】成功", "错误", MB_OK | MB_ICONINFORMATION);
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