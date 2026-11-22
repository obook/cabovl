/*
 * mailto.c
 * Envoi d'un message pour les liens mailto:.
 *
 * Ouvre une connexion SMTP, envoie les commandes et le message
 * (Send_Commande, Send_Message, Send_Char) et analyse les codes
 * de r‚ponse du serveur.
 *
 * Utilise : CONFIG.H, GLOBALS.H, MAIN.H, REPORT.H, RSC.H, TCP.H,
 *   TOOLS.H
 * Utilis‚ par : HTTP_GET.C
 *
 * Cr‚ation : 07/10/1999
 * Auteur : Olivier Booklage
 * Date : Janvier 2000 (V5.68)
 * Licence : GNU General Public License version 2 ou ult‚rieure
 * Notes d'origine : voir le commentaire qui suit.
 */

/*------------------------------------------------
//
//	Module	:	mailto.c
//
//	Objet		:
//
//	ToDo		:
//
//	Tab.		:	2
//
//
//
//	Maintenance	:
//
//		Version		:	V1.0
//		Auteur		:	Olivier Booklage
//		Email			:	xxx@xxx.xxx
//		Homepage	:	http://xxx.xxx.xxx/
//		Date			:	07/10/1999
//		Remarques	: detection \r\n.\r\n a refaire plus bas...
//

> 10 Oct 13:54:16 OVL  Send_Message <Site=Bien&clair=Moyen&informations=Oui&Plait=ligne+1&Cherche=ligne+2&Navi=Netscape&navitext=ligne+3>

//
//
//-----------------------------------------------*/


/* ---       Includes externes       -- */

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<time.h>
#include	<tos.h>
#include	<sting\transprt.h>
#include	<portab.h>
#include	<logfile.h>

#include	<cab\module.h>
#include	"globals.h"
#include	"main.h"
#include	"config.h"
#include	"tcp.h"
#include	"rsc.h"
#include	"report.h"
#include	"tools.h"

/* ---       Includes internes       -- */

#include	"mailto.h"

/* --- Constantes globales internes --- */

#define TCP_PORT				25
#define TCP_MAILBUFFER	500
#define TCP_TIMEOUT			5000

#define WAIT_HELO				1
#define WAIT_SEND_OK			2
#define WAIT_RCPT_OK			3
#define SEND_MESSAGE			4
#define WAIT_MSG_ACCEPTED	5
#define SEND_OK				6
#define SEND_ERROR			7

/* --- Structures globales internes --- */

/* --- Variables globales  internes --- */

LOCAL int16	handle;
LOCAL UBYTE *smtp_server;
LOCAL UBYTE *email;
LOCAL	WORD	task;

LOCAL UBYTE *rcpt;
LOCAL UBYTE *sujet;
LOCAL UBYTE *encoding;
LOCAL UBYTE *content;

LOCAL char		x2c(char *what);

/* ---     Prototypage interne      --- */

LOCAL BOOLEAN analyse( INT code, UBYTE* message );
LOCAL INT Send_Commande( BYTE *fmt, ... );
LOCAL INT Send_Message( UBYTE *message );
LOCAL INT Send_Char( UBYTE car );
LOCAL UBYTE *unescape_url(char *url);
LOCAL char x2c(char *what);

/* ---         Fonctions            --- */

long my_mailto(char *dest, char *enctype, char *mes)
{
INT		ret;
uint32	smtp_ip;
int16	bytes_available;
UBYTE buffer[501];
WORD key = 0;
UBYTE *ptr;
time_t timeout;
time_t call_cab;
INT no_data = 3;	/* d‚compte imm‚diat pendant l'attente de reponse */

	rcpt = dest;
	sujet = "From the WEB";
	content = mes;
	encoding = 	enctype;
	task	= WAIT_HELO;

/*

Il se peut que le sujet soit derriere l'email, alors filtrer ex :

	"res1@vcf-com.fr?ici c'est le sujet"
	
*/

	ptr = strchr( rcpt, '?' );
	if ( ptr != NULL )
	{
		*ptr = '\0';
		sujet = ptr+1;
	}

	LOG_printf("> mailto : rcpt     = <%s>\n", rcpt );
	LOG_printf("> mailto : subject  = <%s>\n", sujet );
	LOG_printf("> mailto : enctype  = <%s>\n", encoding );
/*LOG_printf("> mailto : filename = <%s>\n", filename );*/
	
	email=X_getvstr("EMAIL");
	if ( email[1] == '\0' && (email[0] == '0' || email[0] == '1') )
	{
		my_form_alerte("Mailto","[1][Can not find|\"EMAIL=\"|variable in your system][ Ok ]");
		return(-1L);
	}

	smtp_server=X_getvstr("SMTP_SERVER");
	if ( smtp_server[1] == '\0' && (smtp_server[0] == '0' || smtp_server[0] == '1') )
	{
		my_form_alerte("Mailto","[1][Can not find|\"SMTP_SERVER=\"|variable in your system][ Ok ]");
		return(-1L);
	}

	browser->msg_status(STATUS_RESOLVING_HOST, 0);

	ret = X_resolve (smtp_server, NULL, &smtp_ip, 1);
	if ( ret < 0 )
	{
		browser->msg_error(CANNOT_RESOLVE);
		my_form_alerte("Mailto","[1][resolve error on|%s|%s][ Ok ]", smtp_server, STinGRessource( ret ) );
		return(-1L);
	}

	browser->msg_status(STATUS_CONNECTING_HOST, 0);
	
	handle = X_TCP_open( smtp_ip, TCP_PORT, 0, TCP_MAILBUFFER+1);
	if ( handle < 0 )
	{
		my_form_alerte("Mailto","[1][Network error on|%s|%s][ Ok ]", smtp_server, STinGRessource( handle ) );
		return(-1L);
	}

	browser->msg_status(STATUS_SENDING_REQUEST, 0);
	
	if ( Send_Commande( "helo [127.0.0.1]\r\n" ) < 0 )
	{
		return(-1L);
	}

	LOG_printf("~ Wait SMTP server <%s> reponse....\n", smtp_server);

	timeout	= clock()+config.timeout*CLK_TCK;
	call_cab=clock()+(config.gem/1000)*CLK_TCK;		/* appel CAB toutes les secondes */
			
	do
	{
		bytes_available = X_CNbyte_count(handle);
		if ( bytes_available > 0)
		{
			ret = X_CNget_block(handle,buffer,bytes_available);
			if ( ret > 0 )
			{
				timeout	= clock()+config.timeout*CLK_TCK;
				buffer[ret]=0;
				/* Pour la presentation */
				ptr = buffer+strlen(buffer)-1;
				while( ( ptr > buffer) && (( *ptr == '\r' ) || ( *ptr == '\n' )) )
				{
					*ptr = '\0';
					ptr--;
				}
				LOG_printf("~ receive %d bytes <%s>\n",ret, buffer);
				buffer[3]='\0';
				no_data = 0;
				if ( analyse( atoi(buffer), &buffer[4] ) != TRUE )
				{
					key = 27;
					break;
				}
			}
			else if ( ret < 0 ) /* pour iconnect */
			{
				my_form_alerte("Mailto","[1][Network error on|%s|%s][ Ok ]", smtp_server, STinGRessource( ret ) );
				break;
			}
		}
		else if ( bytes_available < 0 ) /* pour iconnect */
		{
			my_form_alerte("Mailto","[1][Network error on|%s|%s][ Ok ]", smtp_server, STinGRessource( bytes_available ) );
			break;
		}
		
		/* Timeout */
		
		if ( clock() > timeout )
		{
			LOG_printf("~ Timeout!\n");
			browser->msg_error(CONNECTION_FAILED);
			my_form_alerte("Mailto","[1][Server|%s|%s][ Ok ]", smtp_server, STinGRessource( E_CNTIMEOUT ) );
			key = 27;
			task = SEND_ERROR;
		}

		/* Temps pour CAB + touche STOP + touche ESC */

		if ( (clock() > call_cab) && (bytes_available == 0) )
		{
			if ( browser->aes_events(config.timer) == -1L )	/* bouton STOP ou touche Esc */
			{
				LOG_printf("! aes_events : STOP icon or Esc key pressed\n");
				/* s'affiche pas :( */
				browser->msg_error(USERBREAK);	/* histoire de donner quelque chose */
				key = 27;
				task = SEND_ERROR;
			}

			call_cab=clock()+(config.gem/1000)*CLK_TCK;		/* appel CAB toutes les secondes */
			
			if ( no_data == 3 )
			{
				browser->msg_status(STATUS_WAITINGFORDATA,(timeout-clock())/200);
			}else if ( no_data < 3)	no_data ++;

		}
	}
	while ( key != 27 );
	
	if ( Send_Commande("quit\r\n") < 0 )
		X_TCP_close( handle, 0 );
	else
		X_TCP_close( handle, 3 );

	if(	task == SEND_OK )
	{
	INT file_handle = -1;
	
/*	my_form_alerte("Mailto","[1][Your mail is Sended][ Ok ]"); */
		LOG_printf("~ mailto return 0L\n");
		ReportText( "http://cab.ovl?mailto", "dummy.txt", &file_handle, "Your mail is send!");
	 	return(0L);
	}

	LOG_printf("~ mailto return an error\n");
return(-1L);
}

LOCAL BOOLEAN analyse( INT code, UBYTE* message )
{
long ul_today;
UBYTE str_stamp[81];

	switch (task)
	{
		case WAIT_HELO:

		if ( code == 250 )
		{
			if ( Send_Commande("mail from:<%s>\r\n", email) < 0 )
				return( FALSE );
		 task = WAIT_SEND_OK;
		 return(TRUE);
		}
		else if ( code == 220 )
		{
			return(TRUE);
		}
		else
		{
			LOG_printf("! %s\n", message);
			my_form_alerte(smtp_server,"[1][%s][ Ok ]", message );
			return(FALSE);
		}

	
		case WAIT_SEND_OK:

		if ( code == 250 )
		{
			if ( Send_Commande("rcpt to:<%s>\r\n", rcpt) < 0 )
				return(FALSE);
		 task = WAIT_RCPT_OK;
		 return(TRUE);
		}
		else
		{
			LOG_printf("! %s\n", message);
			my_form_alerte(smtp_server,"[1][%s:|%s][ Ok ]", email, message );
			return(FALSE);
		}
		
		case WAIT_RCPT_OK:

		if ( code == 250 )
		{
			if ( Send_Commande("data\r\n") < 0 )
				return(FALSE);
			task = SEND_MESSAGE;
			return(TRUE);
		}
		else
		{
			LOG_printf("! %s\n", message);
			my_form_alerte(smtp_server,"[1][%s:|%s][ Ok ]", rcpt, message );
			return(FALSE);
		}


		case SEND_MESSAGE:

		if ( code == 354 )
		{

			time(&( (time_t)(ul_today) ));
			Strftime(str_stamp,80,"%d %b %Y %X GMT",localtime(&((time_t)(ul_today))));

			if ( Send_Commande("Date: %s\r\n", str_stamp) < 0 )
			{
				return(FALSE);
			}
				if ( Send_Commande("From: %s\r\n", email) < 0 )
			{
				return(FALSE);
			}

			if ( Send_Commande("To: %s\r\n", rcpt) < 0 )
			{
				return(FALSE);
			}

			if ( Send_Commande("Subject: %s\r\n\r\n", sujet) < 0 )
			{
				return(FALSE);
			}
			
			if ( Send_Message( content ) < 0 )
			{
				return(FALSE);
			}
			else
			{
			 task = WAIT_MSG_ACCEPTED;
			 return(TRUE);
			}
		}
		else
		{
			LOG_printf("! %s\n", message);
			my_form_alerte(smtp_server,"[1][error in your mail:|%s][ Ok ]", message );
			return(FALSE);
		}


		case WAIT_MSG_ACCEPTED:

		if ( code == 250 )
		{
			LOG_printf("~ Message sended!\n");
			task = SEND_OK;
			return(FALSE);
		}
		else
		{
			LOG_printf("! %s\n", message);
			my_form_alerte(smtp_server,"[1][error in your mail:|%s][ Ok ]", message );
			task = SEND_ERROR;
			return(FALSE);
		}
	}

return(TRUE);
}

LOCAL INT Send_Commande( BYTE *fmt, ... )
{
INT ret;
va_list	args;
UBYTE buffer[501];
UBYTE *ptr;
time_t	timeout;

	va_start(args,fmt);
	vsprintf(buffer,fmt,args);
	va_end(args);

	timeout	= clock()+TCP_TIMEOUT*CLK_TCK;
	ret = E_OBUFFULL;
	while ( ret == E_OBUFFULL )
	{
		ret = X_TCP_send( handle, buffer, (INT)strlen(buffer) );
		if ( clock() > timeout )
			ret = E_CNTIMEOUT;
	}
	
	/* Pour la presentation */
	ptr = buffer+strlen(buffer)-1;
	while( ( ptr > buffer) && (( *ptr == '\r' ) || ( *ptr == '\n' )) )
	{
		*ptr = '\0';
		ptr--;
	}

	LOG_printf("> Send_Commande <%s> : \"%s\"\n", smtp_server, buffer);

	if ( ret < 0 )
	{
		LOG_printf("! TCP_send <%s> error <%s>\n", smtp_server, get_err_text(ret) );
		browser->msg_error(NETWORK_ERROR);
		my_form_alerte("Mailto","[1][Server|%s|%s][ Ok ]", smtp_server, STinGRessource( ret ) );
	}
return(ret);
}

LOCAL INT Send_Message( UBYTE *message )
{
int16 ret;
UBYTE *ptr = message;
UBYTE *end = message+strlen(message)-1;
BOOLEAN detect_point = FALSE;
BOOLEAN decodage = FALSE;
time_t	timeout;

	LOG_printf("> Send_Message <%s>\n", message);
	
	if ( strnicmp( encoding, "application/x-www-form-urlencoded", 35 ) == SUCCESS )
	{
		end = unescape_url( message );
		decodage = TRUE; 
	}

	while( ptr < end )
	{
	
/*

decodage :

*/
		if ( decodage == TRUE )
		{
			if ( *ptr == '+' )	*ptr = ' ';
			else
			if ( *ptr == '&' )
			{
				ret = Send_Char( '\r' );
				if ( ret < 0 ) return( ret );
				ret = Send_Char( '\n' );
				if ( ret < 0 ) return( ret );
				ptr++;
				if ( ptr > end ) return( 0 );
			}
		}
		
		if ( (*ptr == '.')
		 &&
		  (   (*(ptr+1) == '\r') || (*(ptr+1) == '\n') )
	   &&
	    (   (*(ptr-1) == '\r') || (*(ptr-1) == '\n') )	   )
		{
			detect_point = TRUE;
		}

		timeout	= clock()+TCP_TIMEOUT*CLK_TCK;
		ret = E_OBUFFULL;
		while ( ret == E_OBUFFULL )
		{
			ret = X_TCP_send( handle, ptr, 1 );
			if ( clock() > timeout )
				ret = E_CNTIMEOUT;
		}

		if ( ret < 0 )
		{
			LOG_printf("! TCP_send <%s> error <%s>\n", smtp_server, get_err_text(ret) );
			browser->msg_error(NETWORK_ERROR);
			my_form_alerte("Mailto","[1][Server|%s|%s][ Ok ]", smtp_server, STinGRessource( ret ) );
			return(ret);
		}
		
		ptr++;
		if ( detect_point == TRUE )
		{
			ret = Send_Char( ' ' );
			if ( ret < 0 ) return( ret );
			detect_point = FALSE;
		}

	}

	if ( Send_Commande("\r\n.\r\n") < 0 )
	{
		return(FALSE);
	}

return(0);
}

/*

Faire boucle E_OBUFFULL avec timeout

*/

LOCAL INT Send_Char( UBYTE car )
{
int16 ret;
time_t timeout;

	LOG_printf("> Send_Char <%d>\n", car);

	timeout	= clock()+TCP_TIMEOUT*CLK_TCK;
	ret =	E_OBUFFULL	;
	while ( ret == E_OBUFFULL )
	{
		ret = X_TCP_send( handle, &car, 1 );
		if ( clock() > timeout )
			ret = E_CNTIMEOUT;
	}

	if ( ret < 0 )
	{
		LOG_printf("! X_TCP_send <%s> error <%s>\n", smtp_server, get_err_text(ret) );
		browser->msg_error(NETWORK_ERROR);
		my_form_alerte("Mailto","[1][Server|%s|%s][ Ok ]", smtp_server, STinGRessource( ret ) );
	}
		
return(ret);
}

/*


Decodage

*/

LOCAL UBYTE *unescape_url(char *url)
{
register int x,y;

	for(x=0,y=0;url[y];++x,++y)
	{
	 if((url[x] = url[y]) == '%')
	 {
		 url[x] = x2c(&url[y+1]);
		 y+=2;
	 }
	}
 	url[x] = '\0';

return( &url[x] );
}
/* son outils : */

LOCAL char x2c(char *what)
{
		register char digit;

		digit = (what[0] >= 'A' ? ((what[0] & 0xdf) - 'A')+10 : (what[0] - '0'));
		digit *= 16;
		digit += (what[1] >= 'A' ? ((what[1] & 0xdf) - 'A')+10 : (what[1] - '0'));
		return(digit);
}

/* ---          EOF                 --- */
