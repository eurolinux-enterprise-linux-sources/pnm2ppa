/***************************************************************************
    text-cs.h  -  contains the czech strings
                             -------------------
    begin                : Thu Jan 13 2000
    copyright            : (C) 2000 by Andrew van der Stock
    email                : ajv@greebo.net
    translator           : Ivo Panacek <ivop@regionet.cz>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

/* messages from cutswath.c */

/* 00 */ "nemohu alokovat dal�� pam�\n",
  
  /* 01 */ "nemohu p�e��st velikost horn�ho okraje\n",
  
  /* 02 */ "nemohu p�e��st velikost spodn�ho okraje\n",
  
  /* 03 */ "nemohu p�e��st dal�� ��dku\n",
  
  /* 04 */ "nemohu alokovat pam� pro ppa\n",
  
  /* 05 */ "chyba p�i vol�n�",   //followed by function name
  
  /* 06 */ "nezn�m� n�vratov� hodnota p�ep�na�e\n",
  
  // from ppa,c

  /* 07 */ "zkracuji data pro trysky na 6 ��dek, (p�vodn� ", // followed by number
  
  // messages from image.c
  
  /* 08 */ "Nemohu vytvo�it gamma.ppm soubor. Zkontrolujte pr�va\n",
  
  /* 09 */ "P�ed�asn� ukon�en� vstup - narazil jsem na EOF p�i �ten� obr�zku:", //followed by format
  
  /* 10 */ "Fat�ln� chyba - nemohu ��st obr�zek\n",
  
  /* 11 */ "Fat�ln� chyba - vstupn� obr�zek obsahuje pixl s hodnotou > 255\n",
  
  /* 12 */ "Fat�ln� chyba! Nemohu alokovat pam� pro buffer!\n",
  
  /* 13 */ "Vstupn� obr�zek nem� podporovan� PNM form�t\n",
  
  /* 14 */  // (removed)
  
  /* 15 */ "Tisk�rna nem��e tisknout na pap�r velikosti ", //followed by size 
  
  /* 16 */ "maxim�ln� hodnota PNM pixlu mus� b�t 255, ale je: ", //followed by value
  
  // messages from ppa.c and pnm2ppa.c
  
  /* 17 */ "Nezn�m� verze tisk�rny\n",   
  
  /* 18 */ "Nemohu alokovat prostor pro ulo�en� komprimovan�ch dat\n",
  
  /* 19 */ "Za��n�m zpracov�vat tiskovou �lohu\n",
  
  /* 20 */ "Tisknu str�nku ", //followed by page number
  
  /* 21 */ "Konec zpracov�n� str�nky ",  //followed by page number
  
  /* 22 */ "��dn� str�nky nebyly vyti�t�ny!\n",
  
  /* 23 */ "Tiskov� �loha dopadla �sp�n�.\n",
  
  /* 24 */ "Upov�dan� v�pis na konzoli\n",
  
  /* 25 */ "Pros�m, nastavte si tisk�rnu v souboru /etc/pnm2ppa.conf\n",
  
  /* 26 */ "nemohu otev��t konfigura�n�ho soubor\n", 

  /* 27 */ "nezn�m� velikost pap�ru\n", 

  /* 28 */ "nezn�m� parametr", // not followed by \n

  /* 29 */ "na ��dce", // followed by line number 

  /* 30 */  "chyba p�i �ten� konfigura�n�ho souboru",  // not followed  by \n

  /* 31 */ "Fat�ln� chyba: nezn�m� argument\n" ,  

  /* 32 */ "Nemohu otev��t v�stupn� PPA soubor\n", 
  
  /* 33 */ "Nemohu otev��t vstupn� PPA soubor\n",

  /* 34 */ "verze a datum p�ekladu:", //followed by version number and date

  /* 35 */ "Color correction curve (Gamma) file exists, but is corrupt.\n",

  /* 36 */ "POSSIBLE BUFFER OVERFLOW ATTACK!!: length of a path specified as an option exceeds", //followed by maximum allowed size

    "FATAL: �patn� chybov� hl�en�\n"

