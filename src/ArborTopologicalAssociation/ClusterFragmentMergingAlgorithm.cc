/*
 *
 * ClusterFragmentMergingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : mer. nov. 18 2015
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


#include "ArborTopologicalAssociation/ClusterFragmentMergingAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"
#include "Helpers/ClusterFitHelper.h"

#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/CaloHitHelper.h"
#include "ArborHelpers/ReclusterHelper.h"

namespace arbor_content
{

  pandora::StatusCode ClusterFragmentMergingAlgorithm::Run()
  {
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    pandora::ClusterVector clusterVector;

    for(pandora::ClusterList::const_iterator iter = pClusterList->begin(), endIter = pClusterList->end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Cluster *const pCluster = *iter;

      if(!this->CanMergeCluster(pCluster))
        continue;

      clusterVector.push_back(pCluster);
    }

    // sort them by inner pseudo layer
    std::sort(clusterVector.begin(), clusterVector.end(), SortingHelper::SortClustersByInnerLayer);

    for(pandora::ClusterVector::reverse_iterator iIter = clusterVector.rbegin(), iEndIter = clusterVector.rend() ;
        iEndIter != iIter ; ++iIter)
    {
      const pandora::Cluster *const pDaughterCluster = *iIter;

      if(NULL == pDaughterCluster)
        continue;

      const unsigned int innerDaughterPseudoLayer = pDaughterCluster->GetInnerPseudoLayer();
      const unsigned int lastDaughterFitPseudoLayer =
          pDaughterCluster->GetOuterPseudoLayer() - innerDaughterPseudoLayer+1 < m_maxFirstPseudoLayerFit ?
              pDaughterCluster->GetOuterPseudoLayer() : innerDaughterPseudoLayer + m_maxFirstPseudoLayerFit;
      const pandora::CartesianVector innerDaughterCentroid(pDaughterCluster->GetCentroid(innerDaughterPseudoLayer));

      pandora::ClusterFitResult daughterFitResult;

      if(pandora::STATUS_CODE_SUCCESS != pandora::ClusterFitHelper::FitLayerCentroids(pDaughterCluster, innerDaughterPseudoLayer, lastDaughterFitPseudoLayer, daughterFitResult))
        continue;

      if(!daughterFitResult.IsFitSuccessful())
        continue;

      const pandora::CartesianVector &daughterFitDirection(daughterFitResult.GetDirection());

      pandora::ClusterVector::reverse_iterator bestParentClusterIter = clusterVector.rend();
      float bestChi2(std::numeric_limits<float>::max());

      for(pandora::ClusterVector::reverse_iterator jIter = clusterVector.rbegin(), jEndIter = clusterVector.rend() ;
          jEndIter != jIter ; ++jIter)
      {
        const pandora::Cluster *pParentCluster = *jIter;

        if(NULL == pParentCluster)
          continue;

        if(pDaughterCluster == pParentCluster)
          continue;

        pandora::CaloHitList parentClusterCaloHitList;
        pParentCluster->GetOrderedCaloHitList().GetCaloHitList(parentClusterCaloHitList);

        pandora::CaloHitList leafCaloHitList;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::ExtractLeafCaloHitList(&parentClusterCaloHitList, leafCaloHitList));

        for(pandora::CaloHitList::iterator leafIter = leafCaloHitList.begin(), leafEndIter = leafCaloHitList.end() ;
            leafEndIter != leafIter ; ++leafIter)
        {
          const arbor_content::CaloHit *pLeafCaloHit = dynamic_cast<const arbor_content::CaloHit *>(*leafIter);

          if(pLeafCaloHit->GetPseudoLayer() > innerDaughterPseudoLayer)
            continue;

          const float distanceToDaughterCluster = (pLeafCaloHit->GetPositionVector() - innerDaughterCentroid).GetMagnitude();

          if(distanceToDaughterCluster > m_maxClusterDistance)
            continue;

          pandora::CaloHitList branchFitCaloHitList;
          pandora::ClusterFitPointList fitPointList;
          pandora::ClusterFitResult branchFitResult;

          if(pandora::STATUS_CODE_SUCCESS != CaloHitHelper::BuildCaloHitList(pLeafCaloHit, BACKWARD_DIRECTION, branchFitCaloHitList,
              std::numeric_limits<unsigned int>::max(), m_maxFitPseudoLayer))
            continue;

          for(pandora::CaloHitList::iterator iter = branchFitCaloHitList.begin(), endIter = branchFitCaloHitList.end() ;
              endIter != iter ; ++iter)
            fitPointList.push_back(pandora::ClusterFitPoint(*iter));

          if(pandora::STATUS_CODE_SUCCESS != pandora::ClusterFitHelper::FitPoints(fitPointList, branchFitResult))
            continue;

          if(!branchFitResult.IsFitSuccessful())
            continue;

          const pandora::CartesianVector clusterDifferencePosition = innerDaughterCentroid - pLeafCaloHit->GetPositionVector();
          const pandora::CartesianVector &branchFitDirection(branchFitResult.GetDirection());
          const float clusterFitAngle = branchFitDirection.GetOpeningAngle(daughterFitDirection);
          const float clusterFitAngle2 = branchFitDirection.GetOpeningAngle(clusterDifferencePosition);

          if(clusterFitAngle > m_maxClusterFitAngle || clusterFitAngle2 > m_maxClusterFitAngle2)
            continue;

          const float normalizedDistance = distanceToDaughterCluster / m_maxClusterDistance;
          const float normalizedFitAngle = clusterFitAngle / m_maxClusterFitAngle;
          const float normalizedFitAngle2 = clusterFitAngle2 / m_maxClusterFitAngle2;

          const float chi2 = normalizedDistance*normalizedDistance
              + normalizedFitAngle*normalizedFitAngle
              + normalizedFitAngle2*normalizedFitAngle2;

          if(chi2 < bestChi2)
          {
            bestChi2 = chi2;
            bestParentClusterIter = jIter;
          }
        }
      }

      // parent cluster found ?
      if(clusterVector.rend() == bestParentClusterIter)
        continue;

      const pandora::Cluster *pParentCluster = *bestParentClusterIter;
      bool performAssociation = true;

      // check for improvement in charged particle cases
      if(!pParentCluster->GetAssociatedTrackList().empty())
      {
        const pandora::TrackList &trackList(pParentCluster->GetAssociatedTrackList());
        float trackEnergySum(0.f);

        for (pandora::TrackList::const_iterator trackIter = trackList.begin(), trackIterEnd = trackList.end() ;
            trackIter != trackIterEnd; ++trackIter)
          trackEnergySum += (*trackIter)->GetEnergyAtDca();

        const float parentClusterEnergy = pParentCluster->GetTrackComparisonEnergy(this->GetPandora());
        const float clusterEnergySum = parentClusterEnergy + pDaughterCluster->GetTrackComparisonEnergy(this->GetPandora());

        const float oldChi = ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), parentClusterEnergy, trackEnergySum);
        const float newChi = ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), clusterEnergySum, trackEnergySum);

        const bool improvesCompatibility = newChi*newChi < oldChi*oldChi || newChi*newChi < m_maxEnergyChi2;

        if(!improvesCompatibility)
          performAssociation = false;
      }

      if(performAssociation)
      {
        // merge clusters
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::MergeAndDeleteClusters(*this, pParentCluster, pDaughterCluster));

        // clean iterator for next iterations
        (*iIter) = NULL;
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool ClusterFragmentMergingAlgorithm::CanMergeCluster(const pandora::Cluster *const pCluster) const
  {
    if(NULL == pCluster)
      return false;

	// ???
    if(!PandoraContentApi::IsAvailable(*this, pCluster))
      return false;

    if(pCluster->GetNCaloHits() < m_minNCaloHits || pCluster->GetNCaloHits() > m_maxNCaloHits)
      return false;

    const unsigned int firstPseudoLayer = pCluster->GetInnerPseudoLayer();
    const unsigned int lastPseudoLayer = pCluster->GetOuterPseudoLayer();

    unsigned int nPseudoLayers = lastPseudoLayer - firstPseudoLayer + 1;

    if(nPseudoLayers < m_minNPseudoLayers || nPseudoLayers >= m_maxNPseudoLayers)
      return false;

    const float nHitPerLayer = static_cast<float>(pCluster->GetNCaloHits()) / static_cast<float>(nPseudoLayers);

    if(nHitPerLayer < m_minNHitPerLayer)
      return false;

    const bool isPhoton(pCluster->IsPhotonFast(this->GetPandora()));

    if(isPhoton)
      return false;

    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ClusterFragmentMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_minNCaloHits = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNCaloHits", m_minNCaloHits));

    m_maxNCaloHits = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNCaloHits", m_maxNCaloHits));

    m_minNPseudoLayers = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNPseudoLayers", m_minNPseudoLayers));

    m_maxNPseudoLayers = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNPseudoLayers", m_maxNPseudoLayers));

    m_minNHitPerLayer = 0.8f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNHitPerLayer", m_minNHitPerLayer));

    m_maxFirstPseudoLayerFit = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxFirstPseudoLayerFit", m_maxFirstPseudoLayerFit));

    m_maxClusterDistance = 150.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterDistance", m_maxClusterDistance));

    m_maxFitPseudoLayer = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxFitPseudoLayer", m_maxFitPseudoLayer));

    m_maxClusterFitAngle = M_PI/12.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterFitAngle", m_maxClusterFitAngle));

    m_maxClusterFitAngle2 = M_PI/12.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterFitAngle2", m_maxClusterFitAngle2));

    m_maxEnergyChi2 = 1.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxEnergyChi2", m_maxEnergyChi2));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

