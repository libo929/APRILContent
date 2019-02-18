/// \file PointingClusterAssociationNewAlgorithm.cc
/*
 *
 * PointingClusterAssociationNewAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. avr. 9 2015
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

#include "Pandora/AlgorithmHeaders.h"

#include "ArborTopologicalAssociation/PointingClusterAssociationNewAlgorithm.h"

#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/HistogramHelper.h"

#include "ArborApi/ArborContentApi.h"

#include "ArborUtility/EventPreparationAlgorithm.h"

#include "ArborTools/TrackDrivenSeedingTool.h"
#include "ArborObjects/CaloHit.h"

#include <algorithm>

namespace arbor_content
{

  const pandora::Cluster* PointingClusterAssociationNewAlgorithm::GetMainCluster(const pandora::CaloHitVector& caloHitVector)
  {
      //get the cluster which contains most of calo hits in the vector
      std::map<const pandora::Cluster* const, int> clusterTimes;
      
      for(int iHit = 0; iHit < caloHitVector.size(); ++iHit)
      {
      	const pandora::CaloHit* const pCaloHit = caloHitVector.at(iHit);
      
          const arbor_content::CaloHit *const pArborCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(pCaloHit);
      	//std::cout << " --- calo hit: " << pArborCaloHit << ", cluster: " << pArborCaloHit->GetMother() << std::endl;
      
      	const pandora::Cluster* const cluster = pArborCaloHit->GetMother();
      
      	if(cluster != nullptr)
      	{
      	    if(clusterTimes.find(cluster) == clusterTimes.end())
      	    {
      	    	clusterTimes[cluster] = 1;
      	    }
      	    else
      	    {
      	    	clusterTimes[cluster] += 1;
      	    }
      	}
      }
      
      const pandora::Cluster* mainCluster = nullptr;
      int mainClusterTimes = 0;
      
      for(auto iter = clusterTimes.begin(); iter != clusterTimes.end(); ++iter)
      {
      	const pandora::Cluster* const clu = iter->first;
      	int cluTimes = iter->second;
      
      	if(cluTimes > mainClusterTimes)
      	{
      		mainCluster = clu;
      		mainClusterTimes = cluTimes;
      	}
      }
      
      return mainCluster;
  }

  void PointingClusterAssociationNewAlgorithm::GetNearbyClusters(const pandora::Cluster* const cluster, 
		  const pandora::ClusterVector& clusterVector, pandora::ClusterVector& clustersInRange)
  {
      pandora::CartesianVector centroid(0., 0., 0);
	  ClusterHelper::GetCentroid(cluster, centroid);

	  const float distance = 500.;
	  const mlpack::math::Range range(0., distance);

      arma::mat testPoint(3, 1);
	  testPoint.col(0)[0] = centroid.GetX();
	  testPoint.col(0)[1] = centroid.GetY();
	  testPoint.col(0)[2] = centroid.GetZ();
	  
	  mlpack::range::RangeSearch<> rangeSearch(m_clusterCentroidsMatrix);
      std::vector<std::vector<size_t> > resultingNeighbors;
      std::vector<std::vector<double> > resultingDistances;
      rangeSearch.Search(testPoint, range, resultingNeighbors, resultingDistances);

      std::vector<size_t>& neighbors = resultingNeighbors.at(0);
      std::vector<double>& distances = resultingDistances.at(0);
	  
      for(size_t j=0; j < neighbors.size(); ++j)
      {
      	size_t neighbor = neighbors.at(j);
      	double hitsDist = distances.at(j);

		clustersInRange.push_back( clusterVector.at(neighbor) );
	  }

	  std::cout << "------------ cluster: " << cluster << ", energy: " << cluster->GetHadronicEnergy() << ", nearby clusters: " 
		  << clustersInRange.size() << std::endl;

	  for(int i = 0; i < clustersInRange.size(); ++i)
	  {
		  auto pCluster = clustersInRange.at(i);
		  std::cout << "   cluster " << i << ": " << clustersInRange.at(i) << ", nhits: " << 
			 pCluster->GetNCaloHits() << ", Ehad: " << pCluster->GetHadronicEnergy() << std::endl;
	  }
  }

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::Run()
  {
    // get candidate clusters for association
    pandora::ClusterVector clusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetEligibleClusters(clusterVector));

	//std::cout << "EligibleClusters: " << clusterVector.size() << std::endl;

    // sort them by inner layer
    std::sort(clusterVector.begin(), clusterVector.end(), SortingHelper::SortClustersByInnerLayer);

	// ------------------
	std::vector<pandora::CartesianVector> clusterCentroids;

#if 1
	for(auto clu : clusterVector)
	{
		pandora::CartesianVector centroid(0., 0., 0);
		ClusterHelper::GetCentroid(clu, centroid);
		clusterCentroids.push_back(centroid);
	}

	CaloHitRangeSearchHelper::FillMatixByPoints(clusterCentroids, m_clusterCentroidsMatrix);
#endif

	// -----------------
	TrackCaloHitVector& trackCaloHits = TrackDrivenSeedingTool::GetTrackAndInitCaloHits();

	pandora::TrackVector trackVector;
	pandora::TrackVector trackWithoutCaloHitVector;

	for(auto trackCaloHitsIter = trackCaloHits.begin(); trackCaloHitsIter != trackCaloHits.end(); ++trackCaloHitsIter)
	{
		auto pTrack = trackCaloHitsIter->first;
		auto& caloHits = trackCaloHitsIter->second;

#if 0
		std::cout << " --- Track: " << pTrack << ", p: " << pTrack->GetMomentumAtDca().GetMagnitude() 
			<< ", calo hits: " << caloHits.size() << std::endl;
#endif

		if(caloHits.size() > 0)
		{
			trackVector.push_back(pTrack);
		}
		else
		{
			trackWithoutCaloHitVector.push_back(pTrack);
		}
	}

    std::sort(trackVector.begin(), trackVector.end(), SortingHelper::SortTracksByMomentum);
    std::sort(trackWithoutCaloHitVector.begin(), trackWithoutCaloHitVector.end(), SortingHelper::SortTracksByMomentum);

	for(int iTrack = trackVector.size() - 1; iTrack >= 0; --iTrack)
	{
		auto pTrack = trackVector.at(iTrack);
		pandora::CaloHitVector& caloHits = trackCaloHits.find(pTrack)->second;

		std::cout << " **************************** track p: " << pTrack->GetMomentumAtDca().GetMagnitude() << std::endl;

		const pandora::Cluster* mainCluster = GetMainCluster(caloHits);
		//std::cout << "main cluster: " << mainCluster << std::endl;

		if(mainCluster == nullptr) continue;

#if 0
		// test
        const pandora::MCParticle *pTrackMCParticle = nullptr;
        const pandora::MCParticle *pClusterMCParticle = nullptr;

        try
        {
		    pTrackMCParticle = pandora::MCParticleHelper::GetMainMCParticle( pTrack );
        }
        catch (pandora::StatusCodeException &)
        {
			continue;
        }

        try
        {
		    pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle( mainCluster );
        }
        catch (pandora::StatusCodeException &)
        {
			continue;
        }


		if(pTrackMCParticle == nullptr || pClusterMCParticle == nullptr) continue;


		///
	    std::vector<float> vars;
	    vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	    vars.push_back( pTrack->GetMomentumAtDca().GetMagnitude() );
		vars.push_back( float(pTrack->IsProjectedToEndCap()) );
		vars.push_back( float(caloHits.size()) );
		vars.push_back( float(pTrackMCParticle == pClusterMCParticle) );
	
		HistogramManager::CreateFill("PointingClusterAssociation_TrackCheck", "evtNum:trackMomentum:reachEndcap:nCaloHit:isRightMatch", vars);
#endif

		pandora::ClusterVector nearbyClusters;
		GetNearbyClusters(mainCluster, clusterVector, nearbyClusters);

#if 0
		std::cout << "track: " << pTrack << ", " << pTrack << ", p: " << pTrack->GetMomentumAtDca().GetMagnitude() 
			<< ", calo hits: " << caloHits.size() << std::endl;
#endif
	}


#if 0
    ClusterToClusterMap clusterToClusterMap;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->FindClustersToMerge(clusterVector, clusterToClusterMap));
	std::cout << "ClustersToMerge: " << clusterToClusterMap.size() << std::endl;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::MergeClusters(*this, clusterToClusterMap));

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "<<<cluster number: " << pClusterList->size() << std::endl;
#endif

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::GetEligibleClusters(pandora::ClusterVector &clusterVector) const
  {
	clusterVector.clear();

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	std::cout << ">>>cluster number: " << pClusterList->size() << std::endl;

    if(pClusterList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    for(pandora::ClusterList::const_iterator clusterIter = pClusterList->begin(), clusterEndIter = pClusterList->end() ;
        clusterEndIter != clusterIter ; ++clusterIter)
    {
      const pandora::Cluster *const pCluster = *clusterIter;

      if(!this->CanMergeCluster(pCluster))
        continue;

      clusterVector.push_back(pCluster);
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool PointingClusterAssociationNewAlgorithm::CanMergeCluster(const pandora::Cluster *const pCluster) const
  {
    if(NULL == pCluster)
      return false;

    if(!PandoraContentApi::IsAvailable(*this, pCluster))
      return false;

    if(pCluster->GetNCaloHits() < m_minNCaloHits || pCluster->GetNCaloHits() > m_maxNCaloHits)
      return false;

    const unsigned int nPseudoLayers(pCluster->GetOrderedCaloHitList().size());

    if(nPseudoLayers < m_minNPseudoLayers || nPseudoLayers >= m_maxNPseudoLayers)
      return false;

    if(m_discriminatePhotonPid && pCluster->PassPhotonId(this->GetPandora()))
      return false;

    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::FindClustersToMerge(const pandora::ClusterVector &clusterVector, ClusterToClusterMap &clusterToClusterMap) const
  {
    for(pandora::ClusterVector::const_iterator iter = clusterVector.begin(), endIter = clusterVector.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Cluster *const pDaughterCluster = *iter;

      //if( ! pDaughterCluster->GetAssociatedTrackList().empty() )
      //  continue;

      const pandora::Cluster *pBestParentCluster = NULL;

      if(pandora::STATUS_CODE_SUCCESS != this->FindBestParentCluster(pDaughterCluster, clusterVector, pBestParentCluster))
        continue;

      if(NULL == pBestParentCluster)
        continue;

      clusterToClusterMap[pDaughterCluster] = pBestParentCluster;
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::FindBestParentCluster(const pandora::Cluster *const pDaughterCluster, const pandora::ClusterVector &clusterVector,
      const pandora::Cluster *&pBestParentCluster) const
  {
    pBestParentCluster = NULL;

    if(NULL == pDaughterCluster)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    pandora::CartesianVector innerPosition(0.f, 0.f, 0.f), backwardDirection(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetClusterBackwardDirection(pDaughterCluster, backwardDirection, innerPosition));
    const unsigned int innerPseudoLayer(PandoraContentApi::GetPlugins(*this)->GetPseudoLayerPlugin()->GetPseudoLayer(innerPosition));

    const pandora::Granularity granularity(this->GetPandora().GetGeometry()->GetHitTypeGranularity(pDaughterCluster->GetInnerLayerHitType()));
    const float maxClusterDistance(granularity <= pandora::FINE ? m_maxBackwardDistanceFine : m_maxBackwardDistanceCoarse);
    const unsigned int maxPseudoLayerDifference(m_maxBackwardPseudoLayer);
    float bestClusterCosineAngle(std::numeric_limits<float>::min());

    for(pandora::ClusterVector::const_reverse_iterator jIter = clusterVector.rbegin(), jEndIter = clusterVector.rend() ;
        jEndIter != jIter ; ++jIter)
    {
      const pandora::Cluster *const pCluster(*jIter);

      if(NULL == pCluster)
        continue;

      if(pCluster == pDaughterCluster)
        continue;

      if(pCluster->GetAssociatedTrackList().empty())
        continue;
	  else
		  std::cout << "---cluster tracks: " << pCluster->GetAssociatedTrackList().size() << std::endl;

      const pandora::Track *pTrack((*pCluster->GetAssociatedTrackList().begin()));
      const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());
      const pandora::CartesianVector trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());
      const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
      const pandora::Helix helix(trackProjection, trackMomentum, pTrack->GetCharge(), bField);

      // compute parent cluster end point using cluster fit
      pandora::CartesianVector centroid(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

      const pandora::CartesianVector outerCentroid(pCluster->GetCentroid(pCluster->GetOuterPseudoLayer()));
      const unsigned int outerParentPseudoLayer(pCluster->GetOuterPseudoLayer());
      const unsigned int innerParentPseudoLayer(pCluster->GetInnerPseudoLayer());
      const unsigned int clusterPseudoLayerExtension(outerParentPseudoLayer-innerParentPseudoLayer);
      const unsigned int nFitPseudoLayers(clusterPseudoLayerExtension >= m_nBackwardLayersFit ? m_nBackwardLayersFit : clusterPseudoLayerExtension);

      pandora::ClusterFitResult clusterFitResult;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::ClusterFitHelper::FitEnd(pCluster, nFitPseudoLayers, clusterFitResult));
      const pandora::CartesianVector clusterDirection(clusterFitResult.GetDirection());

      pandora::CartesianVector clusterEndPoint(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnLine(centroid, clusterDirection, outerCentroid, clusterEndPoint));

      pandora::CartesianVector trackEndPointPosition(0.f, 0.f, 0.f), trackEndPointMomentum(0.f, 0.f, 0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnHelix(helix, clusterEndPoint, trackEndPointPosition));
      trackEndPointMomentum = helix.GetExtrapolatedMomentum(trackEndPointPosition);

      // possible parent if :
      // 1) cluster distance if OK AND angle between fits is ok (cut on cosine)
      // 2) cluster distance if OK AND angle between fits is compatible (cut relaxed) AND dca between fits is OK

      // check pseudo layer cluster separation
      const unsigned int endPseudoLayer(PandoraContentApi::GetPlugins(*this)->GetPseudoLayerPlugin()->GetPseudoLayer(clusterEndPoint));
      const unsigned int pseudoLayerDifference(std::max(innerPseudoLayer, endPseudoLayer) - std::min(innerPseudoLayer, endPseudoLayer));

      if(endPseudoLayer >= innerPseudoLayer || pseudoLayerDifference > maxPseudoLayerDifference)
        continue;

      // distance between clusters
      const pandora::CartesianVector clusterDifferenceDirection(innerPosition-clusterEndPoint);
      const float clusterDistance(clusterDifferenceDirection.GetMagnitude());

      if(clusterDistance > maxClusterDistance)
        continue;

      // compute cluster-to-cluster properties
      const float clusterFitCosineAngle(clusterDirection.GetCosOpeningAngle(backwardDirection * -1.));
      const float clusterCosineAngle(trackEndPointMomentum.GetCosOpeningAngle(clusterDifferenceDirection));

      float clusterFitDca(0.f);
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetClosestDistanceBetweenLines(clusterEndPoint, clusterDirection, innerPosition, backwardDirection, clusterFitDca));

      bool possibleAssociation(false);

      // contained in small angle
      if(clusterFitCosineAngle > m_minClusterFitCosOpeningAngle2 && clusterCosineAngle > m_minClusterCosOpeningAngle)
      {
        possibleAssociation = true;
      }
      // contained in larger angle but short dca
      else if(clusterFitDca < m_maxClusterFitDca && clusterFitCosineAngle > m_minClusterFitCosOpeningAngle && clusterCosineAngle > m_minClusterCosOpeningAngle)
      {
        possibleAssociation = true;
      }

      if(possibleAssociation)
      {
        float oldChi(0.f), newChi(0.f);

        if(pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetChiClusterMerging(this->GetPandora(), pCluster, pDaughterCluster, oldChi, newChi))
          continue;

        const bool angleTightened(clusterCosineAngle > bestClusterCosineAngle);
        const bool chi2Compatible(newChi*newChi < oldChi*oldChi || newChi*newChi < m_chi2AssociationCut);

        if(angleTightened && chi2Compatible)
        {
          bestClusterCosineAngle = clusterCosineAngle;
          pBestParentCluster = pCluster;
        }
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::GetClusterBackwardDirection(const pandora::Cluster *const pCluster, pandora::CartesianVector &backwardDirection, pandora::CartesianVector &innerPosition) const
  {
    pandora::CartesianVector centroid(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

    const pandora::CartesianVector innerCentroid(pCluster->GetCentroid(pCluster->GetInnerPseudoLayer()));

    pandora::ClusterFitResult clusterFitResult;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::ClusterFitHelper::FitStart(pCluster, m_nBackwardLayersFit, clusterFitResult));
    const pandora::CartesianVector clusterDirection(clusterFitResult.GetDirection());

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, GeometryHelper::GetProjectionOnLine(centroid, clusterDirection, innerCentroid, innerPosition));
    backwardDirection = clusterDirection * -1.f;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode PointingClusterAssociationNewAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_discriminatePhotonPid = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DiscriminatePhotonPid", m_discriminatePhotonPid));

    m_allowNeutralParentMerging = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "AllowNeutralParentMerging", m_allowNeutralParentMerging));

    m_minNCaloHits = 0;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNCaloHits", m_minNCaloHits));

    m_maxNCaloHits = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNCaloHits", m_maxNCaloHits));

    m_minNPseudoLayers = 2;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinNPseudoLayers", m_minNPseudoLayers));

    m_maxNPseudoLayers = std::numeric_limits<unsigned int>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxNPseudoLayers", m_maxNPseudoLayers));

    m_chi2AssociationCut = 1.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "Chi2AssociationCut", m_chi2AssociationCut));

    m_nBackwardLayersFit = 6;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NBackwardLayersFit", m_nBackwardLayersFit));

    m_maxBackwardAngle = 0.4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardAngle", m_maxBackwardAngle));

    m_maxBackwardDistanceFine = 300.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardDistanceFine", m_maxBackwardDistanceFine));

    m_maxBackwardDistanceCoarse = 1000.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardDistanceCoarse", m_maxBackwardDistanceCoarse));

    m_maxBackwardPseudoLayer = 20;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxBackwardPseudoLayer", m_maxBackwardPseudoLayer));

    m_minParentClusterBackwardNHits = 5;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinParentClusterBackwardNHits", m_minParentClusterBackwardNHits));

    m_maxClusterFitDca = 25.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterFitDca", m_maxClusterFitDca));

    m_minClusterCosOpeningAngle = 0.7;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterCosOpeningAngle", m_minClusterCosOpeningAngle));

    m_minClusterFitCosOpeningAngle = 0.7;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterFitCosOpeningAngle", m_minClusterFitCosOpeningAngle));

    m_minClusterFitCosOpeningAngle2 = 0.8;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterFitCosOpeningAngle2", m_minClusterFitCosOpeningAngle2));

    return pandora::STATUS_CODE_SUCCESS;
  }


} 

