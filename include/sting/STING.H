/*------------------------------------------------
//
// Module:	sting.h
//
// Objet :	Exportation de la librairie sting.lib d'utilisation de STiK/STinG
//					linker avec pctoslib.lib
//
//
// ToDo  : 
//
//
//
// Maintenance :
//
//     Auteur  : Olivier Booklage
//     Version : V1.1
//     Date    : 28/03/1998
//     Remarq. : PremiŠre version
//
//     Auteur  : Olivier Booklage
//     Version : V1.2
//     Date    : 05/04/1998
//     Remarq. : uint32 atoinet(UBYTE *string) : string non detruit
//
//     Auteur  : Olivier Booklage
//     Version : V1.3
//     Date    : 04/06/1998
//     Remarq. : fix_TCP_close forced for TCP.STX version < 1.14
//
//     Auteur  : Olivier Booklage
//     Version : V1.4
//     Date    : 04/08/1998
//     Remarq. : ajout variables STIK_VERSION et STINGLIB_VERSION
//
//-----------------------------------------------*/

#ifndef __STINGLIB_
#define __STINGLIB_

#ifndef __PORTAB__
	#include	<portab.h>
#endif

#ifndef __TOS
	#include	<tos.h>
#endif

#ifndef STING_TRANSPRT_H
	#include	<sting\transprt.h>
#endif

#ifndef STING_PORT_H
	#include	<sting\port.h>
#endif

#ifndef STING_LAYER_H
	#include	<sting\layer.h>
#endif

#ifdef NOTDEF
#ifndef __STINGLIB_INTERN	/* fix TCP_close bug in Version < */
	#define TCP_close	fix_TCP_close
#endif
#endif

/* --- Export Constantes globales   --- */

/* Type of service */

#define TOS_ROUTINE					0
#define TOS_PRIORITY				1
#define TOS_IMMEDIATE				2
#define	TOS_FLASH						3
#define	TOS_FLASH_OVERRIDE	4

/* Standards addresses */

#define	INADDR_ANY				((unsigned long)0x00000000L)
#define	INADDR_BROADCAST	((unsigned long)0xffffffffL)
#define	INADDR_NONE				((unsigned long)0xffffffffL)
#define	INADDR_LOOPBACK		((unsigned long)0x7f000001L)

/* Valeurs retourn‚es par init_network(); */

#define NETWORK_OK				 		 0	/* Initialisation correcte */
#define NETWORK_NOCOOKIE			-1	/* Pas de cookie */
#define NETWORK_NOMAGIC				-2	/* Le nombre magique n'est pas bon */
#define NETWORK_NOTRANSPORT		-3	/* Pas de transport valable pour STiK */
#define NETWORK_NODRIVER			-4	/* Pas de driver STinG pr‚sent */
#define NETWORK_STIK_OK				-4	/* StiK present */
#define NETWORK_NOTACTIVE			-5	/* STinG n'est pas actif */

/* --- Export Structures globales   --- */

#ifdef New_Masquerade_struct /* Masquerade version >= "01.11" only */

typedef struct	masq_port {
  struct port_desc	masq_port_main;		/* as defined upper          */
  struct port_desc	*masq_port_ptr;		/* -> port to be masked      */
	uint32	masq_port_ip;								/*ip number for masking it   */
	uint32	masq_port_ISP_ip;						/*masked IP number (dial-up) */
 } MASQ_PORT;

#endif

/* --- Export Variables globales    --- */

EXTERN UBYTE	*STINGLIB_VERSION;	/* StiK/STinG librarie version Format `00.00' */
EXTERN UBYTE	*STIK_VERSION;			/* StiK/STinG version Format `00.00' */
EXTERN INT		TCP_VERSION;				/* If >= 114, no TCP_ack_wait() before TCP_close() need (old bug) */
EXTERN INT		MASQUE_VERSION;			/* If >= 111, new struct is avalaible */

/* ---    Export Prototypage        --- */

	/* Initialisation de STinG */
	
WORD	init_network( VOID );

	/* Message d'erreur de l'initalisation de STinG */
	
UBYTE *get_network_err( WORD err );

	/* Retourne son adresse en fonction d'un remote , compatible Masquerade >= 1.11 */

uint32 MyIp( uint32 host );

	/* Retourne une chaine correspondant … une adresse IP */

UBYTE *inetoa( uint32 ip );

	/* Retourne l'ip si la chaine represente une adresse, par exemple "194.2.128.1" */

uint32 atoinet( UBYTE *string );

	/* Adresse IP du port masqu‚ par Masquerade ou 0L */
	
int32 MasqueradeIP(VOID);

/* ---          EOF                 --- */
#endif