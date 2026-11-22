/*
 * header.c
 * Analyse de l'en-tˆte HTTP de la r‚ponse.
 *
 * HeaderIsReady d‚tecte la fin de l'en-tˆte dans le tampon re‡u.
 * parse_header d‚coupe les lignes (MakeHeadlist), repŠre les
 * champs (IsHeadline) et remplit la structure REQUEST.
 *
 * Utilise : COOKIE.H, GLOBALS.H, TCP.H, TOOLS.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 17/03/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 */

/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<tos.h>
#include	<upoint/memory.h>
#include	<portab.h>
#include	<logfile.h>
#include	<cab\module.h>
#include	<sting\transprt.h>		/* Pour  TESTABLISH */

#include	"globals.h"
#include	"tcp.h"
#include	"cookie.h"
#include	"tools.h"

/* ---       Includes internes       -- */

#include "header.h"

/* --- Constantes globales internes --- */

#define LINES 200		/* devrait ˆtre r‚glable par soft */

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

/* ---     Prototypage interne      --- */

/* ---         Fonctions            --- */

LOCAL long MakeHeadlist ( char *start, txtdb *lst, memdb *mem );

/* ----- R‚gle et parse le header ----- */

WORD HeaderIsReady( REQUEST *request, memdb *header)
{
INT i;
INT retour;
LONG headlength;
memdb	ptrsmem= _MEMDB;
txtdb		textlst	= _TXTDB;	/* Structure de tableau de lignes de texte */

	LOG_printf("> HeaderIsReady ? on header buf of %ld bytes\n", header->used);

	if ( header->used <= 0L )	return(HEADER_EMPTY);

#ifdef notdef

	for ( i = 0; i < header->used; i++ )
	{
LOG_printf("~ caractere numero %d : %d <%c>\n",i,((UBYTE*)header->beg)[i],((UBYTE*)header->beg)[i]);
	}

#endif

	/* Le header est arriv‚ : */
	
	if (	(strstr( (UBYTE*)header->beg , "\r\n\r\n" ) != NULL) ||
				(strstr( (UBYTE*)header->beg , "\n\r\n\r" ) != NULL) ||
				(strstr( (UBYTE*)header->beg , "\n\n" ) != NULL) ||
				(strstr( (UBYTE*)header->beg , "\r\r" ) != NULL) )
	{

		LOG_printf("~ HeaderIsReady: Header complete\n");
		
#ifdef notdef

		if ( strstr( (UBYTE*)header->beg , "\r\n\r\n" ) != NULL )
			LOG_printf("~ This server use \\r\\n\\r\\n endofline\n");
		else
		if ( strstr( (UBYTE*)header->beg , "\n\r\n\r" ) != NULL )
			LOG_printf("~ This server use \\n\\r\\n\\r endofline\n");
		else
		if ( strstr( (UBYTE*)header->beg , "\n\n" ) != NULL )
			LOG_printf("~ This server use \\n\\n endofline\n");
		else
		if ( strstr( (UBYTE*)header->beg , "\r\r" ) != NULL )
			LOG_printf("~ This server use \\r\\r endofline\n");
		else
			LOG_printf("~ This server use ???? imposible endofline!?\n");
#endif		
		
		/* M‚moire pour le tableau de pointeurs */

		if( !NewMemBlk(&ptrsmem,LINES,sizeof(tline)) )
		{
			LOG_printf("! HeaderIsReady: NewMemBlk error, low memory.\n");
			FreeMemBlk(&ptrsmem);
			return(HEADER_LOW_MEMORY);
		}

		textlst.line 		= ptrsmem.beg;
		textlst.len			= LINES;

		/* D‚coupage de l'entˆte */

		if ( (headlength=MakeHeadlist ((UBYTE*)header->beg, &textlst,header)) <=0 )
		{

			LOG_printf("! HeaderIsReady: No MakeHeadlist in <\n");
			LOG_puts("!",(UBYTE*)header->beg);
			LOG_puts("!","\n>\n");

			FreeMemBlk(&ptrsmem);
			return(HEADER_BAD_FORMAT);
		}

		LOG_printf("~ MakeHeadlist count %ld lines (0 to %ld) in %ld bytes of header\n",textlst.used,textlst.used-1, headlength);

		for(i=0;i<textlst.used;i++)
		{
			LOG_printf("~ MakeHeadlist Line (%d):<",i);
			LOG_puts  ("~",clearwhite(textlst.line[i].str));
			LOG_puts  ("~",">\n");
		}

		retour=parse_header( request, &textlst);

		/* d‚placer les donn‚es */
	
/*	LOG_printf("~ reste %ld - %ld = %ld data apres le header\n",header->used,headlength,header->used-headlength); */

		header->used = header->used - headlength;
		memcpy(header->beg, (UBYTE*)header->beg + headlength , header->used);

		FreeMemBlk(&ptrsmem);

		if ( retour == FALSE )
		{
			LOG_printf("! parse_header error <%d>\n", retour);
			return( HEADER_BAD_FORMAT );
		}

		return( retour ); /* status HTTP */
	}

return(WAITING_HEADER);
}



/* ---------------------------------------------------------------------- */



/* Version originale de UPOINT, celle de weblight ne marche pas */

LOCAL long MakeHeadlist ( char *start, txtdb *lst, memdb *mem )	
{
  char *c = start;
  long  j = 0;

  
/*
  assert ( lst->len  >  0 );
  assert ( lst->line != NULL );
*/
    
  lst->used = 0;
    
  
  if (( c >= mem->beg ) &&  
      ( c <  mem->end ) &&  
      ( lst->len > 0  )) {
  
    lst->line[j++].str = c;				/* erste Zeile				 */									
    	
    while ( c <= mem->end ) {				/* Pointerirrlauf verhindern */
    				
      if ( *c == '\x0a' ) {					/* Zeilenenden in \0 wandeln */
      					
        *c++ = '\0';
		
		if ( c <= mem->end ) {
		
			if ( j < lst->len )				/* n„chster Zeilenbegin 	 */
          	  lst->line[j++].str = c;

 											/* CR/LF */
        	if ( c < mem->end )
	          if (( *c == '\x0d' ) && ( *(c+1) == '\x0a' )) 
         		 *c++ = '\0';  
        
        	if ( *c == '\x0A' ) {			/* zweites LF */
           
          		*c++ = '\0';
           	
           		lst->used = j;				
          									/* Kopfl„nge zurckgeben */
          					
          		return ((long)( c - start ));
        	}
       
        	if ( j == lst->len )			/* Textpointerliste voll 		*/	
          		return FALSE;
		}
      }
      else
         c++;      
    }
  }

  /* Kopf konnte nicht ... 	*/
  
  return 0;
}

int parse_header( REQUEST *request ,txtdb *textlst )
{
LONG j;
UBYTE *c,*p;
UBYTE tmp[4];
INT return_code=0;
double http_version=0;

	LOG_printf("> parse_header\n");

	clear_info(request);

	if (( j = IsHeadline ( "HTTP/", textlst )) >= 0 )
	{
 		*request->http_line = '\0';
 		strncat( request->http_line, clearwhite ( &textlst->line[j].str[9] ), 255 );
		LOG_printf("~ HTTP/ = <%s>\n", request->http_line);
 		*tmp = '\0';
		http_version=atof(strncat(tmp, &textlst->line[j].str[5], 3));
		if ( (http_version==1.0) || (http_version==1.1) )
		{
	 		*tmp = '\0';
			return_code=atoi(strncat(tmp, &textlst->line[j].str[9], 3));
			if ( return_code >= 400 )
				LOG_printf("! HTTP error <%s>\n", request->http_line);
		}
		else
		{
			LOG_printf("! HTTP/ is not 1.0 or 1.1 in <%s>\n",&textlst->line[j]);
			return_code=FALSE;
		}
	}

	if (( j = IsHeadline ( "Content-Length:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[15] );
		request->content_length = atol( c );
 		LOG_printf("~ Content-Length = <%ld>\n", request->content_length);
	}

	if (( j = IsHeadline ( "Content-Encoding:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[17] );
 		*request->content_encoding = '\0';
		strncpy(request->content_encoding, c, 255);
		LOG_printf("~ Content-Encoding = <%s>\n", request->content_encoding);
	}

	if (( j = IsHeadline ( "Content-Type:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[13] );
 		if ( (p=strchr( c, ';' )) != NULL )	*p = '\0';
 		*request->content_type = '\0';
		strncpy(request->content_type, c, 255);
		LOG_printf("~ Content-Type = <%s>\n", request->content_type);
	}

	if (( j = IsHeadline ( "Location:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[9] );
 		*request->location = '\0';
		strncpy(request->location, c, 255);
		LOG_printf("~ Location = <");
		LOG_puts("~",request->location);
		LOG_puts("~",">\n");
	}

	if (( j = IsHeadline ( "MIME-Version:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[13] );
		request->mime_version=atof(c);
		LOG_printf("~ MIME-Version = <%f.1>\n", request->mime_version);
	}

	if (( j = IsHeadline ( "Pragma:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[7] );
 		*request->pragma = '\0';
		strncpy(request->pragma, c, 255);
		LOG_printf("~ Pragma = %s\n", request->pragma);
	}

	if (( j = IsHeadline ( "Server:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[7] );
 		*request->host_server = '\0';
		strncpy(request->host_server, c, 255);
		LOG_printf("~ Server = %s\n", request->host_server);
	}

	if (( j = IsHeadline ( "WWW-Authenticate:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[16] );
 		*request->www_authenticate = '\0';
		strncpy(request->www_authenticate, c, 255);
		LOG_printf("~ WWW-Authenticate = %s\n", request->www_authenticate);
	}

	if (( j = IsHeadline ( "Last-Modified:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[14] );
 		request->last_modified = parse_date(c);
		LOG_printf("~ Last-Modified = %s (%ld)\n", c, request->last_modified);
	}

	if (( j = IsHeadline ( "Date:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[5] );
		request->date=parse_date(c);
		LOG_printf("~ Date = %s (%ld)\n", c, request->date);
	}

	if (( j = IsHeadline ( "Expires:", textlst )) >= 0 )
	{
 		c = clearwhite ( &textlst->line[j].str[8] );
		request->expires=parse_date(c);
		LOG_printf("~ Detect 'Expires' = %s (%ld)\n", c, request->expires);
	}

	for ( j=0; j < textlst->used; j++ )
	{
		if ( textlst->line[j].str[0] != '\0' )
		{
			if ( !strnicmp ( "Set-cookie:", textlst->line[j].str, 11))
			{
		 		c = clearwhite ( &textlst->line[j].str[11] );
				LOG_printf("~ Detect 'Set-cookie' = <");
				LOG_puts("~", c);
				LOG_puts("~", ">\n");
				AddCookie( request, c );
			}
		}
	}
	
	LOG_printf("~ parse_header return(%d)\n",return_code);

return(return_code);
}

/* ----- Outils UPOINT ----- */

/*
	IsHeadline prft, ob im Messagehead eine Zeile vorkommt, die
	mit dem bergebenen String (non-casesensitive) beginnt.
	Headende wird durch Leerzeile erkannt.

	šbergabe: Headzeile, Text
	Rckgabe: Zeilenindex oder -1
*/

long IsHeadline ( const char *s, const txtdb *list )
{
long	i;

/*LOG_printf("> IsHeadline\n");*/

/*
	assert ( s && list );
*/

	for ( i=0; i < list->used; i++ )
	{
		if ( list->line[i].str[0] == '\0' )  										break;
		if ( !strnicmp ( s, list->line[i].str, strlen ( s ))) 	return i;
	}

return(-1);
}

/* ---          EOF                 --- */
