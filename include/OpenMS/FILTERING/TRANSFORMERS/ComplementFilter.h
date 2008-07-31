// -*- mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2008 -- Oliver Kohlbacher, Knut Reinert
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2.1 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
// --------------------------------------------------------------------------
// $Maintainer: Andreas Bertsch $
// --------------------------------------------------------------------------
//
#ifndef OPENMS_FILTERING_TRANSFORMERS_COMPLEMENTFILTER_H
#define OPENMS_FILTERING_TRANSFORMERS_COMPLEMENTFILTER_H

#include <OpenMS/FILTERING/TRANSFORMERS/FilterFunctor.h>

#include <map>
#include <cmath>

namespace OpenMS
{
  /**
    @brief total intensity of peak pairs that could result from complementing fragments of charge state 1
		 
		@ref ComplementFilter_Parameters are explained on a separate page.

		@ingroup SpectraFilter
  */
  class ComplementFilter : public FilterFunctor
  {
  public:

		// @name Constructors and Destructors
		//@{
    /// standard constructor
    ComplementFilter();

    /// copy constructor
    ComplementFilter(const ComplementFilter& source);

    /// destructor
    virtual ~ComplementFilter();
		//@}

		// @name Operators
		//@{
		/// assignment operator
		ComplementFilter& operator = (const ComplementFilter& source);
		//@}

		// @name Accessors
		//@{
    static FilterFunctor* create() { return new ComplementFilter(); }

		/// returns the total intensity of peak pairs which could result from complementing fragments
		template <typename SpectrumType> double apply(SpectrumType& spectrum)
		{
			if (spectrum.size() < 2)
			{
				return 0;
			}
	    double tolerance = (double)param_.getValue("tolerance");
	    double parentmass = spectrum.getPrecursorPeak().getPosition()[0];
	    double result(0);
		
			spectrum.getContainer().sortByPosition();
	
			/// @improvement think about an correct fast algorithm, not just an heuristic (Andreas)
	    uint j = spectrum.size() - 1;
	    for (uint i = 0; i < spectrum.size() && i <= j; /*++i*/)
	    {
	      double sum = spectrum.getContainer()[i].getPosition()[0] + spectrum.getContainer()[j].getPosition()[0];
				
	      if (std::fabs(sum - parentmass) < tolerance)
	      {
	        result += spectrum.getContainer()[i].getIntensity() + spectrum.getContainer()[j].getIntensity();
	      }
				
				if (sum < parentmass)
		    {
	  	     ++i;
	    	}
				else 
				{
					if (sum > parentmass)
	      	{
	        	--j;
	      	}
				}	
	    }
			
	    return result;
		}

		/// returns the name for registration at the factory
		static const String getProductName()
		{
			return "ComplementFilter";
		}
		//@}

  };
}
#endif // OPENMS_FILTERING_TRANSFORMERS_COMPLEMENTFILTER_H
