#pragma once
#ifndef __COMMAND_LINE_OPTION_H__
#define __COMMAND_LINE_OPTION_H__

#include "arch/frame/stdafx.h"
#include "aipcdefs.h"

class CCommandLineOption
{
public:
	CString m_strDisk1Path;
	CString m_strDisk2Path;
	CString m_strStatePath;
	CString m_strHardDiskPath;
	CString m_strErrorMsg;
	BOOL m_bSaveOnExit;
	BOOL m_bReboot;

public:
	CCommandLineOption();
	~CCommandLineOption();
	BOOL Parse(CString strCommandLine);
};

#endif
