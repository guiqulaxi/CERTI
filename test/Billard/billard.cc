// -*- mode:C++ ; tab-width:4 ; c-basic-offset:4 ; indent-tabs-mode:nil -*-
// ---------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002, 2003  ONERA
//
// This file is part of CERTI
//
// CERTI is free software ; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation ; either version 2 of the License, or
// (at your option) any later version.
//
// CERTI is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
//
// $Id: billard.cc,v 3.7 2003/01/31 10:47:52 breholee Exp $
// ---------------------------------------------------------------------------

#include <config.h>

// Entetes systeme
#include <cstdio>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>

#include <iostream>
#include <signal.h>
#include <exception>

// Entetes RTI
#include "RTI.hh"
#include "PrettyDebug.hh"

// Entetes specifiques
#include "bille.hh" // Contient la description de la classe Bille
#include "my_fed.hh" // Contient la classe Fed, surcharge de FederateAmbassador
#include "constants.hh" // Les constantes de classes, attributs, etc.
//#include "fedtime.hh"

#ifdef TEST_USES_GRAPHICS
#include "graph_c.hh"
#endif

using namespace std ;

// Definition des constantes

#define HEURE_DEB 10.0
#define TEMPS_SIMU_F 60 // Duree de la simulation en secondes
#define TEMPS_SIMU_C 60 // Duree de la simulation en secondes
// j'ai diff�renci� le temps suivant si on est ou pas cr�ateur: utilis�
// seulement pour faire des test

const RTIfedTime TIME_STEP(1.0); // lookahead

#define XMAX 500 /* dimensions fenetre graphique */
#define YMAX 100
#define XFEN 650 /* position de cette fenetre */
#define YFEN 70
#define TMAX 30

// ------------------
// -- Declarations --
// ------------------

// Declaration des fonctions externes
extern void InitGraphe(int X, int Y, unsigned WIDHT, unsigned HEIGHT);

// Declaration des objets et variables globaux

RTI::RTIambassador *rtiamb = new RTI::RTIambassador();
Fed *fedamb = new Fed(rtiamb);

static pdCDebug D("HERITAGE", "(main) - ");

bool exit_billard ;
int nbtick=0 ;
unsigned int t, s ;
char c ;

int YOFFSET = 0 ;

// ----------------------------------------
// -- Declaration des fonctions internes --
// ----------------------------------------
extern "C" void sortir(int SignalNumber);
void ExceptionHandler(void);
void InitialisationGraphique(FederateHandle myFederateID);
void SetTimeRegulation(RTI::RTIambassador *rtiamb,
                       bool creator,
                       FedTime& localTime);
void Synchronize(RTI::RTIambassador *rtiamb,
                 Fed *fedamb,
                 bool creator);

// ---------------------------------------------------------------------------
//! Test program entry point.
int
main(int argc, char **argv)
{
    int i = 0 ; // Variable de boucle
    RTIfedTime localTime(0.0); // Temps local du federe
    RTIfedTime* time_aux ;
    FederateHandle myFederateID = 0 ; // Mon numero de federe
    bool creator = false ; // Suis-je le Createur ?
    char *FederationName ;
    char *FederateName ;
    char *DotFedFile ;

    printf("CERTI-Billard " VERSION " - Copyright 2002 ONERA\n");
    printf("This is free software ; see the source for copying conditions. "
           "There is NO\n");
    printf("warranty ; not even for MERCHANTABILITY or FITNESS FOR A "
           "PARTICULAR PURPOSE.\n\n");

    if (argc != 4 && argc != 5) {
        cerr<<"usage: "<<argv[0]<<" <nom_federe> <coordonne on/off> "
            "<nom_federation> [<y_display_offset>]\n" ;
        exit(-1);
    }

    // Handlers
    std::signal(SIGINT, sortir);
    std::signal(SIGALRM, sortir);

    set_terminate(ExceptionHandler);
    set_unexpected(ExceptionHandler);

    // Nom de la f�d�ration
    FederationName = argv[3] ;
    DotFedFile = new char[strlen(FederationName)+5] ;
    strcpy(DotFedFile, FederationName);
#ifdef HAVE_XML // if compiled with XML, use the .xml description
    strcat(DotFedFile, ".xml");
#else
    strcat(DotFedFile, ".fed");
#endif
    printf("Using %s file\n", DotFedFile);

    FederateName = argv[1] ;

    // Verifier que la federation existe
    try {
        rtiamb->createFederationExecution(FederationName,
                                          DotFedFile);
        D.Out(pdInit, "Federation execution created.");
        creator = true ;
    }
    catch (FederationExecutionAlreadyExists& e) {
        D.Out(pdInit, "Federation execution already created.");
    }


    // Participer a la federation
    D.Out(pdInit, "Federate %s attempting to join the %s federation.",
          FederateName, FederationName);

    bool joined = false ;
    // int numTries = 0 ;

    while (!joined) {
        try {
            myFederateID=rtiamb->joinFederationExecution(FederateName,
                                                         FederationName,
                                                         fedamb);
            joined = true ;
            D.Out(pdInit, "Federate %s joined the %s : I'm #%d .",
                  argv[1], FederationName, myFederateID);
            break ;
        }
        catch (FederateAlreadyExecutionMember& e) {
            D.Out(pdExcept,
                  "Federate %s already exists in the Federation Execution.",
                  argv[1]);
            throw ;
        }
        catch (FederationExecutionDoesNotExist& e) {
            D.Out(pdExcept, "Federate %s : FederationExecutionDoesNotExist.",
                  argv[1]);
            // sleep(1);
        }
        catch (Exception& e) {
            D.Out(pdExcept,
                  "Federate %s :Join Federation Execution failed : %d .",
                  argv[1], &e);
            throw ;
        }
    }

    if (argc==5)
        YOFFSET = atoi(argv[4]);
    else
        YOFFSET = YMAX*(myFederateID-1)+25*myFederateID ;

    // Le createur met la federation en pause.

    if (creator) {
        D.Out(pdInit, "Pause requested");
        try {
            rtiamb->registerFederationSynchronizationPoint("Freeze",
                                                           "OK");
            fedamb->paused = true ;
            strcpy(fedamb->CurrentPauseLabel, "Freeze");
        }
        catch (Exception& e) {
            D.Out(pdExcept, "Federate #%s : Register Federation "
                  "Synchronization Point failed : %d .", argv[1], &e);
        }
    }

    // Publication et abonnement
    D.Out(pdInit, "Federate #%s publishinng and subscribing objects.", argv[1]);
    fedamb->PublishAndsubscribe();

    // Waiting for callbacks
    rtiamb->tick(1.0, 2.0);
    nbtick++ ;

    // --------------------
    // -- Initialization --
    // --------------------
#ifdef TEST_USES_GRAPHICS
    // Ouvrir fenetre graphique
    InitialisationGraphique(myFederateID);
#endif

    // Mettre en route la regulation, se declarer contraint etc.
    if (strcmp(argv[2], "on") == 0) {
        SetTimeRegulation(rtiamb, creator, localTime);

        // Waiting for callbacks
        try {
            rtiamb->tick(1.0, 2.0);
            nbtick++ ;
        }
        catch (Exception& e) {
            D.Out(pdExcept, "Exception ticking the RTI : %d", &e);
        }
    }

    // Phase de synchronisation initiale.
    Synchronize(rtiamb, fedamb, creator);
    D.Out(pdInit, "Initial synchronization done.");

    // Creer ma boule
    fedamb->Local.Initialiser(myFederateID);
    D.Out(pdTrace, "creation de la boule r�ussie.");

    // Declarer la boule aux autres federes
    fedamb->RegisterObjects();
    D.Out(pdInit, "Local Object registered under ID %d", fedamb->Local.ID);
    rtiamb->queryFederateTime(localTime);

    // ------------------------------
    // -- Boucle de la simulation --
    // ------------------------------
    D.Out(pdTrace, "Debut de la boucle de simulation.");
    D.Out(pdTrace, "L'objet local a pour ID %d.", fedamb->Local.ID);

    exit_billard = false ;
    while (!exit_billard) {
        // Advance Time
        // localTime = rtiamb->requestFederateTime();
        fedamb->granted = false ;
        rtiamb->queryFederateTime(localTime);

        try {
            time_aux=new RTIfedTime(localTime.getTime()+TIME_STEP.getTime());

            D.Out(pdDebug, "time_aux : %.2f - localtime : %.2f - "
                  "timestep : %.2f", time_aux->getTime(),
                  ((RTIfedTime&)localTime).getTime(),
                  ((RTIfedTime&)TIME_STEP).getTime());
            rtiamb->timeAdvanceRequest(*time_aux);
        }
        catch (Exception& e) {
            D.Out(pdExcept, "******* Exception sur timeAdvanceRequest.");
        }
        delete time_aux ;

        while (!fedamb->granted) {
            try {
                rtiamb->tick();
                nbtick++ ;
            }
            catch (Exception& e) {
                D.Out(pdExcept, "******** Exception ticking the RTI : %d.", &e);
            }
        }
        try {
            rtiamb->queryFederateTime(localTime);
        }
        catch (Exception& e) {
            D.Out(pdExcept,
                  "**** Exception asking for federate local time : ", &e);
        }

        D.Out(pdTrace, "Time advanced, local time is now %.2f.",
              localTime.getTime());

        // Teste la collision avec d'autres billes
        D.Out(pdTrace, "objet: %d, ID: %d.", fedamb->RemoteCount,
              fedamb->Remote[i].ID);

        for (i = 0 ; i < fedamb->RemoteCount ; i++) {
            if ((fedamb->Remote[i].ID != 0) &&
                (fedamb->Local.Collision(&fedamb->Remote[i]))) {
                CBoule *Local = &(fedamb->Local);
                CBille *Remote = &(fedamb->Remote[i]);

                D.Out(pdTrace, "Collision de %d et de %d.", Local->ID,
                      Remote->ID);
#ifndef TEST_USES_GRAPHICS
                // Meme message si on est en mode non graphique
                printf("\nCollision de %d et %d\n", Local->ID, Remote->ID);
#endif
                time_aux = new RTIfedTime(localTime.getTime() +
                                          TIME_STEP.getTime());

                D.Out(pdDebug, 
                      "time_aux : %.2f - localtime : %.2f - timestep : %.2f",
                      time_aux->getTime(),
                      ((RTIfedTime&)localTime).getTime(),
                      ((RTIfedTime&)TIME_STEP).getTime());

                fedamb->sendInteraction(*time_aux, Remote->ID);
                delete time_aux ;
                // On prend la vitesse de l'autre sauf dans le cas ou
                // on avait deja la meme. Dans ce cas, on inverse la notre.
                if ((Local->dx == Remote->dx) &&(Local->dy == Remote->dy)) {
                    Local->dx = -(Local->dx);
                    Local->dy = -(Local->dy);
                }
                else
                    Local->Direction(Remote->dx, Remote->dy);

            }
            D.Out(pdTrace, "pas de collision entre les billes.");

        }

        // Teste la collision avec le bord
        fedamb->Local.CollisionBords(XMAX, YMAX);
        D.Out(pdTrace, "traitement des collisions avec les bords.");

        // Mise a jour graphique
        fedamb->Local.Effacer();
        D.Out(pdTrace, "boule effacee.");

        fedamb->Local.Deplacer();
        fedamb->Local.Afficher();
        D.Out(pdTrace, "boule reaffichee.");

        // Envoie d'une mise a jour des attributs
        time_aux=new RTIfedTime(localTime.getTime()+TIME_STEP.getTime());
        D.Out(pdDebug, "time_aux : %.2f - localtime : %.2f - timestep : %.2f",
              time_aux->getTime(),
              ((RTIfedTime&)localTime).getTime(),
              ((RTIfedTime&)TIME_STEP).getTime());

        fedamb->SendUpdate(*time_aux);
        delete time_aux ;
        D.Out(pdTrace, "fin tour de boucle.");

    } // fin de la boucle de simulation.

    // -----------------
    // -- Terminaison --
    // -----------------
    D.Out(pdTrace, "Fin de la boucle de simulation.");

    fedamb->DeleteObjects(localTime);
    D.Out(pdTerm, "Local objects deleted.");

    if (strcmp(argv[2], "on") == 0) {
        rtiamb->disableTimeConstrained();
        D.Out(pdTerm, "Not constrained anymore.");

        rtiamb->disableTimeRegulation();
        D.Out(pdTerm, "Not regulating anymore");
    }

    try {
        rtiamb->
            resignFederationExecution(DELETE_OBJECTS_AND_RELEASE_ATTRIBUTES);
        D.Out(pdTerm, "Just resigned from federation");
    }
    catch (Exception &e) {
        D.Out(pdExcept,
              "** Exception during resignFederationExecution by federate");
    }
    // Detruire la federation

    if (creator) {
        for(;;) {
            rtiamb->tick(1.0, 2.0);
            nbtick++ ;

            try {
                D.Out(pdTerm, "Asking from federation destruction...");
                rtiamb->destroyFederationExecution(FederationName);

                D.Out(pdTerm, "Federation destruction granted.");
                break ;
            }
            catch (FederatesCurrentlyJoined) {
                sleep(5);
            }
        }
    }
    D.Out(pdTerm, "Destroying RTIAmbassador and FedAmbassador.");
    delete fedamb ;
    delete rtiamb ;

    D.Out(pdTerm, "Federation terminated.");
}

// ---------------------------------------------------------------------------
//! sortir.
void
sortir(int SignalNumber)
{
    if (SignalNumber == SIGALRM) {
        D.Out(pdTerm, "Alarme declenchee, fonction Sortir appellee");
        exit_billard = true ;
    }
    else {
        D.Out(pdTerm, "Emergency stop, destroying Ambassadors.");
        delete fedamb ;
        delete rtiamb ;
        D.Out(pdTerm, "Federate terminated.");
        exit(EXIT_FAILURE);
    }
}

// ---------------------------------------------------------------------------
//! ExceptionHandler.
void
ExceptionHandler(void)
{
    D.Out(pdExcept, "****Exception thrown on the 'test_heritage' Federate.");
    exit(-1);
}

// ---------------------------------------------------------------------------
//! InitialisationGraphique.
void
InitialisationGraphique(FederateHandle myFederateID)
{
    int x = XFEN ;
    int y = YFEN + YOFFSET ;
    unsigned int width = XMAX ;
    unsigned int height = YMAX ;

#ifdef TEST_USES_GRAPHICS
    InitGraphe(x, y, width, height);
    D.Out(pdInit, "InitGraphe(%d, %d, %d, %d)", x, y, width, height);
#endif
}

// ---------------------------------------------------------------------------
//! SetTimeRegulation.
void
SetTimeRegulation(RTI::RTIambassador *rtiamb,
                  bool /*creator*/,
                  FedTime& localTime)
{
    // S'enregistrer comme regulateur.
    while (1) {
        rtiamb->queryFederateTime(localTime);

        try {
            rtiamb->enableTimeRegulation(localTime, TIME_STEP);
            break ;
        }
        catch (FederationTimeAlreadyPassed) {
            // Si Je ne suis pas le premier, je vais les rattraper.
            rtiamb->queryFederateTime(localTime);
            fedamb->granted = false ;
            RTIfedTime requestTime(((RTIfedTime&)localTime).getTime());
            // D.Out(pdInit, "localtime %.2f, requestTime %.2f",
            // ((RTIfedTime&)localTime).getTime(),
            // ((RTIfedTime&)requestTime).getTime());
            requestTime += TIME_STEP ;
            // D.Out(pdInit, "localtime %.2f, requestTime %.2f",
            // ((RTIfedTime&)localTime).getTime(),
            // ((RTIfedTime&)requestTime).getTime());
            rtiamb->timeAdvanceRequest(requestTime);

            while (!fedamb->granted) {
                try {
                    rtiamb->tick(1.0, 2.0);
                }
                catch (RTIinternalError) {
                    printf("RTIinternalError Raised in tick.\n");
                    exit(-1);
                }
            }

        }
        catch (RTIinternalError) {
            printf("RTIinternalError Raised in setTimeRegulating.\n");
            exit(-1);
        }
    }

    D.Out(pdInit, "Time Regulating on.");

    // Se declarer contraint.
    rtiamb->enableTimeConstrained();
    D.Out(pdInit, "Time Constrained true.");
    rtiamb->modifyLookahead(TIME_STEP);
}

// ---------------------------------------------------------------------------
//! Return the remaining time when the alarm is stopped.
void
Synchronize(RTI::RTIambassador *rtiamb, Fed *fedamb, bool creator)
{
    if (creator) {
        // Attendre le signal de l'utilisateur et arreter la pause.

#ifndef NO_CONTROLS
        printf("APPUYER sur ENTER pour lancer l'execution...\n");
        getchar();
#endif

        D.Out(pdInit, "Le createur peut demander de reprendre l'execution.");
        while (!fedamb->paused)
            try {
                D.Out(pdInit, "je ne suis pas en pause");
                D.Out(pdInit, "label - %s",
                      fedamb->CurrentPauseLabel);
                rtiamb->tick(1.0, 2.0);
            }
            catch (Exception& e) {
                D.Out(pdExcept, "******** Exception ticking the RTI : %d ", &e);
            }
        D.Out(pdDebug, "je suis en pause");

        try {
            strcpy(fedamb->CurrentPauseLabel, "Unfreeze");
            // rtiamb->synchronizationPointAchieved(fedamb->CurrentPauseLabel);
            rtiamb->registerFederationSynchronizationPoint(fedamb->
                                                           CurrentPauseLabel,
                                                           "");
        }
        catch (Exception& e) {
            D.Out(pdExcept, "**** Exception achieving a synchronization "
                  "point by creator : %d", &e);
        }
        // while (fedamb->paused)
        // try
        // {
        // rtiamb->tick(1.0, 2.0);
        // }
        // catch (Exception& e)
        // {
        // D.Out("******** Exception ticking the RTI : %d.", &e);
        // }
    }
    else {
        if (!fedamb->paused) {
            D.Out(pdInit,
                  "le Federe n'est pas en pause, il est arriv� trop tot.");
            while (!fedamb->paused) {
                try {
                    rtiamb->tick(1.0, 2.0);
                }
                catch (Exception& e) {
                    D.Out(pdExcept,
                          "******** Exception ticking the RTI : %d.", &e);
                }
            }
        }
        D.Out(pdInit, "le Federe est en pause.");
        D.Out(pdInit, "label avant pause achieved - %s.",
              fedamb->CurrentPauseLabel);

        try {
            // le f�d�r� fait un pause achieved
            rtiamb->synchronizationPointAchieved(fedamb->CurrentPauseLabel);
            D.Out(pdInit, "Pause achieved");
        }
        catch (Exception& e) {
            D.Out(pdExcept,
                  "**** Exception achieving a synchronization point : %d",
                  &e);
        }

        D.Out(pdInit,
              "Le federe attend le fin de la pause demandee par le createur.");
        while (fedamb->paused) {
            try {
                rtiamb->tick(1.0, 2.0);
            }
            catch (Exception& e) {
                D.Out(pdExcept, "******** Exception ticking the RTI : %d.", &e);
            }
        }
        D.Out(pdInit, "fin de la pause");
        D.Out(pdInit, "label - %s", fedamb->CurrentPauseLabel);
        rtiamb->synchronizationPointAchieved(fedamb->CurrentPauseLabel);
        D.Out(pdInit, "Resume achieved");
    }

    D.Out(pdInit, "Federation is synchronized.");

    // Demarre le compte-a-rebours de la simulation.
    struct sigaction a ;

    a.sa_handler = sortir ;
    sigemptyset(&a.sa_mask);
    sigaction(SIGALRM, &a, NULL);
    // sigset(SIGALRM, sortir);

    if (creator)
        alarm(TEMPS_SIMU_C);
    else
        alarm(TEMPS_SIMU_F);

}

// EOF $Id: billard.cc,v 3.7 2003/01/31 10:47:52 breholee Exp $
