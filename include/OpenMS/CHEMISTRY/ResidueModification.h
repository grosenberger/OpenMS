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

#ifndef OPENMS_CHEMISTRY_RESIDUEMODIFICATION_H
#define OPENMS_CHEMISTRY_RESIDUEMODIFICATION_H

#include <OpenMS/CHEMISTRY/EmpiricalFormula.h>
#include <OpenMS/CONCEPT/Types.h>
#include <OpenMS/DATASTRUCTURES/String.h>

#include <set>

namespace OpenMS
{
	// forward declaration
	class Residue;

	/** @brief Representation of a modification

	*/
	class ResidueModification
	{
		public:
	
			/** Enums
			*/
			//@{
			/** @brief Position where the modification is allowed to occur
			
					The allowed sites are
					Any C-term
					Any N-term
					Anywhere
					Protein C-term
					Protein N-term

					This does not describe the amino acids which are valid for a 
					specific amino acid!

			*/
			enum Term_Specificity
			{
				ANYWHERE = 0,
				C_TERM = 1,
				N_TERM =2,
				NUMBER_OF_TERM_SPECIFICITY
			};

			/** @brief Classification of the modification

					the PSI-MOD defined the following classes of modifications
						AA substitution
						Artefact
						Chemical derivative
						Co-translational
						Isotopic label
						Multiple
						N-linked glycosylation
						Non-standard residue
						O-linked glycosylation
						Other
						Other glycosylation
						Post-translational
						Pre-translational
						Synth. pep. protect. gp.
													 
				
			*/
			enum Source_Classification
			{
				ARTIFACT = 0,
				HYPOTHETICAL, 
				NATURAL,
				NUMBER_OF_SOURCE_CLASSIFICATIONS
			};
			//@}
			


			/** @name Constructors and Destructors
			*/
			//@{
			/// default constructor
			ResidueModification();
			
			/// copy constructor
			ResidueModification(const ResidueModification& modification);

			/// destructor				
			virtual ~ResidueModification();
			//@}

			/** @name Assignment operator
			*/
			//@{
			/// assignment operator
			ResidueModification& operator = (const ResidueModification& modification);
			//@}

			/** @name Accessors
			*/
			//@{
			/// set the identifier of the modification
			void setId(const String& id);

			/// returns the identifier the modification
			const String& getId() const;

			/// sets the full name of the modification
			void setFullName(const String& full_name);

			/// returns the full name of the modification
			const String& getFullName() const;

			/// sets the name of modification
			void setName(const String& name);

			/// returns the PSI-MS-label if available; e.g. Mascot uses this name
			const String& getName() const;
			
			/// sets the term specificity 
			void setTermSpecificity(Term_Specificity term_spec);

			/// sets the term specificity specified using a name
			void setTermSpecificity(const String& name);
			
			/// returns terminal specificity
			Term_Specificity getTermSpecificity() const;
			
			/// returns the terminal specificity name which is set or given as parameter
			String getTermSpecificityName(Term_Specificity = NUMBER_OF_TERM_SPECIFICITY) const;
	
			///	sets the origin 
			void setOrigin(const String& origin);

			/// returns the origin if set
			const String& getOrigin() const;

			/// classification as defined by the PSI-MOD
			void setSourceClassification(const String& classification);

			/// sets the source classification
			void setSourceClassification(Source_Classification classification);

			/// returns the source classification, if none was set, it is unspecific
			Source_Classification getSourceClassification() const;
			
			/// returns the classification
			String getSourceClassificationName(Source_Classification classification = NUMBER_OF_SOURCE_CLASSIFICATIONS) const;
			
			/// sets the average mass
			void setAverageMass(double mass);

			/// returns the average mass if set 
			double getAverageMass() const;

			/// sets the monoisotopic mass
			void setMonoMass(double mass);

			/// return the monoisotopic mass, if set
			double getMonoMass() const;

			/// set the difference average mass
			void setDiffAverageMass(double mass);

			/// returns the difference average mass if set
			double getDiffAverageMass() const;

			/// sets the difference monoisotopic mass 
			void setDiffMonoMass(double mass);

			/// returns the diff monoisotopic mass if set
			double getDiffMonoMass() const;
			
			/// set the formula 
			void setFormula(const String& composition);

			/// returns the chemical formula if set
			const String& getFormula() const;

			/// sets diff formula
			void setDiffFormula(const String& diff_formula);

			/// returns the diff formula if one was set
			const String& getDiffFormula() const;
			
			/// sets the synonyms of that modification
			void setSynonyms(const std::set<String>& synonyms);

			/// adds a synonym to the unique list
			void addSynonym(const String& synonym);

			/// returns the set of synonyms
			const std::set<String>& getSynonyms() const;
			//@}

			/** @name Predicates
			*/
			//@{
			/// equality operator
			bool operator == (const ResidueModification& modification) const;

			/// inequality operator
			bool operator != (const ResidueModification& modification) const;
			//@}
				
		protected:

			String id_;

			String full_name_;

			String name_;
			
			Term_Specificity term_spec_;
			
			String origin_;

			Source_Classification classification_;

			double average_mass_;
			
			double mono_mass_;
			
			double diff_average_mass_;

			double diff_mono_mass_;
			
			String formula_;

			String diff_formula_;

			//std::vector<String> valid_residues_;

			std::set<String> synonyms_;
	};
}

#endif
