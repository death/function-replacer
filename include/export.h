/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
/* Visual Basic Disassembled String Reference Analyser        (c) DEATH  */
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
/* export.h                                                              */
/*                                                                       */
/* Export references functions include file                              */
/* 컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴� */
#ifndef _EXPORT_H
#define _EXPORT_H


#define MAX_EXPREF  1024

extern char szExportRef[MAX_EXPREF][256];

DWORD GetExports(char *Filename);


#endif /* _EXPORT_H */
