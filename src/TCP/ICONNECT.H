/*
 * iconnect.h
 * Prototypes de l'interface IConnect.
 *
 * Interface de ICONNECT.C. D‚clare le drapeau iconnect.
 *
 * Utilis‚ par : ICONNECT.C
 *
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

EXTERN	BOOLEAN	iconnect;

/* ---    Export Prototypage        --- */

BOOLEAN ICONNECT_init(VOID);

int16 ICONNECT_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize);
int16 ICONNECT_TCP_wait_state(int16 cn, int16 state, int16 timeout );
int16 ICONNECT_TCP_send(int16 cn, char *buf, int16 len);
int16 ICONNECT_TCP_close(int16 cn, int16 timeout);
char  *ICONNECT_get_err_text(int16 code);
int16 ICONNECT_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize);
int16 ICONNECT_CNbyte_count(int16 cn);
int16 ICONNECT_CNget_block(int16 cn, char *blk, int16 len);
char *ICONNECT_getvstr(char *variable );


/* ---          EOF                 --- */
