/*
 * rsc.c
 * Fichier ressource GEM et boŒtes de dialogue du module.
 *
 * Charge CAB_OVL.RSC, gŠre les dialogues (cookie, authentification,
 * alerte) et leur boucle d'‚v‚nements AES.
 *
 * Utilise : AUTHEN.H, CONFIG.H, GLOBALS.H, MAIN.H
 * Utilis‚ par : AUTHEN.C, CONFIG.C, COOKIE.C, HTTP_GET.C, LANGUAGE.C,
 *   MAILTO.C, MAIN.C, MY_HEAD.C, REPORT.C, SETUP.C, TCP.C, TOOLS.C
 *
 * Cr‚ation : 07/07/1998
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<ctype.h>	/* toupper */
#include	<tos.h>
#include	<aes.h>
#include	<vdi.h>
#include	<gem.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab/module.h>

#ifdef _RSC_INT_
	#include	<windform/windforx.h>
#else
	#include	<windform/wform300.h>
#endif

#ifdef _RSC_INT_
	#pragma -STR
	#include "cab_ovl.rh"
	#include "cab_ovl.rsh"
	#pragma +STR
#else
	#include "cab_ovl.h"
	MFDB Screen_MFDB;
#endif

#include "globals.h"
#include "main.h"
#include "config.h"
#include "authen.h"

/* ---       Includes internes       -- */

#include "rsc.h"	

/* --- Constantes globales internes --- */

#define 	objc_set_xdisabled(tree,objc)  objc_change(tree,objc,0,0,0,640,400,DISABLED,0)

#define	LONG_MAX_TITRE	60
#define	LONG_MAX_LIGNE	60
#define	LONG_MAX_BOUTON	10

#define	NBRE_LIGNES			4
#define	NBRE_BOUTONS		5

#define	ATTEND_BRAKET_DROIT		0
#define	ATTEND_BRAKET_GAUCHE	1

/* pour x_stdkey */

#define	SHIFT		0x03	/* les 2 shifts,droite et gauche */
#define	CONTROL		0x04
#define	ALT			0x08
#define	CAPSLOCK	0x10

/* --- Structures globales internes --- */

typedef struct
{
	char	shift;
	char	scan;
	char	std;
	char	ascii1;
	char	ascii2;
}Key;

/* --- Variables globales  internes --- */

OBJECT *sting_form;
OBJECT *report_form;
OBJECT *cook_form;
OBJECT *alerte_form;
OBJECT *authen_form;

WINDFORM_VAR cook_var;
WINDFORM_VAR authen_var;
WINDFORM_VAR alerte_var;

LOCAL UBYTE buffer[256];
UBYTE titre_alerte[26];	/* utilis‚ par rsc.c */

/*int h_ligne;*/		/* hauteur d'une ligne de texte	?? */
/*int baseline;*/		/* pour v_gtext() ?? */
int xf, yf, wf, hf; /* Coordonn‚es de la fenˆtre texte ?? */

extern int vdi_handle;
extern int x_desk, y_desk, w_desk, h_desk; /* Coordonn‚es du bureau */
extern int buff[8], mousex, mousey, mousek, key, key_s, nbr_clics;
extern int w_char, h_char;
extern int nb_planes;				/* Nombre de plans de couleurs.	*/
extern int color_3d1, color_3d2;	/* Couleurs pour la 3D */
extern int wtop;					/* fenetre actuellement active */

/* ---     Prototypage interne      --- */

LOCAL VOID fix_rsc(WORD nb_tree);
LOCAL UBYTE *MaxString( UBYTE *string, INT limite );
VOID X_strncat( UBYTE *dest , UBYTE *orig, INT limit );

INT AES_LOOP( INT index );
LOCAL int dialogue_event(int event);
LOCAL int error_event(int event);
LOCAL int cook_event(int event);
LOCAL int authen_event(int event);
LOCAL int alerte_event(int event);
LOCAL VOID SetLignes( UBYTE *chaine );
LOCAL VOID SetBoutons( UBYTE *chaine );
LOCAL VOID keyboard(int handle,int key,int key_s);
LOCAL void x_stdkey( int k, int ks , Key *r );

/* ---         Fonctions            --- */

BOOLEAN rsc_load( UBYTE *rsc )
{
#ifdef _RSC_INT_
int i, attributes[12], work_in[11], work_out[57];
#endif

	LOG_printf("> rsc_load <%s>\n", rsc);
	
#ifdef _RSC_INT_

	app_id = appl_init();

/*vdi_handle = graf_handle(&w_char, &h_char, &i, &i); */
	vdi_handle = (aes_handle = graf_handle(&w_char, &h_char, &i, &i));
/* hauteur des caractŠres + 1 pour ‚viter que les lignes de textes
ne se touchent. */

/*h_ligne = h_char + 1; */

	for (i = 0 ; i < 9 ; work_in[i++] = 1);
	work_in[9] = 0;		/* Couleur de Remplissage = 0 */
	work_in[10] = 2;	/* SystŠme de coordonn‚es RC */
	v_opnvwk(work_in, &aes_handle, work_out);
	v_opnvwk(work_in, &vdi_handle, work_out);

	if (work_out[13] < 16)	/* Etablissement des couleurs 3D */
	{
		color_3d1 = WHITE;	/* color index 0	*/
		color_3d2 = BLACK;	/* color index 1	*/
	}
	else
	{
		color_3d1 = LWHITE;	/* color index 8	*/
		color_3d2 = LBLACK;	/* color index 9	*/
	}

	vq_extnd (vdi_handle, TRUE, work_out);
	nb_planes = work_out[4];	/* Nbre de plans de couleurs */

	vqt_attributes(vdi_handle, attributes);
/*baseline = 1 + attributes[7]; *//* Ligne de base des caractŠres + 1 pour ‚viter
que la premiŠre ligne de texte ne touche le haut de la fenˆtre. */

	/* Coordonn‚es du bureau : */
	wind_get(0, WF_WORKXYWH, &x_desk, &y_desk, &w_desk, &h_desk);

#else

	init_gem();

#endif

#ifdef _RSC_INT_

	fix_rsc( NUM_TREE );

	report_form=rs_trindex[ REPORT ];
	cook_form=rs_trindex[ COOK ];
	authen_form=rs_trindex[ AUTH ];
	alerte_form=rs_trindex[ ALERT ];

	init_var(&cook_var, cook_form, 0, 0);
	init_var(&authen_var, authen_form, 0, 0);
	init_var(&alerte_var, alerte_form, 0, 0);

#else

	if ( load_rsc( rsc , NO_WINDOW, ICONIF) == FALSE)
	{
		return(FALSE);
	}

	init_windform(&cook_var, COOK, 0, 0);
	cook_form=cook_var.adr_form;

	init_windform(&authen_var, AUTH, 0, 0);
	authen_form=authen_var.adr_form;

	init_windform(&alerte_var, ALERT, 0, 0);
	alerte_form=alerte_var.adr_form;

	rsrc_gaddr(0, STING, &sting_form);
	rsrc_gaddr(0, REPORT, &report_form);
	
#endif

return( TRUE );
}

INT rsc_free( VOID )
{
	v_clsvwk(vdi_handle);
	v_clsvwk(aes_handle);	/* NEW */
#ifndef _RSC_INT_
	rsrc_free();					/* NEW */
#endif
	appl_exit();
return(0);
}

INT ask_cookie_accepte( UBYTE *server, UBYTE *domain, UBYTE *path, UBYTE *name, UBYTE *value, UBYTE *expires )
{

	LOG_printf("> ask_cookie_accepte\n");
	
	*cook_form[COOK_SERVER].ob_spec.tedinfo->te_ptext = '\0';
	*cook_form[COOK_PATH].ob_spec.tedinfo->te_ptext = '\0';
	*cook_form[COOK_NAME].ob_spec.tedinfo->te_ptext = '\0';
	*cook_form[COOK_VALUE].ob_spec.tedinfo->te_ptext = '\0';
	*cook_form[COOK_EXPIRES].ob_spec.tedinfo->te_ptext = '\0';
	*cook_form[COOK_DOMAINE].ob_spec.tedinfo->te_ptext = '\0';

	if ( server != NULL )
		X_strncat( cook_form[COOK_SERVER].ob_spec.tedinfo->te_ptext , server, MAX_RSC);

	if ( domain != NULL )
		X_strncat( cook_form[COOK_DOMAINE].ob_spec.tedinfo->te_ptext , domain, 50);

	if ( path != NULL )
		X_strncat( cook_form[COOK_PATH].ob_spec.tedinfo->te_ptext , path, 50);

	if ( name != NULL )
		X_strncat( cook_form[COOK_NAME].ob_spec.tedinfo->te_ptext , name, 50);

	if ( value != NULL )
		X_strncat( cook_form[COOK_VALUE].ob_spec.tedinfo->te_ptext , value, 50);

	if ( expires != NULL )
		X_strncat( cook_form[COOK_EXPIRES].ob_spec.tedinfo->te_ptext , expires, 50);

return ( AES_LOOP( COOK ) );
}

INT ask_authentification( UBYTE *server, UBYTE **login_ptr, UBYTE **password_ptr)
{

	LOG_printf("> ask_authentification\n", server);
	
	*authen_form[AUTH_LOGIN].ob_spec.tedinfo->te_ptext = '\0';
	*authen_form[AUTH_PASSWORD].ob_spec.tedinfo->te_ptext = '\0';

	*(char **)&login_ptr[0] = authen_form[AUTH_LOGIN].ob_spec.tedinfo->te_ptext;
	*(char **)&password_ptr[0] = authen_form[AUTH_PASSWORD].ob_spec.tedinfo->te_ptext;
	
return ( AES_LOOP( AUTH ) );
}

#ifdef _RSC_INT_

LOCAL VOID fix_rsc(WORD nb_tree)
{
WORD i, j;

	LOG_printf("> fix_rsc\n");

	for (i = 0 ; i < nb_tree ; i++)
	{
		j = 0;
		do
		{
			rsrc_obfix (rs_trindex[i], j);
		} while ( !(rs_trindex[i][j++].ob_flags & LASTOB) );
	}
}

#endif

LOCAL UBYTE *MaxString( UBYTE *string, INT limite )
{
INT len;
static UBYTE rien[]="";

	if ( string == NULL ) return(rien);
	len = (INT)strlen(string);
	if ( len > limite )
		return ( &string[len-limite] );
return(string);
}

/*LOCAL VOID X_strncat( UBYTE *dest , UBYTE *orig, INT limit ) */
VOID X_strncat( UBYTE *dest , UBYTE *orig, INT limit )
{
	*dest = '\0';
	strncat( dest , MaxString( orig, limit ), limit);
}

INT AES_LOOP( INT index )
{
int quit = 0, event, dummy;

	xf = x_desk + 10;	/* Coordonn‚es de d‚part de la fenˆtre */
	yf = y_desk + 10;
	wf = w_desk / 2;
	hf = h_desk / 2;
	
	if ( index == COOK )
		cook_event( OPEN_DIAL );
	else if ( index == AUTH )
		authen_event( OPEN_DIAL );
	else if ( index == ALERT )
		alerte_event( OPEN_DIAL );
	else
		return(0);

	graf_mouse(ARROW, 0);
	printf("\a");

	do		/* BOUCLE PRINCIPALE DU PROGRAMME */
	{
		event = evnt_multi((MU_MESAG|MU_BUTTON|MU_KEYBD), 258,3,0, /*2, 1, 1,*/
								0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
								buff, 10, 0,
								&mousex, &mousey, &mousek, &key_s, &key, &nbr_clics);

/*	browser->aes_messages( (int*)&buff[0] ); */
		browser->aes_messages( buff );
		wind_get(0, WF_TOP, &wtop, &dummy, &dummy, &dummy); /* R‚mi Vanel */

/* *** Forcer le topped *** */

			if ( (event & MU_MESAG) && ( buff[0] == WM_TOPPED ) )
			{
/* pourquoi ? 	if ( (buff[3] != adr_var.w_handle) && (buff[3] != err_var.w_handle) ) */

				if ( cook_var.w_handle > 0 )
					wind_set( cook_var.w_handle, WF_TOP);
				else
				if ( authen_var.w_handle > 0 )
					wind_set( authen_var.w_handle, WF_TOP);
				else
				if ( alerte_var.w_handle > 0 )
					wind_set( alerte_var.w_handle, WF_TOP);
			}

/* *** Affecter un Handle de fenˆtre aux ‚venements qui en sont d‚pourvus : *** */

			if (event & MU_KEYBD)	/* Gard‚ pour Control+U */
			{
/*			buff[3] = wtop;	*//* ??? */
				keyboard(wtop,key,key_s);
			}
			else if (event & MU_BUTTON)
				buff[3] = wind_find(mousex, mousey);

/* *** Aiguillage en fonction du Handle : *** */

			if (buff[3] > 0)						/* Si W_Handle > 0 .... */
			{
				if (buff[3] == cook_var.w_handle)
					quit = cook_event(event);
				else if (buff[3] == authen_var.w_handle)
					quit = authen_event(event);
				else if (buff[3] == alerte_var.w_handle)
					quit = alerte_event(event);
			}
	}
	while ( quit == 0 );
	
	close_dialog(&cook_var);
	close_dialog(&authen_var);
	close_dialog(&alerte_var);

	form_dial(FMD_FINISH,0,0,0,0,x_desk,y_desk,w_desk,h_desk);

return(quit);
}

LOCAL int cook_event(int event)
{
WINDFORM_VAR *ptr_var = &cook_var;
int choix;

	if (event == OPEN_DIAL)
	{
		fw_attrib =  NAME|MOVER|SMALLER;	/* mettre plus tot ! */
		open_dialog(ptr_var, " CAB.OVL: Accept cookie ? ", 0, TRUE);
	}
	else
	{
		choix = windform_do(ptr_var, event);
		if (choix != 0)
		{
#ifdef _RSC_INT_
			change(ptr_var, choix, NORMAL, 0);
#else
			wf_change(ptr_var, choix, NORMAL, 0);
#endif
			close_dialog(ptr_var);
			return(choix);
		}
	}
return(0);
}

LOCAL int authen_event(int event)
{
WINDFORM_VAR *ptr_var = &authen_var;
int choix;

	if (event == OPEN_DIAL)
	{
		fw_attrib =  NAME|MOVER|SMALLER;	/* mettre plus tot ! */
		open_dialog(ptr_var, " CAB.OVL: Access Restricted ", AUTH_LOGIN, TRUE);
	}
	else
	{
		choix = windform_do(ptr_var, event);
		if (choix != 0)
		{
#ifdef _RSC_INT_
			change(ptr_var, choix, NORMAL, 0);
#else
			wf_change(ptr_var, choix, NORMAL, 0);
#endif
			if( choix == AUTH_VALID )
			{
				*name_password = '\0';
				strncat( name_password, authen_form[AUTH_LOGIN].ob_spec.tedinfo->te_ptext, MAX_STR );
				strncat( name_password, ":", MAX_STR );
				strncat( name_password, authen_form[AUTH_PASSWORD].ob_spec.tedinfo->te_ptext, MAX_STR );
			}
			close_dialog(ptr_var);
			return(choix);
		}
	}
return(0);
}

LOCAL int alerte_event(int event)
{
WINDFORM_VAR *ptr_var = &alerte_var;
int choix;

	if (event == OPEN_DIAL)
	{
		fw_attrib =  NAME|MOVER|SMALLER;	/* mettre plus tot ! */
		open_dialog(ptr_var, titre_alerte, 0, TRUE);
	}
	else
	{
		choix = windform_do(ptr_var, event);
		if (choix != 0)
		{
#ifdef _RSC_INT_
			change(ptr_var, choix, NORMAL, 0);
#else
			wf_change(ptr_var, choix, NORMAL, 0);
#endif
			close_dialog(ptr_var);
			return(choix-ALER_BOUTON1+1);
		}
	}
return(0);
}

INT NextString( INT pere , UBYTE **string)
{
static OBJECT *obj;
static INT old_pere = -1;
static INT index;
static BOOLEAN last = FALSE;

	if ( pere != old_pere )
	{
		rsrc_gaddr(0, pere, &obj);
		index =0;
		old_pere = pere;
		last = FALSE;
	}

	do							/* L'objet Racine n'est pas trait‚... */
	{
		obj++;				/* ... pointe sur l'objet … traiter */
		index++;
		if ((obj->ob_type & 0xff) == G_TEXT)
		{
			if ( last == TRUE ) break;
			if ( obj->ob_next == 0 )	last = TRUE;
			*string = obj->ob_spec.tedinfo->te_ptext;
			return( index );
		}
	} while ( ((obj->ob_flags & LASTOB) == 0) || ( last == FALSE ));

	*string = NULL;
	old_pere = -1;
	
return( -1 );
}

/* ----- Fonctions d'alerte ----- */

INT my_form_alerte( UBYTE *titre, const char *message, ... )
{
UBYTE *token;
INT len, loop;
INT type = 0;
INT attend = ATTEND_BRAKET_DROIT;
va_list	args;

	LOG_printf("> my_form_alerte <");
	LOG_puts(">",titre);
	LOG_puts(">",">\n");

/*	
	if ( config.alertes != TRUE )
	{
		return(-1);	
	}
*/

/* Comme d'habitude le vsprintf est trŠs dangereux ! */
	
	*buffer = '\0';
	va_start(args,message);
	vsprintf(buffer,message,args);
	va_end(args);

	len = (INT)strlen( buffer );
	for( loop = 0; loop < len; loop++)
	{
		if ( ( buffer[loop] == '[' ) && ( attend ==  ATTEND_BRAKET_DROIT ) )
		{
		 buffer[loop] = '\01';
		 attend = ATTEND_BRAKET_GAUCHE;
		}
		else
		if ( ( buffer[loop] == ']' ) && ( attend == ATTEND_BRAKET_GAUCHE ) )
		{
		 buffer[loop] = '\01';
		 attend = ATTEND_BRAKET_DROIT;
		}
	}

	token = strtok(buffer, "\01");
	while ( token != NULL )
	{
		if ( type == 0 )
		{
/*		printf("Trouv‚ icone <%s>\n", token ); */
		}
		else
		if ( type == 1 )
		{
			SetLignes( token );
		}
		else
		if ( type == 2 )
		{
			SetBoutons( token );
		}
		token = strtok(NULL, "\01");
		type++;
	}


	*titre_alerte = '\0';
	strncat( titre_alerte, " CAB.OVL: ", 25 );
	strncat( titre_alerte, titre, 25 );
	
return ( AES_LOOP( ALERT ) );
}

/*
LOCAL VOID SetTitre( UBYTE *chaine )
{
	X_strncat( (UBYTE *)((alerte_form[ALER_TITRE].ob_spec.userblk)->ub_parm) , chaine, LONG_MAX_TITRE);
}
*/

LOCAL VOID SetLignes( UBYTE *chaine )
{
UBYTE *start;
UBYTE *end;
INT len;
BOOLEAN fini = FALSE;
INT ligne = 0;

	len = (INT)strlen( chaine );
	start = chaine;
	do
	{
		end = strchr( start, '|' );
		if ( end == NULL )
		{
			end = &start[len-1];
			fini = TRUE;
		}else	*end = '\0';
		if ( ligne < NBRE_LIGNES )
		{
			X_strncat( alerte_form[ALER_LIGNE1+ligne].ob_spec.tedinfo->te_ptext , start, LONG_MAX_LIGNE);
			ligne++;
		}
		start = end+1;
		if ( start == '\0' ) break;
	}while ( fini == FALSE );

	while( ligne < NBRE_LIGNES )
	{
		X_strncat( alerte_form[ALER_LIGNE1+ligne].ob_spec.tedinfo->te_ptext , "", LONG_MAX_BOUTON);
		ligne++;
	}
	
}

LOCAL VOID SetBoutons( UBYTE *chaine )
{
UBYTE *start;
UBYTE *end;
INT len;
BOOLEAN fini = FALSE;
INT ligne = 0;

	len = (INT)strlen( chaine );
	start = chaine;
	do
	{
		end = strchr( start, '|' );
		if ( end == NULL )
		{
			end = &start[len-1];
			fini = TRUE;
		}
		else	*end = '\0';
		if ( ligne < NBRE_BOUTONS )
		{
			X_strncat( (UBYTE *)((alerte_form[ALER_BOUTON1+ligne].ob_spec.userblk)->ub_parm) , start, LONG_MAX_BOUTON);
			objc_unhide(alerte_form,ALER_BOUTON1+ligne);
			ligne++;
		}
		start = end+1;
		if ( start == '\0' ) break;
	}while ( fini == FALSE );

	while( ligne < NBRE_BOUTONS )
	{
		objc_hide(alerte_form,ALER_BOUTON1+ligne);
		ligne++;
	}
}


UBYTE	*ChaineAlerte( INT index )
{
UBYTE **free_string;

	rsrc_gaddr(R_FRSTR, index, &free_string);

return( *free_string );
}

/* ----- outils ----- */

UBYTE *STinGRessource( INT err )
{

	if (err < 0)    err = -err;

	err = err + M_STING_0;
	
	if (err >  M_STING_DEF )	/* > E_LASTERROR */
	{
		err = M_STING_DEF;
	}
	
	if ( err < M_STING_0 )
	{
		err = M_STING_DEF;
	}

#ifdef notdef

	if (err == 500)
	{
		return (error_func);
	}

#endif
	
return( sting_form[err].ob_spec.tedinfo->te_ptext );
}

LOCAL VOID keyboard(int handle,int key,int key_s)
{
Key	r;

	x_stdkey(key,key_s,&r);

	if ( r.shift & CONTROL )	/* + touche control = raccourcis menu */
	{
		switch(r.ascii2)
		{
			case 'U' :
				if	( handle == cook_var.w_handle )
					close_dialog(&cook_var);
				else if	( handle == authen_var.w_handle )
					close_dialog(&authen_var);
				else if	( handle == alerte_var.w_handle )
					close_dialog(&alerte_var);
				break;
		}
	}
}

LOCAL void x_stdkey( int k, int ks , Key *r )
{
static KEYTAB	*kt=0;

	if (!kt)		kt=Keytbl( (void*)-1, (void*)-1, (void*)-1);
	
	r->shift=(char)(ks);
	r->scan =(char)(k>>8);
	r->std  =(char)(k);
	r->ascii1=toupper(kt->unshift[r->scan]);
	r->ascii2=toupper(kt->shift[r->scan]);
}

/* ---          EOF                 --- */
