/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the header and constant necessary for the player program
*
*/

#include "socket.h"
#ifndef JOUEUR_H
#define JOUEUR_H
void signup(int *);
void interrupt_handler(int);
void try_to_connect(int *, struct sockaddr_in *);
void show_cards(card *, int);
void show_card(card cardToShow, char *);
void init_deck(card *, int);
void choose_ecart();
#endif
