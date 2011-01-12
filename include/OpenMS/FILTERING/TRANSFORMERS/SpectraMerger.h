// -*- mode: C++; tab-width: 2; -*-
// vi: set ts=2:
//
// --------------------------------------------------------------------------
//                   OpenMS Mass Spectrometry Framework
// --------------------------------------------------------------------------
//  Copyright (C) 2003-2010 -- Oliver Kohlbacher, Knut Reinert
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
// $Maintainer: Chris Bielow $
// $Authors: Chris Bielow, Andreas Bertsch $
// --------------------------------------------------------------------------
//
#ifndef OPENMS_FILTERING_TRANSFORMERS_SPECTRAMERGER_H
#define OPENMS_FILTERING_TRANSFORMERS_SPECTRAMERGER_H

#include <OpenMS/DATASTRUCTURES/DefaultParamHandler.h>
#include <OpenMS/COMPARISON/CLUSTERING/CompleteLinkage.h>
#include <OpenMS/COMPARISON/CLUSTERING/SingleLinkage.h>
#include <OpenMS/COMPARISON/CLUSTERING/ClusterAnalyzer.h>
#include <OpenMS/COMPARISON/CLUSTERING/ClusterHierarchical.h>
#include <OpenMS/KERNEL/StandardTypes.h>
#include <OpenMS/KERNEL/RangeUtils.h>
#include <OpenMS/KERNEL/BaseFeature.h>
#include <OpenMS/CONCEPT/LogStream.h>
#include <vector>

namespace OpenMS
{

	/**	
  	@brief Merges blocks of MS or MS2 spectra
		
    Parameter's are accessible via the DefaultParamHandler.

		@htmlinclude OpenMS_SpectraMerger.parameters

  */
  class OPENMS_DLLAPI SpectraMerger
    : public DefaultParamHandler
  {

  protected:

    /* Determine distance between two spectra

      Distance is determined as 
      
        (d_rt/rt_max_ + d_mz/mz_max_) / 2

    */
    class SpectraDistance_
      : public DefaultParamHandler
    {
      public:
      SpectraDistance_()
        : DefaultParamHandler("SpectraDistance")
      {
        defaults_.setValue("rt_tolerance", 10.0, "Maximal RT distance (in [s]) for two spectra's precursor.s");
        defaults_.setValue("rt_weight", 1, "Multiplier for RT distance, to determine distance (delta m/z * weight + delta rt * weight < threshold).");
        defaults_.setValue("mz_tolerance", 1.0, "Maximal m/z distance (in Da) for two spectra's precursors.");
        defaults_.setValue("mz_weight", 10, "Multiplier for m/z distance, to determine distance (delta m/z * weight + delta rt * weight < threshold).");
        defaultsToParam_();
      }
      
      void updateMembers_()
      {
        rt_max_ = (DoubleReal) param_.getValue("rt_tolerance");
        rt_weight_ = (DoubleReal) param_.getValue("rt_weight");
        mz_max_ = (DoubleReal) param_.getValue("mz_tolerance");
        mz_weight_ = (DoubleReal) param_.getValue("mz_weight");

        return;
      }

      double getSimilarity(const DoubleReal d_rt, const DoubleReal d_mz) const
      {
        //     1 - distance
        return 1 - ( (d_rt/rt_max_ + d_mz/mz_max_) / 2);
      }

      // measure of SIMILARITY (not distance, i.e. 1-distance)!!
      double operator()(const BaseFeature& first, const BaseFeature& second) const
      {
        // get RT distance:
        DoubleReal d_rt = fabs(first.getRT() - second.getRT());
        DoubleReal d_mz = fabs(first.getMZ() - second.getMZ());

        if (d_rt > rt_max_ || d_mz > mz_max_) {return 0;}
        
        // calculate similarity (0-1):
        DoubleReal sim = getSimilarity(d_rt, d_mz);
      
        return sim;
      }

    protected:
      DoubleReal rt_max_;
      DoubleReal rt_weight_;
      DoubleReal mz_max_;
      DoubleReal mz_weight_;
      DoubleReal max_possible_distance_;

    }; // end of SpectraDistance

  public:

    /// blocks of spectra (master-spectrum index to sacrifice-spectra(the ones being merged into the master-spectrum))
    typedef Map<Size, std::vector<Size> > MergeBlocks;

		// @name Constructors and Destructors
		// @{
    /// default constructor
    SpectraMerger();

    /// copy constructor 
    SpectraMerger(const SpectraMerger& source);

    /// destructor
    virtual ~SpectraMerger();
		// @}

		// @name Operators
		// @{
    /// assignment operator
    SpectraMerger& operator=(const SpectraMerger& source);
		// @}

		// @name Merging functions
		// @{
		/// 
		template <typename MapType> void mergeSpectraBlockWise(MapType& exp)
		{
			IntList ms_levels = (IntList) (param_.getValue("block_method:ms_levels"));
      Int rt_block_size(param_.getValue("block_method:rt_block_size"));
      DoubleReal rt_max_length = (param_.getValue("block_method:rt_max_length"));

      if (rt_max_length == 0)  // no rt restriction set?
      {
        rt_max_length = std::numeric_limits<DoubleReal>::max(); // set max rt span to very large value
      }

      for (IntList::iterator it_mslevel = ms_levels.begin(); it_mslevel<ms_levels.end(); ++it_mslevel)
      {
        MergeBlocks spectra_to_merge;
        Size idx_block(0);
        SignedSize block_size_count(rt_block_size+1);
        Size idx_spectrum(0);
        for (typename MapType::const_iterator it1 = exp.begin(); it1 != exp.end(); ++it1)
			  {
				  if (Int(it1->getMSLevel()) == *it_mslevel)
				  {
            // block full if it contains a maximum number of scans or if maximum rt length spanned
            if (++block_size_count >= rt_block_size ||
                exp[idx_spectrum].getRT() - exp[idx_block].getRT() > rt_max_length)
            {
              block_size_count = 0;
              idx_block = idx_spectrum;
            }
            else
            {
              spectra_to_merge[idx_block].push_back(idx_spectrum);
            }
          }

          ++idx_spectrum;
        }
        // check if last block had sacrifice spectra
        if (block_size_count==0)
        { //block just got initialized
          spectra_to_merge[idx_block] = std::vector<Size>();
        }

        // merge spectra, remove all old MS spectra and add new consensus spectra
        mergeSpectra_(exp, spectra_to_merge, *it_mslevel);
      }

      exp.sortSpectra();

			return;
		}

    /// merges spectra with similar precursors (must have MS2 level)
		template <typename MapType> void mergeSpectraPrecursors(MapType& exp)
		{

      // convert spectra's precursors to clusterizable data
			Size data_size;
			std::vector<BinaryTreeNode> tree;
			Map<Size, Size> index_mapping;
			// local scope to save memory - we do not need the clustering stuff later
			{
				std::vector<BaseFeature> data;

				for (Size i=0;i<exp.size(); ++i)
				{
					if (exp[i].getMSLevel() != 2) continue;

					// remember which index in distance data ==> experiment index
					index_mapping[data.size()] = i;

					// make cluster element
					BaseFeature bf;
					bf.setRT(exp[i].getRT());
					std::vector< Precursor > pcs = exp[i].getPrecursors();
					if (pcs.size()==0) throw Exception::MissingInformation(__FILE__, __LINE__, __PRETTY_FUNCTION__, String("Scan #") + String(i) + " does not contain any precursor information! Unable to cluster!");
					if (pcs.size()>1) LOG_WARN << "More than one precursor found. Using first one!" << std::endl;
					bf.setMZ(pcs[0].getMZ());
					data.push_back(bf);
				}
				data_size = data.size();

				SpectraDistance_ llc;
				llc.setParameters(param_.copy("precursor_method:",true));
				SingleLinkage sl;
				DistanceMatrix<Real> dist; // will be filled
				ClusterHierarchical ch;
				
        //ch.setThreshold(0.99);
				// clustering ; threshold is implicitly at 1.0, i.e. distances of 1.0 (== similiarity 0) will not be clustered
				ch.cluster<BaseFeature,SpectraDistance_>(data,llc,sl,tree,dist);
			}

      // extract the clusters
      ClusterAnalyzer ca;
      std::vector<std::vector<Size> > clusters;
      // count number of real tree nodes (not the -1 ones):
      Size node_count=0;
      for (Size ii=0;ii<tree.size();++ii)
			{
				if (tree[ii].distance >= 1) tree[ii].distance=-1; // manually set to disconnect, as SingleLinkage does not support it
        if (tree[ii].distance != -1) ++node_count;
			}
      ca.cut(data_size-node_count, tree, clusters);

      //std::cerr << "Treesize: " << (tree.size()+1) << "   #clusters: " << clusters.size() << std::endl;
      //std::cerr << "tree:\n" << ca.newickTree(tree, true) << "\n";

      // convert to blocks
      MergeBlocks spectra_to_merge;

      for (Size i_outer=0;i_outer<clusters.size(); ++i_outer)
      {
        if (clusters[i_outer].size() <= 1) continue;
        // init block with first cluster element
        Size cl_index0 = clusters[i_outer][0];
        spectra_to_merge[ index_mapping[cl_index0] ] = std::vector<Size>();
        // add all other elements
        for (Size i_inner=1;i_inner<clusters[i_outer].size();++i_inner)
        {
          Size cl_index = clusters[i_outer][i_inner];
          spectra_to_merge[ index_mapping[cl_index0] ].push_back( index_mapping[cl_index] );
        }
      }

      // do it
      mergeSpectra_(exp, spectra_to_merge, 2);

      exp.sortSpectra();

			return;
		}


		// @}

    protected:
    
    /**
        @brief merges blocks of spectra of a certain level

        Merges spectra belonging to the same block, setting their MS level to @p ms_level.
        All old spectra of level @p ms_level are removed, and the new consensus spectra (one per block)
        are added.
        All spectra with other MS levels remain untouched.
        The resulting map is NOT sorted!

    */
    template <typename MapType>
    void mergeSpectra_(MapType& exp, const MergeBlocks& spectra_to_merge, const UInt ms_level)
    {
			DoubleReal mz_binning_width(param_.getValue("mz_binning_width"));
			String mz_binning_unit(param_.getValue("mz_binning_width_unit"));

      // merge spectra
			MapType merged_spectra;

      Map <Size, Size> cluster_sizes;
      std::set <Size> merged_indices;

      // each BLOCK
			for (Map<Size, std::vector<Size> >::ConstIterator it = spectra_to_merge.begin(); it != spectra_to_merge.end(); ++it)
			{

        ++cluster_sizes[ it->second.size() + 1]; // for stats

  			typename MapType::SpectrumType consensus_spec;
		  	consensus_spec.setMSLevel(ms_level);

        consensus_spec.unify(exp[it->first]); // append meta info
        merged_indices.insert(it->first);

        typename MapType::SpectrumType all_peaks = exp[it->first];			
        DoubleReal rt_average=all_peaks.getRT();
        DoubleReal mz_average=all_peaks.getPrecursors()[0].getMZ();

        // block elements
				for (std::vector<Size>::const_iterator sit = it->second.begin(); sit != it->second.end(); ++sit)
				{
          consensus_spec.unify(exp[*sit]); // append meta info
          merged_indices.insert(*sit);

          rt_average+=exp[*sit].getRT();
          if (ms_level >= 2) mz_average+=exp[*sit].getPrecursors()[0].getMZ();
					for (typename MapType::SpectrumType::ConstIterator pit = exp[*sit].begin(); pit != exp[*sit].end(); ++pit)
					{
						all_peaks.push_back(*pit);
					}
				}
				all_peaks.sortByPosition();
        rt_average/=it->second.size()+1;
        consensus_spec.setRT(rt_average);

        mz_average/=it->second.size()+1;
        if (ms_level >= 2)
        {
          std::vector< Precursor > pcs = consensus_spec.getPrecursors();
          //if (pcs.size()>1) LOG_WARN << "Removing excessive precursors - leaving only one per MS2 spectrum.\n";
          pcs.resize(1);
          pcs[0].setMZ(mz_average);
          consensus_spec.setPrecursors(pcs);
        }

        if (all_peaks.size()==0) continue;
        else
        {
          typename MapType::PeakType old_peak = *all_peaks.begin();
          DoubleReal distance;
		  	  for (typename MapType::SpectrumType::ConstIterator it = (++all_peaks.begin()); it != all_peaks.end(); ++it)
		  	  {
            if (mz_binning_unit=="Da") distance=fabs(old_peak.getMZ() - it->getMZ());   //Da delta
            else distance= fabs(old_peak.getMZ() - it->getMZ())*1e6 / old_peak.getMZ(); //ppm delta

		    	  if (distance < mz_binning_width)
		    	  {
		      	  old_peak.setIntensity(old_peak.getIntensity() + it->getIntensity());
		    	  }
   		 		  else
    			  {
      			  consensus_spec.push_back(old_peak);
     		 		  old_peak = *it;
    			  }
  			  }
          consensus_spec.push_back(old_peak); // store last peak

				  merged_spectra.push_back(consensus_spec);
        }
			}

      LOG_INFO << "Cluster sizes:\n";
      for (Map <Size, Size>::const_iterator it=cluster_sizes.begin(); it!=cluster_sizes.end();++it)
      {
        LOG_INFO << "  size " << it->first << ": " << it->second << "x\n";
      }
      
      // remove all spectra that were within a cluster
      MapType::SpectrumType empty_spec;
      MapType exp_tmp;
      for (Size i=0;i<exp.size();++i)
      {
        if (merged_indices.count(i)==0) // save unclustered ones
        {
          exp_tmp.push_back(exp[i]);
          exp[i] = empty_spec;
        }
      }
      typedef std::vector<MapType::SpectrumType> Base;
      exp.Base::operator=(exp_tmp);

      // exp.erase(remove_if(exp.begin(), exp.end(), InMSLevelRange<typename MapType::SpectrumType>(IntList::create(String(ms_level)), false)), exp.end());

      // ... and add consensus spectra
			exp.insert(exp.end(), merged_spectra.begin(), merged_spectra.end());

    }
	
  };
	
}
#endif //OPENMS_FILTERING_TRANSFORMERS_SPECTRAMERGER_H
