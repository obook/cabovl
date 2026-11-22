/*
 * report.c
 * Messages d'erreur et rapports affichÇs sous forme de page HTML.
 *
 * êcrit dans un fichier la page que CAB affiche quand une requàte
 * Çchoue : texte pris dans la ressource, code d'erreur ou texte
 * libre.
 *
 * Utilise : CAB_OVL.H, GLOBALS.H, MAIN.H, RSC.H, TOOLS.H
 * UtilisÇ par : HTTP_GET.C, MAILTO.C, SETUP.C
 *
 * CrÇation : 25/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ultÇrieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<string.h>
#include	<tos.h>
#include	<time.h>
#include	<aes.h>
#include	<portab.h>
#include	<logfile.h>

#include	"globals.h"
#include	"cab_ovl.h"		/* indexes ressource */
#include	"rsc.h"
#include	"tools.h"

#include	<cab\module.h>
#include	"main.h"

/* ---       Includes internes       -- */

#include "report.h"

/* --- Constantes globales internes --- */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

EXTERN OBJECT *report_form;	/* dans rsc.c */
LOCAL UBYTE date[51];
LOCAL ULONG ul_today;

/* nouveau nom en cas d'erreur */

char lecteur1 = 'A', lecteur2 = 'C';
LOCAL UBYTE upfname[] = "12345678";/* doivent OBLIGATOIREMENT garder leur valeur */
LOCAL UBYTE wrkname[] = "12345678";

LOCAL UBYTE HEADER[]="<HTML><HEAD><TITLE>\n\nCAB.OVL Report\n\n</TITLE></HEAD><BODY>\n\n";

/* avant serveur */
LOCAL UBYTE SERVER_START[]="Server: ";

/* apräs serveur */
LOCAL UBYTE SERVER_END[]="<BR>\n";

/* avant URL */
LOCAL UBYTE URL_1_3[]="URL: <A HREF=\"";

/* avant URL */
LOCAL UBYTE URL_2_3[]="\">";

/* apräs URL */
LOCAL UBYTE URL_3_3[]="</A><BR><BR>\n\n";

/* avant texte */
LOCAL UBYTE TEXTE_START[]="<HR><H1><B>\n\n";

/* apräs texte */
LOCAL UBYTE TEXTE_END[]="\n\n</B></H1>";

/* avant version */
LOCAL UBYTE VERSION_START[]="<HR><I>\nCreated by cab.ovl V";

/* avant date */
LOCAL UBYTE DATE_START[]=" ";

/* fin */
LOCAL UBYTE FOOTER[]="\n</I></BODY></HTML>";

/* DÇplacement de l'RL contenant l'erreure */

LOCAL UBYTE	err_URL[1024];

/* ---     Prototypage interne      --- */

LOCAL UBYTE *ChaineRessouce( INT index );
LOCAL INT Write( INT handle, UBYTE *string );
LOCAL UBYTE *NewIndex (UBYTE	*upfname,UBYTE	*wrkname);

/* ---         Fonctions            --- */


/* ++++++++++++++++++ Utilisation de la structure Request ++++++++++++++++++ */

/*

	Retourne la longueur du fichier HTML
	file_name		nom du fichier Ö ouvrir ou NULL
	file_handle	poiteur du fichier ou -1 si on doit se servir de file_name
	code				code d'erreur, index dans r du ressource
	
*/

INT ReportRequestIndex( REQUEST *request, UBYTE *file_name, INT *file_handle, INT rsc_index)
{
	NewIndex (upfname,wrkname);
	*err_URL = '\0';
	strncat( err_URL, "http://cab.ovl/error?", 1024);
	strncat( err_URL, strlwr(wrkname), 1024);
	return ( ReportText( request->URL , file_name, file_handle, ChaineRessouce(rsc_index) ) );
}

INT ReportRequestCode( REQUEST *request, INT code)
{
	return ( ReportRequestText( request, ChaineRessouce(code) ) );
}

INT ReportRequestText( REQUEST *request, UBYTE *texte)
{
INT 	my_handle=-1;
INT 	total = 0;
UBYTE	*new_filename;

	LOG_printf("> ReportRequestText <%s> <%d> <%s>\n", request->filename, request->file_handle, texte);

	if ( request->file_handle > 0 )
	{
		LOG_printf("~ ReportRequest : Fclose <%s>\n", request->filename);
		Fclose( request->file_handle );
		if ( request->cache_exist != TRUE )
		{
			LOG_printf("~ ReportRequest : delete <%s>\n", request->filename);
			Fdelete( request->filename );
		}
		request->file_handle = -1;
	}

#ifdef notdef
	
	/* Si fichier dÇjÖ dans le cache, on ne l'Çcrase pas ! */
	
	if ( request->cache_exist == TRUE )
	{
		LOG_printf("~ ReportRequest : <%s> old cache file is used\n", request->filename);
		return(0);
	}

#endif

#ifdef NOTDEF

	NewIndex (upfname,wrkname);
	*err_URL = '\0';
	strncat( err_URL, "http://cab.ovl/error?", 1024);
	strncat( err_URL, strlwr(wrkname), 1024);

	if (browser->new_url( err_URL, &new_filename) !=0 )
	{
		new_filename = request->filename;
		LOG_printf("! browser can not give a new filename, use default <%s>.\n", new_filename);
	}

	/* Probleme avec AUTOSURF */
	
	if ( *new_filename == '\0' )
		new_filename = TMP_FILE;

#endif

	new_filename = request->filename;
			
	my_handle=(INT)Fcreate( new_filename, 0);
	
	if ( my_handle < 0 )
	{
		LOG_printf("? Couldn't create <%s> <%d>\n", request->filename, my_handle);
		return(-1);
	}
		
	time(&( (time_t)(ul_today) ));
	Strftime(date,50,"%a, %d %b %Y %X",localtime(&((time_t)(ul_today))));

	total = total + Write( my_handle, HEADER );

	total = total + Write( my_handle, SERVER_START );

	total = total + Write( my_handle, request->server );

	total = total + Write( my_handle, SERVER_END );

	total = total + Write( my_handle, URL_1_3 );

	total = total + Write( my_handle, request->URL );

	total = total + Write( my_handle, URL_2_3 );

	total = total + Write( my_handle, request->URI );

	total = total + Write( my_handle, URL_3_3 );

	total = total + Write( my_handle, TEXTE_START);

	total = total + Write( my_handle, texte );

	total = total + Write( my_handle, TEXTE_END);

	total = total + Write( my_handle, VERSION_START);

	total = total + Write( my_handle, ovl_version_str);

	total = total + Write( my_handle, DATE_START);

	total = total + Write( my_handle, date);

	total = total + Write( my_handle, FOOTER );

	Fclose(my_handle);

return(total);
}

/* ++++++++++++++++++ Utilisation de la ressource ++++++++++++++++++ */

INT ReportRscMessage( UBYTE *file_name, INT *file_handle, INT index)
{
	NewIndex (upfname,wrkname);
	*err_URL = '\0';
	strncat( err_URL, "http://cab.ovl/error?", 1024);
	strncat( err_URL, strlwr(wrkname), 1024);
	return ( ReportText( err_URL , file_name, file_handle, ChaineRessouce(index) ) );
}

/* ++++++++++++++++++ Message Simple ++++++++++++++++++ */

INT ReportText( UBYTE *URL, UBYTE *file_name, INT *file_handle, UBYTE *texte)
{
INT		my_handle=-1;
INT		total = 0;
UBYTE	*new_filename;

	LOG_printf("> ReportText <%s> <%d> <%s>\n", file_name, *file_handle, texte);

	if ( *file_handle > 0 )
	{
		Fclose(*file_handle);
		Fdelete( file_name );
		*file_handle = -1;
	}
	
	if (browser->new_url( URL, &new_filename) !=0 )
	{
		new_filename = file_name;
		LOG_printf("! browser can not give a new filename, use default <%s>\n", new_filename);
	}

	if ( *new_filename == '\0' )
		new_filename = TMP_FILE;

	
	my_handle=(INT)Fcreate(new_filename, 0);
	
	if ( my_handle < 0 )
	{
		LOG_printf("? Couldn't create <%s> <%d>\n", new_filename, my_handle);
		return(-1);
	}

	time(&( (time_t)(ul_today) ));
	Strftime(date,50,"%a, %d %b %Y %X",localtime(&((time_t)(ul_today))));

	total = total + Write( my_handle, HEADER );





	total = total + Write( my_handle, URL_1_3 );

	total = total + Write( my_handle, URL );

	total = total + Write( my_handle, URL_2_3 );

	total = total + Write( my_handle, URL );

	total = total + Write( my_handle, URL_3_3 );





	
	total = total + Write( my_handle, TEXTE_START);

	total = total + Write( my_handle, texte );

	total = total + Write( my_handle, TEXTE_END);

	total = total + Write( my_handle, VERSION_START);

	total = total + Write( my_handle, ovl_version_str);

	total = total + Write( my_handle, DATE_START);

	total = total + Write( my_handle, date);

	total = total + Write( my_handle, FOOTER );

	Fclose(my_handle);

return(total);
}

/* ++++++++++++++++++ Outils ++++++++++++++++++ */

LOCAL UBYTE *ChaineRessouce( INT index )
{
	return( report_form[index].ob_spec.tedinfo->te_ptext);
}

LOCAL INT Write( INT handle, UBYTE *string )
{
INT len = (INT)strlen(string);
return( (INT)Fwrite(handle, len, string) );
}

LOCAL UBYTE *NewIndex (UBYTE	*upfname,UBYTE	*wrkname)
{
	if (( lecteur1 == 'A' ) && ( lecteur2 == 'C' ))
	{
		time_t zeit = time( NULL );
		struct tm *t = localtime ( &zeit );
		wrkname[0] = (char)(( t->tm_year % 10 ) + 0x41 );
		wrkname[1] = t->tm_mon + 0x41;
		wrkname[2] = t->tm_mday + 0x41;
		if ( wrkname[2] > 'Z' ) wrkname[2] -= 0x2a;
		wrkname[3] = (char)(( t->tm_hour ) + 0x41 );
		wrkname[4] = (char)(( t->tm_min / 10 ) + 0x41 );
		wrkname[5] = (char)(( t->tm_min % 10 ) + 0x41 );

	}

	if ( ++lecteur2 > 'Z' )
	{
		lecteur2 = 'C';
		if ( ++lecteur1 > 'Z' ) lecteur1 = 'A';
	}

	wrkname[6] = lecteur1;
	wrkname[7] = lecteur2;
	wrkname[8] = '\0';

	*upfname=0;
	strncat( upfname, wrkname ,8);
return ( upfname );
}

/* ---          EOF                 --- */
