/*
 * rsc.h
 * Prototypes de la gestion de la ressource.
 *
 * Interface de RSC.C.
 *
 * Utilise : CAB_OVL.H
 * Utilis‚ par : AUTHEN.C, CONFIG.C, COOKIE.C, HTTP_GET.C, LANGUAGE.C,
 *   MAILTO.C, MAIN.C, MY_HEAD.C, REPORT.C, RSC.C, SETUP.C, TCP.C,
 *   TOOLS.C
 *
 * Cr‚ation : 07/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

#include	"cab_ovl.h"

/* --- Export Constantes globales   --- */

/* --- Export Structures globales   --- */

/* --- Export Variables globales    --- */

#ifdef __AES__
	EXTERN OBJECT *adr_form;
	EXTERN OBJECT *err_form;
#endif

/* ---    Export Prototypage        --- */

/* ----- Principal ----- */

BOOLEAN	rsc_load( UBYTE *rsc );
INT			rsc_free( VOID );

/* ----- cookie dialogue ----- */

INT	ask_cookie_accepte( UBYTE *server, UBYTE *domain, UBYTE *path, UBYTE *name, UBYTE *value, UBYTE *expires );

/* ----- authentification dialogue ----- */

INT ask_authentification( UBYTE *server, UBYTE **login_ptr, UBYTE **password_ptr);

/* ----- Alerte ----- */

INT		my_form_alerte( UBYTE *titre, const char *message, ... );
UBYTE	*ChaineAlerte( INT index );

/* ----- Outils ----- */

UBYTE *STinGRessource( INT err );
INT		NextString( INT pere , UBYTE **string);

/* ---          EOF                 --- */
