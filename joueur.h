/**
*
*  AUTHORS : MANIET Alexandre (amaniet15)(serie 2) , MEUR Damien (dmeur15)(serie 2)
*  This file contains the header and constant necessary for the player program
*
*/

#include "socket.h"
#ifndef JOUEUR_H
#define JOUEUR_H
void signup(int *);//used to signup a client with his name (once connected to the server)
void interrupt_handler(int);//use to catch SIGTERM, SIGQUIT AND SIGINT FOR CLEAN EXIT
void try_to_connect(int *, struct sockaddr_in *);//use to try to connect to server till it worked
void show_cards(card *, int);//used to show an entier deck of cards
void show_card(card cardToShow, char *);//used to show a single card
void init_deck(card *, int);//used to init the deck of the player with cards sent by server
void choose_ecart();//used to ask player to remove 5 cards and sends them to the server for th "ecart"
#endif
