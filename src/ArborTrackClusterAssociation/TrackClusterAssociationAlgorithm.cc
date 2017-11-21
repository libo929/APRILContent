/// \file TrackClusterAssociationAlgorithm.cc
/*
 *
 * TrackClusterAssociationAlgorithm.cc source template automatically generated by a class generator
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

#include "ArborTrackClusterAssociation/TrackClusterAssociationAlgorithm.h"
#include "ArborHelpers/ReclusterHelper.h"


bool SortTracksByEnergy(const pandora::Track *const pLhs, const pandora::Track *const pRhs)
{
   	return (pLhs->GetEnergyAtDca() > pRhs->GetEnergyAtDca());
}

namespace arbor_content
{

  pandora::StatusCode TrackClusterAssociationAlgorithm::Run()
  {
    // extract tracks and clusters eligible for track-cluster associations
    pandora::TrackList trackList; pandora::ClusterList clusterList;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ExtractAssociationLists(clusterList, trackList));

    if(trackList.empty() || clusterList.empty())
      return pandora::STATUS_CODE_SUCCESS;

	std::cout << "----> clusterList size: " << clusterList.size() << std::endl;
	//std::cout << "trackList size: " << trackList.size() << std::endl;
    // reset the track-cluster associations
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveCurrentTrackClusterAssociations(*this));

    AssociationMap associationMap;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->BuildPossibleAssociations(clusterList, trackList, associationMap));
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->PerformAssociations(associationMap));

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::ExtractAssociationLists(pandora::ClusterList &clusterList, pandora::TrackList &trackList)
  {
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "-------> the cluster size of list: " << pClusterList->size() << std::endl;

    if(pTrackList->empty() || pClusterList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    // filter clusters
    for(pandora::ClusterList::const_iterator iter = pClusterList->begin() , endIter = pClusterList->end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Cluster *const pCluster = *iter;

      if( ! PandoraContentApi::IsAvailable(*this, pCluster) )
        continue;

      const unsigned int innerPseudoLayer(pCluster->GetInnerPseudoLayer());

      if( innerPseudoLayer > m_maxClusterInnerPseudoLayer )
        continue;

      clusterList.insert(pCluster);
    }

    // filter tracks
    for(pandora::TrackList::const_iterator trackIter = pTrackList->begin() , trackEndIter = pTrackList->end() ;
        trackEndIter != trackIter ; ++trackIter)
    {
      const pandora::Track *const pTrack = *trackIter;

      if(!PandoraContentApi::IsAvailable(*this, pTrack))
        continue;

      if(!pTrack->ReachesCalorimeter())
        continue;

      if(!pTrack->CanFormPfo())
        continue;

      trackList.insert(pTrack);
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::BuildPossibleAssociations(const pandora::ClusterList &clusterList, const pandora::TrackList &trackList,
      AssociationMap &associationMap)
  {
    //AssociationConstraintsMap contraintsMap;
    MultiAssociationMap multiAssociationMap;

    for(pandora::TrackList::const_iterator trackIter = trackList.begin() , trackEndIter = trackList.end() ;
        trackEndIter != trackIter ; ++trackIter)
    {
      const pandora::Track *const pTrack = *trackIter;
	  //std::cout << "-------> track energy: " << pTrack->GetEnergyAtDca() << std::endl;

      const pandora::Cluster *pBestCluster = NULL;
      float bestCompatibility(std::numeric_limits<float>::max());
      AssociationConstraints bestConstraints;

      for(pandora::ClusterList::const_iterator iter = clusterList.begin() , endIter = clusterList.end() ;
          endIter != iter ; ++iter)
      {
        const pandora::Cluster *const pCluster = *iter;

		//double trackEnergy = pTrack->GetEnergyAtDca();
		//double clusterEnergy = pCluster->GetElectromagneticEnergy();
		//bool canPrint = fabs(trackEnergy - 3.2) < 0.1 && fabs(clusterEnergy - 3.32) < 0.1;
		//bool canPrint = fabs(trackEnergy - 3.2) < 0.1;

        if( ! this->PassesInitialCuts(pCluster, pTrack) )
		{
		  //if(canPrint) std::cout << "track: " << trackEnergy << ", cluster: " << clusterEnergy << ", not pass init" << std::endl;
          continue;
		}

        AssociationConstraints constraints;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CalculateAssociationConstraints(pCluster, pTrack, constraints));

        if( ! this->FitConstraints(constraints) )
		{
		  //if(canPrint) std::cout << "track: " << trackEnergy << ", cluster: " << clusterEnergy << ", not pass Fit" << std::endl;
          continue;
		}

        if(constraints.m_compatibility < bestCompatibility)
        {
          bestCompatibility = constraints.m_compatibility;
          bestConstraints = constraints;
          pBestCluster = pCluster;
        }
      }

      if(NULL != pBestCluster)
      {
        if(m_allowMultiAssociations)
        {
          associationMap[pTrack] = pBestCluster;
          continue;
        }

        MultiAssociationMap::iterator iter = multiAssociationMap.find(pBestCluster);

        if(multiAssociationMap.end() != iter)
        {
          AssociationMap::iterator iter2 = associationMap.find(iter->second);

          if(iter2 == associationMap.end())
          {
            associationMap[pTrack] = pBestCluster;
            multiAssociationMap[pBestCluster] = pTrack;
            continue;
          }

          pandora::TrackList chiTrackList, newChiTrackList;
          chiTrackList.insert(iter->second);
          newChiTrackList.insert(pTrack);

          const float chi = ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pBestCluster, chiTrackList);
          const float newChi = ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pBestCluster, newChiTrackList);

          if(chi*chi > newChi*newChi)
          {
            multiAssociationMap[pBestCluster] = pTrack;
            associationMap.erase(iter2);
            associationMap[pTrack] = pBestCluster;
          }
        }
        else
        {
          associationMap[pTrack] = pBestCluster;
          multiAssociationMap[pBestCluster] = pTrack;
        }
      }
	  else
	  {
		std::cout << "-------> track energy: " << pTrack->GetEnergyAtDca() << ", no cluster" << std::endl;

		// if the track energy is greater than a prescribed value but has no associated cluster, then
		// try to check nearby photon candidates ???
		if(!m_usePhotonClusters) continue;
    
		const pandora::ClusterList *pPhotonList = NULL;
		std::string photonListName("PhotonClusters");
    
        if(pandora::STATUS_CODE_SUCCESS != PandoraContentApi::GetList(*this, photonListName, pPhotonList))
        {
			std::cout << "TrackClusterAssociationAlgorithm: cluster list PhotonClusters is not available" << std::endl;
		}

		//if(pPhotonList!=NULL) std::cout << "=============> photon cluster number: " << pPhotonList->size() << std::endl;
	  }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::PerformAssociations(const AssociationMap &associationMap)
  {
#if 1	  
    //std::cout << "TrackClusterAssociationAlgorithm::PerformAssociations" << std::endl;

	pandora::TrackVector trackVector;

    for(AssociationMap::const_iterator iter = associationMap.begin() , endIter = associationMap.end() ;
        endIter != iter ; ++iter)
    { 
		const pandora::Track *const pTrack(iter->first);
		trackVector.push_back(pTrack);
	}

	std::sort(trackVector.begin(), trackVector.end(), SortTracksByEnergy);

	//std::cout << "# of pfo track: " << trackVector.size() << std::endl;

    for (pandora::TrackVector::const_iterator trackIter = trackVector.begin(), trackIterEnd = trackVector.end(); trackIter != trackIterEnd; ++trackIter)
    {
        //const pandora::Track *const pTrack = *trackIter;

        //std::cout << "Pandora: track energy: " << pTrack->GetEnergyAtDca() << std::endl; 
	}
#endif
    for(AssociationMap::const_iterator iter = associationMap.begin() , endIter = associationMap.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::Track *const pTrack(iter->first);
      const pandora::Cluster *const pCluster(iter->second);
	  //std::cout << "--track energy: " << pTrack->GetEnergyAtDca() << " cluster: " << pCluster->GetElectromagneticEnergy() << std::endl;

      if((NULL == pTrack) || (NULL == pCluster)) {
		  //if(pTrack!=NULL) std::cout << "track has no cluster, track energy:  " << pTrack->GetEnergyAtDca() << std::endl;
		  //if(pCluster!=NULL) std::cout << "cluster has no track, cluster energy:  " << std::endl;
        continue;
	  }

      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, pTrack, pCluster));
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::ComputeInnerCentroid(const pandora::Cluster *const pCluster, const pandora::Track *const pTrack, pandora::CartesianVector &innerCentroid) const
  {
    const unsigned int innerPseudoLayer(pCluster->GetInnerPseudoLayer());
    const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());
    const pandora::CartesianVector trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());
    const pandora::OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());
    unsigned int nDof(0);

    for(pandora::OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), endIter = orderedCaloHitList.end() ; endIter != iter ; ++iter)
    {
      if( ( iter->first - innerPseudoLayer ) > m_nFirstClusterPseudoLayer )
        break;

      for(pandora::CaloHitList::const_iterator hiter = iter->second->begin(), endhiter = iter->second->end() ; endhiter != hiter ; ++hiter)
      {
        const pandora::CaloHit *const pCaloHit(*hiter);

        const float openingAngle(trackMomentum.GetOpeningAngle(pCaloHit->GetPositionVector() - trackProjection));

        if(openingAngle > m_maxTrackClusterAngle)
          continue;

        ++nDof;
        innerCentroid += pCaloHit->GetPositionVector();
      }
    }

    //	std::cout << " ==> nDof = " << nDof <<  std::endl;

    if(0 == nDof)
      return pandora::STATUS_CODE_FAILURE;

    innerCentroid *= (1.f / nDof);

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool TrackClusterAssociationAlgorithm::PassesInitialCuts(const pandora::Cluster *const pCluster, const pandora::Track *const pTrack) const
  {
    if( NULL == pCluster || NULL == pTrack)
      return false;

    const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());
    const pandora::CartesianVector trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());

    pandora::CartesianVector innerCentroid(0.f, 0.f, 0.f);

    if(pandora::STATUS_CODE_SUCCESS != this->ComputeInnerCentroid(pCluster, pTrack, innerCentroid))
	{
		//std::cout << "ComputeInnerCentroid failed" << std::endl;
		//std::cout << "pCluster: " << pCluster->GetElectromagneticEnergy() << " track: " << pTrack->GetEnergyAtDca() << " ComputeInnerCentroid failed" << std::endl;
      return false;
	}

    // get b field and track helix
    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
    const pandora::Helix helix(trackProjection, trackMomentum, pTrack->GetCharge(), bField);

    //	pandora::CartesianVector projectionOnHelix(0.f, 0.f, 0.f);
    //
    //	if(pandora::STATUS_CODE_SUCCESS != GeometryHelper::GetProjectionOnHelix(helix, innerCentroid, projectionOnHelix))
    //		return false;

    pandora::CartesianVector distanceToHelix(0.f, 0.f, 0.f);
    helix.GetDistanceToPoint(innerCentroid, distanceToHelix);

    if( distanceToHelix.GetZ() > m_maxDistanceToHelix )
	{
		//std::cout << "distanceToHelix.Z: " << distanceToHelix.GetZ() << ", m_maxDistanceToHelix: " << m_maxDistanceToHelix << std::endl;
		//std::cout << "pCluster: " << pCluster->GetElectromagneticEnergy() << " track: " << pTrack->GetEnergyAtDca() << " ComputeInnerCentroid failed" << std::endl;
      return false;
	}

    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::CalculateAssociationConstraints(const pandora::Cluster *const pCluster, const pandora::Track *const pTrack, AssociationConstraints &constraints)
  {
    if( NULL == pCluster || NULL == pTrack)
      return pandora::STATUS_CODE_INVALID_PARAMETER;

    const pandora::CartesianVector trackMomentum(pTrack->GetTrackStateAtCalorimeter().GetMomentum());
    const pandora::CartesianVector trackProjection(pTrack->GetTrackStateAtCalorimeter().GetPosition());

    pandora::CartesianVector innerCentroid(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ComputeInnerCentroid(pCluster, pTrack, innerCentroid));

    // get b field and track helix
    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));
    const pandora::Helix helix(trackProjection, trackMomentum, pTrack->GetCharge(), bField);

    pandora::CartesianVector distanceToHelix(0.f, 0.f, 0.f);
    helix.GetDistanceToPoint(innerCentroid, distanceToHelix);

    pandora::TrackList trackList;
    trackList.insert(pTrack);

    const float openingAngle(trackMomentum.GetOpeningAngle(innerCentroid - trackProjection));
    const float chi = ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pCluster, trackList);

    const float energyCompatibility( m_useEnergyCompatibility ? chi*chi : 1.f );
    const float distanceCompatibility( distanceToHelix.GetZ() / m_maxDistanceToHelix );

    // compatibility as a distance to helix combined with energy information
    const float compatibility( energyCompatibility * distanceCompatibility );

    constraints.m_pCluster = pCluster;
    constraints.m_chi2 = chi*chi;
    constraints.m_trackClusterAngle = openingAngle;
    constraints.m_distanceToHelix = distanceToHelix.GetZ();
    constraints.m_compatibility = compatibility;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool TrackClusterAssociationAlgorithm::FitConstraints(const AssociationConstraints &constraints) const
  {
    if( constraints.m_distanceToHelix > m_maxDistanceToHelix )
      return false;

    if( constraints.m_trackClusterAngle > m_maxTrackClusterAngle )
      return false;

    return true;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  bool TrackClusterAssociationAlgorithm::SortByBestCompatibility(const AssociationConstraints &lhs, const AssociationConstraints &rhs)
  {
    return lhs.m_compatibility > rhs.m_compatibility;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode TrackClusterAssociationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_maxClusterInnerPseudoLayer = 3;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterInnerPseudoLayer", m_maxClusterInnerPseudoLayer));

    m_nFirstClusterPseudoLayer = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NFirstClusterPseudoLayer", m_nFirstClusterPseudoLayer));

    m_maxDistanceToHelix = 25.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxDistanceToHelix", m_maxDistanceToHelix));

    m_maxTrackClusterAngle = 0.6f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackClusterAngle", m_maxTrackClusterAngle));

    m_allowMultiAssociations = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "AllowMultiAssociations", m_allowMultiAssociations));

    m_useEnergyCompatibility = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "UseEnergyCompatibility", m_useEnergyCompatibility));

	m_usePhotonClusters = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "UsePhotonClusters", m_usePhotonClusters));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

