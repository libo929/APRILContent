  /// \file ClosebySeedMergingAlgorithm.h
/*
 *
 * ClosebySeedMergingAlgorithm.h header template automatically generated by a class generator
 * Creation date : jeu. avr. 23 2015
 *
 * This file is part of ArborContent libraries.
 * 
 * ArborContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * ArborContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#ifndef CLOSEBYSEEDMERGINGALGORITHM_H
#define CLOSEBYSEEDMERGINGALGORITHM_H

#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInputTypes.h"

namespace arbor_content
{

class CaloHit;

/**
 *  @brief  ClosebySeedMergingAlgorithm class
 */ 
class ClosebySeedMergingAlgorithm : public pandora::Algorithm
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

	typedef std::map<const CaloHit *, const pandora::Cluster *> CaloHitSeedToClusterMap;

	/**
	 *
	 */
	pandora::StatusCode FindMergeCandidateClusters(const pandora::ClusterList *const pClusterList, CaloHitSeedToClusterMap &caloHitSeedToClusterMap) const;

	/**
	 *
	 */
	pandora::StatusCode MergeCloseBySeedClusters(CaloHitSeedToClusterMap &caloHitSeedToClusterMap) const;

	/**
	 *
	 */
	pandora::StatusCode ReplaceClusterEntryInSeedMap(const pandora::Cluster *const pClusterToReplace, const pandora::Cluster *const pClusterReplacement,
			CaloHitSeedToClusterMap &caloHitSeedToClusterMap) const;

	bool                                 m_discriminateSeedLeafHits;
	unsigned int                         m_maxSeedPseudoLayerDifference;
	float                                m_maxSeedDistanceFine;
	float                                m_maxSeedDistanceCoarse;
	float                                m_maxSeedTransverseDistanceFine;
	float                                m_maxSeedTransverseDistanceCoarse;
	bool                                 m_mergeECalSeedClusters;
	bool                                 m_mergeHCalSeedClusters;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *ClosebySeedMergingAlgorithm::Factory::CreateAlgorithm() const
{
    return new ClosebySeedMergingAlgorithm();
}

} 

#endif  //  CLOSEBYSEEDMERGINGALGORITHM_H
