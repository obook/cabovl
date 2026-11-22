/*=== struct.h =============================================================*/

#ifndef __STRUCT_H_
#define __STRUCT_H_

/*
	typedefs und structs fÅr verschiedene Anwendungen
*/

/*--------------------------------------------------------------------------*/

typedef struct
{
 	int  id;				/* Font-Id			*/
	int  pts;				/* absolute Points	*/

    int  proportional;		/* Flag 			*/

    int  ch_w;				/* max. Fontsize	*/
    int  ch_h;

} vdifont;	

#define _VDIFONT { 0, 0, 0, 0, 0 }

/*--------------------------------------------------------------------------*/
/* 
	Typ-Struktur eines markierten (Text-) Blocks
*/

typedef struct
{
  long start_line;
  long start_x;
  
  long end_line;
  long end_x;	

} txtblk;


/*--------------------------------------------------------------------------*/

/* (Text-) Zeileneintrag	 */

typedef struct	
{
  char * str;			/* Pointer auf den Text */
  char   style;			/* VDI-Textstil 		*/
  char   color;      	/* VDI-Farbe    		*/
  
} tline;

/*--------------------------------------------------------------------------*/
/* 
	Typ-Struktur eines Textes
*/

typedef struct 	
{
  tline		*	line;					/* Zeiger auf Array mit Textzeilen	*/	
  long          len;					/* Arraygrîûe						*/

  long			used;					/* Anzahl benutzter Textzeilen		*/
  
  long			width;					/* Breite des Textes in Character	*/
  
  memdb			ptrsmem;				/* Speicher fÅr Pointerarray		*/	 
							
} txtdb;			

#define _TXTDB {NULL,0,0,0,_MEMDB}

/*--------------------------------------------------------------------------*/

#endif /* __STRUCT_H_ */

/*--- fin ------------------------------------------------------------------*/
