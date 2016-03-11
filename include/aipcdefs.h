#ifndef __AIPCDEFS_H__
#define __AIPCDEFS_H__


// OS 환경에 따른 설정
#define PATH_MAX _MAX_PATH
#define FILE_SEPARATOR '\\'

// define for return values
#define E_SUCCESS			0
#define E_OPEN_FAIL			1
#define E_READ_FAIL			2
#define E_WRITE_FAIL		3
#define E_UNKNOWN_FORMAT	4

#define CHANGE_WORD_ORDER(x)	( (x >> 8) | ( (x << 8) & 0xFF00 ) )
#define CHANGE_DWORD_ORDER(x)	( ( ( (x >> 24) | ( x << 8 ) ) & 0x00FF00FF ) \
								  ( ( (x << 24) | ( x >> 8 ) ) & 0xFF00FF00 ) )
#ifdef _BIGENDIAN
#define WORD_ORDER(x)	CHANGE_WORD_ORDER(x)
#define DWORD_ORDER(x)	CHANGE_DWORD_ORDER(x)
#else
#define WORD_ORDER(x)	(x)
#define DWORD_ORDER(x)	(x)
#endif

// define for card type
#define CARD_EMPTY				0
#define CARD_MOUSE_INTERFACE	1
#define CARD_DISK_INTERFACE		2
#define CARD_PHASOR				3
#define CARD_SD_DISK_II			4

// define for disk images
#define IMAGE_DOS		1
#define IMAGE_PRODOS	2
#define	IMAGE_NIBBLE	3

// apple thread messages
#define ACS_RESET		1
#define ACS_REBOOT		2
#define ACS_DEBUG		3
#define ACS_POWERON		4
#define ACS_POWEROFF	5
#define ACS_SUSPEND		6
#define ACS_RESUME		7
#define ACS_EXIT		8

// User Message
#define UM_REQACQUIRE	( WM_USER+1 )
#define UM_KEYDOWN		( WM_USER+2 )
#define UM_KEYUP		( WM_USER+3 )
#define UM_KEYREPEAT	( WM_USER+4 )
#define UM_MOUSE_EVENT	( WM_USER+5 )
#define UM_DISPLAY_CHANGE	( WM_USER+6 )

#define MACHINE_APPLE2P		1
#define MACHINE_APPLE2E		2
#define MACHINE_APPLE2C		3

#endif