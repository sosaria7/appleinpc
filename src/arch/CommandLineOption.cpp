#include "arch/CommandLineOption.h"

CCommandLineOption::CCommandLineOption() :
	m_strDisk1Path(),
	m_strDisk2Path(),
	m_strStatePath(),
	m_strHardDiskPath(),
	m_bSaveOnExit(FALSE),
	m_bReboot(FALSE)
{
}

CCommandLineOption::~CCommandLineOption()
{
}

extern "C"
TCHAR**
CommandLineToArgv(
	const TCHAR* CmdLine,
	int* _argc
	)
{
	TCHAR** argv;
	TCHAR*  _argv;
	ULONG   len;
	ULONG   argc;
	TCHAR   a;
	ULONG   i, j;

	BOOLEAN  in_QM;
	BOOLEAN  in_TEXT;
	BOOLEAN  in_SPACE;

	len = _tcslen(CmdLine);
	i = ((len + 2) / 2)*sizeof(PVOID) + sizeof(PVOID);

	argv = (TCHAR**)LocalAlloc(LMEM_FIXED,
		i + (len + 2)*sizeof(TCHAR));

	_argv = (TCHAR*)(((PUCHAR)argv) + i);

	argc = 0;
	argv[argc] = _argv;
	in_QM = FALSE;
	in_TEXT = FALSE;
	in_SPACE = TRUE;
	i = 0;
	j = 0;

	while (a = CmdLine[i]) {
		if (in_QM) {
			if (a == '\"') {
				in_QM = FALSE;
			}
			else {
				_argv[j] = a;
				j++;
			}
		}
		else {
			switch (a) {
			case '\"':
				in_QM = TRUE;
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				in_SPACE = FALSE;
				break;
			case ' ':
			case '\t':
			case '\n':
			case '\r':
				if (in_TEXT) {
					_argv[j] = '\0';
					j++;
				}
				in_TEXT = FALSE;
				in_SPACE = TRUE;
				break;
			default:
				in_TEXT = TRUE;
				if (in_SPACE) {
					argv[argc] = _argv + j;
					argc++;
				}
				_argv[j] = a;
				j++;
				in_SPACE = FALSE;
				break;
			}
		}
		i++;
	}
	_argv[j] = '\0';
	argv[argc] = NULL;

	(*_argc) = argc;
	return argv;
}

BOOL CCommandLineOption::Parse(CString strCommandLine)
{
	TCHAR** argv;
	int argc;
	int i;

	m_strErrorMsg = TEXT("");
	m_strDisk1Path = TEXT("");
	m_strDisk2Path = TEXT("");
	m_strHardDiskPath = TEXT("");
	m_strStatePath = TEXT("");
	m_bSaveOnExit = FALSE;
	m_bReboot = FALSE;

	argv = CommandLineToArgv(strCommandLine, &argc);

	for (i = 1; i < argc; i++)
	{
		if (i < argc - 1)
		{
			if (_tcscmp(argv[i], TEXT("-d1")) == 0)
			{
				m_strDisk1Path = argv[i + 1];
				i++;
				continue;
			}
			else if (_tcscmp(argv[i], TEXT("-d2")) == 0)
			{
				m_strDisk2Path = argv[i + 1];
				i++;
				continue;
			}
			else if (_tcscmp(argv[i], TEXT("-h")) == 0)
			{
				m_strHardDiskPath = argv[i + 1];
				i++;
				continue;
			}
			else if (_tcscmp(argv[i], TEXT("-s")) == 0)
			{
				m_strStatePath = argv[i + 1];
				i++;
				continue;
			}
		}
		if (_tcscmp(argv[i], TEXT("-w")) == 0)
		{
			m_bSaveOnExit = TRUE;
			continue;
		}
		else if (_tcscmp(argv[i], TEXT("-r")) == 0)
		{
			m_bReboot = TRUE;
			continue;
		}
		break;
	}
	LocalFree(argv);
	if (i < argc)
	{
		m_strErrorMsg.Format(TEXT("Invalid argument: %s"), argv[i]);
		return FALSE;
	}
	return TRUE;
}
