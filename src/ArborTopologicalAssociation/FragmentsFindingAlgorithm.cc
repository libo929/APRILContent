/// \file FragmentsFindingAlgorithm.cc
/*
 *
 * FragmentsFindingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. juin 30 2016
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


#include "ArborTopologicalAssociation/FragmentsFindingAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/CaloHitHelper.h"
#include "ArborTools/CaloHitMergingTool.h"

namespace arbor_content
{

  pandora::StatusCode FragmentsFindingAlgorithm::Run()
  {
	std::cout << "======= FragmentsFindingAlgorithm ====== " << std::endl;

    // find fragments 
    pandora::ClusterVector removalClusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->FindClusterFragments(removalClusterVector));

    return pandora::STATUS_CODE_SUCCESS;

#if 0
    if(removalClusterVector.empty())
      return pandora::STATUS_CODE_SUCCESS;

    // remove fragments and grab their calo hits
    pandora::CaloHitList removalCaloHitList;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->RemoveClusterFragments(removalClusterVector, removalCaloHitList));

    // merge hits in remaining clusters
    pandora::ClusterVector clusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetMergingClusters(clusterVector));

	if(m_pCaloHitMergingTool != nullptr)
	{
		PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, m_pCaloHitMergingTool->MergeCaloHits(*this, removalCaloHitList, clusterVector));
	}

    return pandora::STATUS_CODE_SUCCESS;
#endif
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFindingAlgorithm::FindClusterFragments(pandora::ClusterVector &removalClusterVector) const
  {
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    if(pClusterList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    pandora::ClusterVector clusterVector(pClusterList->begin(), pClusterList->end());
    pandora::ClusterVector nonFragmentsClusterVector;
    pandora::ClusterVector potentialFragmentsClusterVector;

    // loop over clusters and identify fragments
    for(pandora::ClusterVector::iterator clusterIter = clusterVector.begin(), clusterEndIter = clusterVector.end() ;
        clusterEndIter != clusterIter ; ++clusterIter)
    {
      const pandora::Cluster *const pCluster(*clusterIter);

	  std::cout << " --- cluster: " << pCluster << std::endl;

      // enough hits or energy for a real cluster
      if(pCluster->GetNCaloHits() > m_maxNHitsNonFragments || pCluster->GetHadronicEnergy() > m_maxEnergyNonFragments)
      {
		std::cout << " ------ enough hits or energy for a real cluster " << std::endl;
        continue;
      }

      if(pCluster->PassPhotonId(this->GetPandora()))
	  {
		std::cout << " ------ PassPhotonId" << std::endl;
        continue;
	  }

      // discriminate charged particles
      if(!pCluster->GetAssociatedTrackList().empty())
	  {
		std::cout << " ------ has track " << std::endl;
        continue;
	  }

#if 0
      if( (!ClusterHelper::ContainsHitType(pCluster, pandora::ECAL)) &&
          (!ClusterHelper::ContainsHitType(pCluster, pandora::MUON)) &&
          (pCluster->GetHadronicEnergy() < m_maxHadronicEnergyForAutomaticRemoval))
      {
	    potentialFragmentsClusterVector.push_back(pCluster);
        continue;
      }
#endif

      float meanDensity(0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetMeanDensity(pCluster, meanDensity));

      if(meanDensity > m_maxFragmentDensity)
      {
		std::cout << " ------ density: " << meanDensity << std::endl;
        continue;
      }
        
	  std::cout << " ------ potentialFragments" << std::endl;
	  potentialFragmentsClusterVector.push_back(pCluster);
    }

	removalClusterVector = potentialFragmentsClusterVector;

	std::cout << " --- removalClusterVector: " << removalClusterVector.size() << std::endl;

    for(pandora::ClusterVector::const_iterator fragIter = potentialFragmentsClusterVector.begin(); 
	    fragIter != potentialFragmentsClusterVector.end(); ++fragIter)
	{
		const pandora::Cluster *const pFragment(*fragIter);
		auto pArborFragment = ArborContentApi::Modifiable(dynamic_cast<const arbor_content::ArborCluster*>(pFragment));
		pArborFragment->SetFragment();
	}
		
#if 0
    for(pandora::ClusterVector::const_iterator fragIter = potentialFragmentsClusterVector.begin(), fragEndIter = potentialFragmentsClusterVector.end() ;
        fragEndIter != fragIter ; ++fragIter)
    {
      const pandora::Cluster *const pFragmentCluster(*fragIter);

      pandora::CaloHitList fragmentCaloHitList;
      pFragmentCluster->GetOrderedCaloHitList().FillCaloHitList(fragmentCaloHitList);

      bool foundNearbyCluster(false);
      unsigned int nHitsContact(0);

      for(pandora::ClusterVector::const_iterator iter = clusterVector.begin(), endIter = clusterVector.end() ;
          endIter != iter ; ++iter)
      {
        const pandora::Cluster *const pCluster(*iter);

        if(foundNearbyCluster)
          break;

        if(pCluster == pFragmentCluster)
          continue;

        pandora::CaloHitList clusterCaloHitList;
        pCluster->GetOrderedCaloHitList().FillCaloHitList(clusterCaloHitList);

        for(pandora::CaloHitList::const_iterator hitIter = fragmentCaloHitList.begin(), hitEndIter = fragmentCaloHitList.end() ;
            hitEndIter != hitIter ; ++hitIter)
        {
          const pandora::CaloHit *const pCaloHit(*hitIter);

          if(foundNearbyCluster)
            break;

          const pandora::CartesianVector position(pCaloHit->GetPositionVector());
          const pandora::Granularity granularity(this->GetPandora().GetGeometry()->GetHitTypeGranularity(pCaloHit->GetHitType()));
          const float maxProximityDistance(granularity <= pandora::FINE ? m_maxProximityDistanceFine : m_maxProximityDistanceCoarse);

          for(pandora::CaloHitList::const_iterator hitIter2 = clusterCaloHitList.begin(), hitEndIter2 = clusterCaloHitList.end() ;
              hitEndIter2 != hitIter2 ; ++hitIter2)
          {
            const pandora::CaloHit *const pCaloHit2(*hitIter2);

            const pandora::CartesianVector position2(pCaloHit2->GetPositionVector());
            const float distance((position-position2).GetMagnitude());

            if(distance < maxProximityDistance)
              ++nHitsContact;

            if(nHitsContact > m_maxNHitsProximity)
            {
              foundNearbyCluster = true;
              break;
            }
          }
        }
      }

      if(foundNearbyCluster)
        removalClusterVector.push_back(pFragmentCluster);
    }
#endif

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFindingAlgorithm::RemoveClusterFragments(const pandora::ClusterVector &clusterVector, pandora::CaloHitList &removalCaloHitList) const
  {
    for(pandora::ClusterVector::const_iterator iter = clusterVector.begin(), endIter = clusterVector.end() ;
        endIter != iter ; ++iter)
    {
      pandora::CaloHitList caloHitList;
      (*iter)->GetOrderedCaloHitList().FillCaloHitList(caloHitList);

      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::RemoveConnections(&caloHitList));
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::Delete(*this, *iter));

      removalCaloHitList.insert(removalCaloHitList.begin(), caloHitList.begin(), caloHitList.end());
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFindingAlgorithm::GetMergingClusters(pandora::ClusterVector &clusterVector) const
  {
    // get current cluster list
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    pandora::ClusterList clusterList(pClusterList->begin(), pClusterList->end());

    // get additional cluster lists
    for(pandora::StringVector::const_iterator iter = m_additionalClusterMergingListNames.begin(), endIter = m_additionalClusterMergingListNames.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::ClusterList *pAdditionalClusterList = NULL;

      if(pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, *iter, pAdditionalClusterList))
      {
        std::cout << "FragmentsFindingAlgorithm: additional cluster list '" << *iter << "' is not available" << std::endl;
        continue;
      }
	  else
	  {
		  std::cout << "FragmentsFindingAlgorithm: additional cluster list '" << *iter << "' size: " << pAdditionalClusterList->size() << std::endl;
	  }

      clusterList.insert(clusterList.begin(), pAdditionalClusterList->begin(), pAdditionalClusterList->end());
    }

    clusterVector.insert(clusterVector.end(), clusterList.begin(), clusterList.end());

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode FragmentsFindingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_maxNHitsForAutomaticRemoval = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNHitsForAutomaticRemoval", m_maxNHitsForAutomaticRemoval));

    m_maxEnergyForAutomaticRemoval = 0.1f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxEnergyForAutomaticRemoval", m_maxEnergyForAutomaticRemoval));

    m_maxHadronicEnergyForAutomaticRemoval = 0.5f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxHadronicEnergyForAutomaticRemoval", m_maxHadronicEnergyForAutomaticRemoval));

    m_maxNHitsNonFragments = 200;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNHitsNonFragments", m_maxNHitsNonFragments));

    m_maxEnergyNonFragments = 5.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxEnergyNonFragments", m_maxEnergyNonFragments));

    m_maxFragmentDensity = 0.4f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxFragmentDensity", m_maxFragmentDensity));

    m_maxProximityDistanceFine = 20.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxProximityDistanceFine", m_maxProximityDistanceFine));

    m_maxProximityDistanceCoarse = 40.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxProximityDistanceCoarse", m_maxProximityDistanceCoarse));

    m_maxNHitsProximity = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNHitsProximity", m_maxNHitsProximity));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadVectorOfValues(xmlHandle,
        "AdditionalClusterMergingListNames", m_additionalClusterMergingListNames));

    m_pCaloHitMergingTool = NULL;
    pandora::AlgorithmTool *pAlgorithmTool = NULL;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ProcessAlgorithmTool(*this, xmlHandle,
        "CaloHitMerging", pAlgorithmTool));

    m_pCaloHitMergingTool = dynamic_cast<CaloHitMergingTool*>(pAlgorithmTool);

    return pandora::STATUS_CODE_SUCCESS;
  }


} 

