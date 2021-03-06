  /// \file SurroundingHitsMergingAlgorithm.h
/*
 *
 * SurroundingHitsMergingAlgorithm.h header template automatically generated by a class generator
 * Creation date : jeu. nov. 19 2015
 *
 * This file is part of APRILContent libraries.
 * 
 * APRILContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * APRILContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with APRILContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef SURROUNDINGHITSMERGINGALGORITHM_H
#define SURROUNDINGHITSMERGINGALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"
#include "APRILApi/APRILInputTypes.h"

namespace april_content
{

class CaloHitMergingTool;

/** 
 *  @brief  SurroundingHitsMergingAlgorithm class
 */ 
class SurroundingHitsMergingAlgorithm : public pandora::Algorithm
{
public:
	/**
	 *  @brief  Factory class for instantiating algorithm
	 */
	class Factory : public pandora::AlgorithmFactory
	{
	public:
		pandora::Algorithm *CreateAlgorithm() const;
	};

private:
	pandora::StatusCode Run();
	pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

private:
	/**
	 *  @brief  Get the pandora content to perform the algorithm
	 *
	 *  @param  caloHitList the calo hit list to receive
	 *  @param  clusterVector the cluster list to receive
	 */
	pandora::StatusCode GetContents(pandora::CaloHitList &caloHitList, pandora::ClusterVector &clusterVector) const;

	/**
	 *  @brief  Get the list of available calo hits
	 *
	 *  @param pCaloHitList the input calo hit list to extract available ones
	 *  @param availableCaloHitList the list of available calo hits to receive
	 */
	pandora::StatusCode GetAvailableCaloHitList(const pandora::CaloHitList *const pCaloHitList, pandora::CaloHitList &availableCaloHitList) const;

private:
	bool                                     m_shouldMergeIsolatedHits;      ///< Whether to merge isolated hits in clusters
	pandora::StringVector                    m_additionalClusterListNames;   ///< Additional cluster list names to perform the merging
	CaloHitMergingTool                      *m_pCaloHitMergingTool;          ///< The calo hit merging tool
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *SurroundingHitsMergingAlgorithm::Factory::CreateAlgorithm() const
{
    return new SurroundingHitsMergingAlgorithm();
}

} 

#endif  //  SURROUNDINGHITSMERGINGALGORITHM_H
