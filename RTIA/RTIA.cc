// ----------------------------------------------------------------------------
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
// $Id: RTIA.cc,v 3.8 2003/05/23 14:54:06 breholee Exp $
// ----------------------------------------------------------------------------

#include "RTIA.hh"

namespace certi {
namespace rtia {

static pdCDebug D("RTIA", "(RTIA) ");

// Tableau des messages pouvant etre recus du RTIG

#define MSG_RTIG_MAX 18

const char *Messages_RTIG[MSG_RTIG_MAX] = {
    "MESSAGE NULL",
    "SET_TIME_REGULATING",
    "REQUEST_PAUSE", // to be removed or changed later.
    "REQUEST_RESUME", // to be removed or changed later.
    "DISCOVER_OBJECT",
    "START_REGISTRATION_FOR_OBJECT_CLASS",
    "STOP_REGISTRATION_FOR_OBJECT_CLASS",
    "REFLECT_ATTRIBUTE_VALUES",
    "SEND_INTERACTION",
    "REMOVE_OBJECT",
    "INFORM_ATTRIBUTE_OWNERSHIP",
    "ATTRIBUTE_IS_NOT_OWNED",
    "REQUEST_ATTRIBUTE_OWNERSHIP_ASSUMPTION",
    "ATTRIBUTE_OWNERSHIP_UNAVAILABLE",
    "ATTRIBUTE_OWNERSHIP_ACQUISITION_NOTIFICATION",
    "ATTRIBUTE_OWNERSHIP_DIVESTITURE_NOTIFICATION",
    "REQUEST_ATTRIBUTE_OWNERSHIP_RELEASE",
    "CONFIRM_ATTRIBUTE_OWNERSHIP_ACQUISITION_CANCELLATION"
};

// Tableau des messages pouvant etre recus du Federe.

#define MSG_FED_MAX 53

const char *Messages_Fed[MSG_FED_MAX] = {
    "CREATE_FEDERATION_EXECUTION",
    "DESTROY_FEDERATION_EXECUTION",
    "JOIN_FEDERATION_EXECUTION",
    "RESIGN_FEDERATION_EXECUTION",
    "REQUEST_PAUSE", // to be removed or changed later.
    "PAUSE_ACHIEVED", // to be removed or changed later.
    "REQUEST_RESUME", // to be removed or changed later.
    "RESUME_ACHIEVED", // to be removed or changed later.
    "PUBLISH_OBJECT_CLASS",
    "UNPUBLISH_OBJECT_CLASS",
    "PUBLISH_INTERACTION_CLASS",
    "UNPUBLISH_INTERACTION_CLASS",
    "SUBSCRIBE_OBJECT_CLASS_ATTRIBUTE",
    "UNSUBSCRIBE_OBJECT_CLASS_ATTRIBUTE",
    "SUBSCRIBE_INTERACTION_CLASS",
    "UNSUBSCRIBE_INTERACTION_CLASS",
    "REQUEST_ID",
    "REGISTER_OBJECT",
    "UPDATE_ATTRIBUTE_VALUES",
    "SEND_INTERACTION",
    "DELETE_OBJECT",
    "CHANGE_ATTRIBUTE_TRANSPORT_TYPE",
    "CHANGE_ATTRIBUTE_ORDER_TYPE",
    "CHANGE_INTERACTION_TRANSPORT_TYPE",
    "CHANGE_INTERACTION_ORDER_TYPE",
    "REQUEST_FEDERATION_TIME",
    "REQUEST_LBTS",
    "REQUEST_FEDERATE_TIME",
    "SET_LOOKAHEAD",
    "REQUEST_LOOKAHEAD",
    "TIME_ADVANCED_REQUEST",
    "NEXT_EVENT_REQUEST",
    "GET_OBJECT_CLASS_HANDLE",
    "GET_OBJECT_CLASS_NAME",
    "GET_ATTRIBUTE_HANDLE",
    "GET_ATTRIBUTE_NAME",
    "GET_INTERACTION_CLASS_HANDLE",
    "GET_INTERACTION_CLASS_NAME",
    "GET_PARAMETER_HANDLE",
    "GET_PARAMETER_NAME",
    "SET_TIME_REGULATING",
    "SET_TIME_CONSTRAINED",
    "TICK_REQUEST",
    "IS_ATTRIBUTE_OWNED_BY_FEDERATE",
    "QUERY_ATTRIBUTE_OWNERSHIP",
    "NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
    "ATTRIBUTE_OWNERSHIP_ACQUISITION_IF_AVAILABLE",
    "UNCONDITIONAL_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
    "ATTRIBUTE_OWNERSHIP_ACQUISITION",
    "CANCEL_NEGOTIATED_ATTRIBUTE_OWNERSHIP_DIVESTITURE",
    "ATTRIBUTE_OWNERSHIP_RELEASE_RESPONSE",
    "CANCEL_ATTRIBUTE_OWNERSHIP_ACQUISITION",
    "GET_SPACE_HANDLE"
};

// ----------------------------------------------------------------------------
// Displays statistics (requests, rtig messages, ...).
void
RTIA::count()
{
#ifdef RTI_PRINTS_STATISTICS

    char *s = getenv("CERTI_NO_STATISTICS");
    if (s) return ;

    cout << endl << "RTIA: Statistics (processed messages)" << endl ;

    for (int j = 0 ; j < MSG_FED_MAX ; j++)
        cout << " Federate request type " << Messages_Fed[j]
             << ": " << nb_requetes[j] << endl ;

    cout << endl ;

    for (int j = 0 ; j < MSG_RTIG_MAX ; j++)
        cout << " RTIG messages type " << Messages_RTIG[j]
             << ": " << nb_messages[j] << endl ;

    cout << endl << " Number of interactions : " << nb_evenements << endl ;
    cout << " Number of RTIG messages : " << TOTAL << endl ;

#endif
}

// ----------------------------------------------------------------------------
//! RTIA constructor.
RTIA::RTIA()
{
    // No SocketServer is passed to the RootObject.
    rootObject = new RootObject(NULL);

    comm = new Communications();
    queues = new Queues ;
    fm = new FederationManagement(comm);
    om = new ObjectManagement(comm, fm, rootObject);
    owm = new OwnershipManagement(comm, fm);
    dm = new DeclarationManagement(comm, fm, rootObject);
    tm = new TimeManagement(comm, queues, fm, om, owm);
    ddm = new DataDistribution(rootObject, fm, comm);

    fm->tm = tm ;
    queues->fm = fm ;

    TOTAL = 0 ;
    nb_evenements = 0 ;

    for (int j = 0 ; j < MSG_FED_MAX ; j++)
        nb_requetes[j]=0 ;

    for (int j = 0 ; j < MSG_RTIG_MAX ; j++)
        nb_messages[j]=0 ;
}

// ----------------------------------------------------------------------------
// RTIA Destructor
RTIA::~RTIA()
{
    // BUG: TCP link destroyed ?

    delete tm ;
    delete dm ;
    delete om ;
    delete fm ;
    delete queues ;
    delete comm ;
    delete ddm ;
    delete rootObject ;

    // Display statistics.
    count();

    cout << "RTIA: End execution." << endl ;
}

// ----------------------------------------------------------------------------
//! RTIA mainloop.
/*! Messages allocated for reading data exchange between RTIA and federate/RTIG
  are freed by 'processFederateRequest' or 'processNetworkMessage'.
*/
void
RTIA::execute()
{
    Message *msg_un ;
    NetworkMessage *msg_tcp_udp ;
    int n ;

    while (!fm->_fin_execution) {

        msg_tcp_udp = new NetworkMessage ;
        msg_un = new Message ;

        try {
            comm->readMessage(n, msg_tcp_udp, msg_un);
        }
        catch (NetworkSignal) {
            fm->_fin_execution = RTI_TRUE ;
            n = 0 ;
            delete msg_un ;
            delete msg_tcp_udp ;
        }

        switch (n) {
          case 0:
            break ;
          case 1:
            processNetworkMessage(msg_tcp_udp);
            delete msg_un ;
            break ;
          case 2:
            processFederateRequest(msg_un);
            delete msg_tcp_udp ;
            break ;
          default:
            assert(false);
        }
    }
}

}} // namespace certi/rtia

// $Id: RTIA.cc,v 3.8 2003/05/23 14:54:06 breholee Exp $
