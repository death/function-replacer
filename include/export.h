/* ��������������������������������������������������������������������� */
/* Visual Basic Disassembled String Reference Analyser        (c) DEATH  */
/* ��������������������������������������������������������������������� */
/* export.h                                                              */
/*                                                                       */
/* Export references functions include file                              */
/* ��������������������������������������������������������������������� */
#ifndef _EXPORT_H
#define _EXPORT_H


#define MAX_EXPREF  1024

extern char szExportRef[MAX_EXPREF][256];

DWORD GetExports(char *Filename);


#endif /* _EXPORT_H */
