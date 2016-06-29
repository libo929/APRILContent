  /// \file SurroundingHitsMergingAlgorithm.cc
/*
 *
 * SurroundingHitsMergingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. nov. 19 2015
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


#include "ArborTopologicalAssociation/SurroundingHitsMergingAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborApi/ArborContentApi.h"
#include "ArborHelpers/ClusterHelper.h"

namespace arbor_content
{

// TODO use open mp
// => parallel on cluster loop in FindCaloHitClusterMerging()

pandora::StatusCode SurroundingHitsMergingAlgorithm::Run()
{
	pandora::CaloHitList caloHitList; pandora::ClusterVector clusterVector;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetContents(caloHitList, clusterVector));

	CaloHitToClusterMap caloHitToClusterMap;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->FindCaloHitClusterMerging(caloHitList, clusterVector, caloHitToClusterMap));
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->MergeCaloHits(caloHitToClusterMap));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode SurroundingHitsMergingAlgorithm::GetAvailableCaloHitList(const pandora::CaloHitList *const pCaloHitList, pandora::CaloHitList &availableCaloHitList) const
{
	for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
			endIter != iter ; ++iter)
	{
		const pandora::CaloHit *const pCaloHit(*iter);

		if(PandoraContentApi::IsAvailable(*this, pCaloHit))
			availableCaloHitList.insert(pCaloHit);
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode SurroundingHitsMergingAlgorithm::GetContents(pandora::CaloHitList &caloHitList, pandora::ClusterVector &clusterVector) const
{
	// get available calo hit list
	const pandora::CaloHitList *pCaloHitList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetAvailableCaloHitList(pCaloHitList, caloHitList));

	// get cluster list
	const pandora::ClusterList *pClusterList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	pandora::ClusterList clusterList(pClusterList->begin(), pClusterList->end());

	for(pandora::StringVector::const_iterator iter = m_additionalClusterListNames.begin(), endIter = m_additionalClusterListNames.end() ;
			endIter != iter ; ++iter)
	{
		const pandora::ClusterList *pAdditionalClusterList = NULL;

		if(pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, *iter, pAdditionalClusterList))
		{
			std::cout << "SurroundingHitsMergingAlgorithm: additional cluster list '" << *iter << "' is not available" << std::endl;
			continue;
		}

		clusterList.insert(pAdditionalClusterList->begin(), pAdditionalClusterList->end());
	}

	clusterVector.insert(clusterVector.end(), clusterList.begin(), clusterList.end());

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode SurroundingHitsMergingAlgorithm::FindCaloHitClusterMerging(const pandora::CaloHitList &caloHitList, const pandora::ClusterVector &clusterVector, CaloHitToClusterMap &caloHitToClusterMap) const
{
	for(pandora::CaloHitList::const_iterator hitIter = caloHitList.begin(), hitEndIter = caloHitList.end() ;
			hitEndIter != hitIter ; ++hitIter)
	{
		const pandora::CaloHit *const pCaloHit(*hitIter);

		const pandora::Granularity &granularity(PandoraContentApi::GetGeometry(*this)->GetHitTypeGranularity(pCaloHit->GetHitType()));
		const float maxCaloHitDistance = (granularity <= pandora::FINE) ? m_maxCaloHitDistanceFine : m_maxCaloHitDistanceCoarse;

		float bestDistanceToHit(std::numeric_limits<float>::max());
		const pandora::Cluster *pBestCluster = NULL;

		for(pandora::ClusterVector::const_iterator clusterIter = clusterVector.begin(), clusterEndIter = clusterVector.end() ;
				clusterEndIter != clusterIter ; ++clusterIter)
		{
			const pandora::Cluster *const pCluster(*clusterIter);

			// Get calo hit closest distance approach
			float caloHitDistance(std::numeric_limits<float>::max());

			if(pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetClosestDistanceApproach(pCluster, pCaloHit->GetPositionVector(), caloHitDistance))
				continue;

			if(caloHitDistance < maxCaloHitDistance && caloHitDistance < bestDistanceToHit)
			{
				bestDistanceToHit = caloHitDistance;
				pBestCluster = pCluster;
			}
		}

		if(NULL != pBestCluster)
			caloHitToClusterMap[pCaloHit] = pBestCluster;
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode SurroundingHitsMergingAlgorithm::MergeCaloHits(const CaloHitToClusterMap &caloHitToClusterMap) const
{
	for(CaloHitToClusterMap::const_iterator iter = caloHitToClusterMap.begin(), endIter = caloHitToClusterMap.end() ;
			endIter != iter ; ++iter)
	{
		const pandora::CaloHit *const pCaloHit(iter->first);
		const pandora::Cluster *const pCluster(iter->second);

		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddToCluster(*this, pCluster, pCaloHit));
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode SurroundingHitsMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	m_maxCaloHitDistanceFine = 100.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
		 "MaxCaloHitDistanceFine", m_maxCaloHitDistanceFine));

	m_maxCaloHitDistanceCoarse = 200.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
		 "MaxCaloHitDistanceCoarse", m_maxCaloHitDistanceCoarse));

	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadVectorOfValues(xmlHandle,
			"AdditionalClusterListNames", m_additionalClusterListNames));

	return pandora::STATUS_CODE_SUCCESS;
}

} 

