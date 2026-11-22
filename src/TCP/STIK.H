/*
 * stik.h
 * Prototypes de l'interface STiK/STinG.
 *
 * Interface de STIK.C.
 *
 * Utilis‚ par : STIK.C
 *
 * Cr‚ation : 10/04/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

/* ---    Export Prototypage        --- */

BOOLEAN STIK_init(VOID);
int16 STIK_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize);
int16 STIK_TCP_wait_state(int16 cn, int16 state, int16 timeout );
int16 STIK_TCP_send(int16 cn, char *buf, int16 len);
int16 STIK_TCP_close(int16 cn, int16 timeout);
char  *STIK_get_err_text(int16 code);
int16 STIK_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize);
int16 STIK_CNbyte_count(int16 cn);
int16 STIK_CNget_block(int16 cn, char *blk, int16 len);
char *STIK_getvstr(char *variable );

/* ---          EOF                 --- */
