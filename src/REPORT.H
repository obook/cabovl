/*
 * REPORT.H - pages d'erreur et de rapport de CAB.OVL (V5.68)
 *
 * Reconstitué à partir du binaire CAB.OVL. Prototypes de REPORT.C.
 */

#ifndef REPORT_H
#define REPORT_H

#include "REQUEST.H"

void report_error(REQUEST *req, char *file, int code, short *fh);   /* 0x146ee */
int ReportRequestText(REQUEST *req, char *text);                    /* 0x14760 */
void file_error(char *file, int op, short *err);                    /* 0x14952 */
int ReportText(char *url, char *file, short *fh, char *text);       /* 0x149c0 */

#endif
