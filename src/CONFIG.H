/*
 * config.h
 * Prototypes de la configuration.
 *
 * Interface de CONFIG.C.
 *
 * Utilis‚ par : CONFIG.C, COOKIE.C, HTTP_GET.C, MAILTO.C, MAIN.C,
 *   MY_HEAD.C, RSC.C, SETUP.C, TOOLS.C
 *
 * Cr‚ation : 01/01/1996
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* --- Export Constantes globales   --- */

#define CONFIG_VERSION	3L

/* --- Export Structures globales   --- */

typedef struct Config
{
	LONG		version;
	BOOLEAN	logfile_open;
	BOOLEAN	debug;
	INT			timeout;
	FLOAT		gem;				/* appel de aes_event toutes les x ms */
	INT			timer;			/* duree du aes_event de x ms */
	WORD		cookie;			/* reaction face on un cookie */
	BOOLEAN online;			/* si FALSE on ne contacte personne (force cache) */
	BOOLEAN	reserve1;		/* pas utilis‚ */
}CONFIG;

/* --- Export Variables globales    --- */

EXTERN CONFIG config;

/* ---    Export Prototypage        --- */

BOOLEAN InitConf(UBYTE *filename);
BOOLEAN SaveConf(UBYTE *filename);

/* ---          EOF                 --- */
