// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*- 
// ---------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2002  ONERA
//
// This file is part of CERTI-libcerti
//
// CERTI-libcerti is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libcerti is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: Subscriber.hh,v 3.2 2003/01/15 12:07:46 breholee Exp $
// ---------------------------------------------------------------------------

// $Id: Subscriber.hh,v 3.2 2003/01/15 12:07:46 breholee Exp $
// ---------------------------------------------------------------------------
// Classe Subscriber, qui decrit un souscripteur a une classe
// d'interaction ou a une classe d'objet.

#ifndef _CERTI_SUBSCRIBER_HH
#define _CERTI_SUBSCRIBER_HH

#include <config.h>
#include "RTItypes.hh"

namespace certi {

//! Contains the federate handle of a subscriber.
/*! The description of the region of subscribtion, this attribute,
    CSubscribeRegion Region, should exists. More to be implemented.
*/
class Subscriber {
  // ATTRIBUTES
protected:
  FederateHandle handle; //!< The ID of the Subscriber.

  // METHODS
public:
  Subscriber(FederateHandle);
  FederateHandle getHandle(void) const;
};

}

#endif // _CERTI_SUBSCRIBER_HH

// $Id: Subscriber.hh,v 3.2 2003/01/15 12:07:46 breholee Exp $

