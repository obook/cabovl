/*
 * mintnet.h
 * Prototypes de l'interface MiNTNet.
 *
 * Interface de MINTNET.C. D‚clare le drapeau mintnet.
 *
 * Utilis‚ par : MINTNET.C
 *
 * Cr‚ation : 09/09/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

BOOLEAN mintnet;

/* ---    Export Prototypage        --- */

int16 MINTNET_TCP_open(uint32 rhost, int16 rport, int16 tos, uint16 obsize);

int16 MINTNET_TCP_wait_state(int16 cn, int16 state, int16 timeout );

int16 MINTNET_TCP_send(int16 cn, char *buf, int16 len);

int16 MINTNET_TCP_close(int16 cn, int16 timeout);

char  *MINTNET_get_err_text(int16 code);

int16 MINTNET_resolve(char *dn, char **rdn, uint32 *alist, int16 lsize);

int16 MINTNET_CNbyte_count(int16 cn);

int16 MINTNET_CNget_block(int16 cn, char *blk, int16 len);

char *MINTNET_getvstr(char *variable );

/* ---          EOF                 --- */
