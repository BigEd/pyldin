#ifndef _SWI_H_
#define _SWI_H_

#define SystemSWI			0x80

#define SWI_WriteC			0x70
#define SWI_Write			0x71
#define SWI_WriteHex			0x72
#define SWI_ReadC			0x73

#define SWI_NEWLIB_Open_r		0x80
#define SWI_NEWLIB_Read_r		0x81
#define SWI_NEWLIB_Write_r		0x82
#define SWI_NEWLIB_Close_r		0x83
#define SWI_NEWLIB_Lseek_r		0x84
#define SWI_NEWLIB_Fstat_r		0x85
#define SWI_NEWLIB_Isatty_r		0x86

#define SWI_NEWLIB_Gettimeofday_r	0x87
#define SWI_NEWLIB_Exit			0x88

#define SWI_NEWLIB_Mkdir_r		0x89
#define SWI_NEWLIB_Rmdir_r		0x8a

#define SWI_NEWLIB_Opendir_r		0x8b
#define SWI_NEWLIB_Readdir_r		0x8c
#define SWI_NEWLIB_Closedir_r		0x8d

#define SWI_NEWLIB_Unlink_r		0x8e
#define SWI_NEWLIB_Rename_r		0x8f

#define SWI_NEWLIB_Chdir_r		0x90
#define SWI_NEWLIB_Getcwd_r		0x91

#define SWI_MountFS			0xa0
#define SWI_UmountFS			0xa1

#define SWI_ELF_Load			0xb0
#define SWI_ELF_SetStackSize		0xb1
#define SWI_ELF_GetStackSize		0xb2

/* Now the SWI numbers and reason codes for RDI (Angel) monitors.  */
#define AngelSWI_ARM 			0x123456
#ifdef __thumb__
#define AngelSWI 			0xAB
#else
#define AngelSWI 			AngelSWI_ARM
#endif

/* The reason codes:  */
#define AngelSWI_Reason_Open		0x01
#define AngelSWI_Reason_Close		0x02
#define AngelSWI_Reason_WriteC		0x03
#define AngelSWI_Reason_Write0		0x04
#define AngelSWI_Reason_Write		0x05
#define AngelSWI_Reason_Read		0x06
#define AngelSWI_Reason_ReadC		0x07
#define AngelSWI_Reason_IsTTY		0x09
#define AngelSWI_Reason_Seek		0x0A
#define AngelSWI_Reason_FLen		0x0C
#define AngelSWI_Reason_TmpNam		0x0D
#define AngelSWI_Reason_Remove		0x0E
#define AngelSWI_Reason_Rename		0x0F
#define AngelSWI_Reason_Clock		0x10
#define AngelSWI_Reason_Time		0x11
#define AngelSWI_Reason_System		0x12
#define AngelSWI_Reason_Errno		0x13
#define AngelSWI_Reason_GetCmdLine 	0x15
#define AngelSWI_Reason_HeapInfo 	0x16
#define AngelSWI_Reason_EnterSVC 	0x17
#define AngelSWI_Reason_ReportException 0x18
#define ADP_Stopped_ApplicationExit 	((2 << 16) + 38)
#define ADP_Stopped_RunTimeError 	((2 << 16) + 35)

#endif
