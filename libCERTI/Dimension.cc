// ----------------------------------------------------------------------------
// CERTI - HLA RunTime Infrastructure
// Copyright (C) 2003-2005  ONERA
//
// This file is part of CERTI-libCERTI
//
// CERTI-libCERTI is free software ; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation ; either version 2 of
// the License, or (at your option) any later version.
//
// CERTI-libCERTI is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY ; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this program ; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
//
// $Id: Dimension.cc,v 3.7 2007/09/03 13:26:05 erk Exp $
// ----------------------------------------------------------------------------

#include "Dimension.hh"

namespace certi {

ULong Dimension::axisLowerBound = 0 ;
ULong Dimension::axisUpperBound = LONG_MAX ;

Dimension::Dimension(DimensionHandle dimensionHandle)
{
	/* initialize handle from superclass */
    this->handle = dimensionHandle ;
}

// ----------------------------------------------------------------------------
void
Dimension::setLowerBound(ULong lowerBound)
{
    Dimension::axisLowerBound = lowerBound ;
}

// ----------------------------------------------------------------------------
void
Dimension::setUpperBound(ULong upperBound)
{
    Dimension::axisUpperBound = upperBound ;
}

} // namespace certi

// $Id: Dimension.cc,v 3.7 2007/09/03 13:26:05 erk Exp $


