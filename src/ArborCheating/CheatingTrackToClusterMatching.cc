/**
 *  @file   ArborContent/src/ArborCheating/CheatingTrackToClusterMatching.cc
 * 
 *  @brief  Implementation of the cheating track to cluster matching algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborCheating/CheatingTrackToClusterMatching.h"

using namespace pandora;

namespace arbor_content
{

  StatusCode CheatingTrackToClusterMatching::Run()
  {
    // Read current lists
    const TrackList *pCurrentTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCurrentTrackList));

    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // Clear any existing track - cluster associations
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveCurrentTrackClusterAssociations(*this));

    // Construct a map from mc particle to tracks
    typedef std::map<const MCParticle*, TrackList> TracksPerMCParticle;
    TracksPerMCParticle tracksPerMCParticle;

    for (TrackList::const_iterator iter = pCurrentTrackList->begin(), iterEnd = pCurrentTrackList->end(); iter != iterEnd; ++iter)
    {
      try
      {
        const Track *const pTrack = *iter;
        const MCParticle *const pMCParticle(pTrack->GetMainMCParticle());

        TracksPerMCParticle::iterator itTracksPerMCParticle(tracksPerMCParticle.find(pMCParticle));

        if (tracksPerMCParticle.end() == itTracksPerMCParticle)
        {
          TrackList trackList;
          trackList.insert(pTrack);

          if (!tracksPerMCParticle.insert(TracksPerMCParticle::value_type(pMCParticle, trackList)).second)
            throw StatusCodeException(STATUS_CODE_FAILURE);
        }
        else
        {
          itTracksPerMCParticle->second.insert(pTrack);
        }
      }
      catch (StatusCodeException &)
      {
      }
    }

    // Construct a map from mc particle to clusters
    typedef std::map<const MCParticle*, ClusterList> ClustersPerMCParticle;
    ClustersPerMCParticle clustersPerMCParticle;

    for (ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
      try
      {
        const Cluster *const pCluster = *iter;
        const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));

        ClustersPerMCParticle::iterator itClustersPerMCParticle(clustersPerMCParticle.find(pMCParticle));

        if (clustersPerMCParticle.end() == itClustersPerMCParticle)
        {
          ClusterList clusterList;
          clusterList.insert(pCluster);

          if (!clustersPerMCParticle.insert(ClustersPerMCParticle::value_type(pMCParticle, clusterList)).second)
            throw StatusCodeException(STATUS_CODE_FAILURE);
        }
        else
        {
          itClustersPerMCParticle->second.insert(pCluster);
        }
      }
      catch (StatusCodeException &)
      {
      }
    }

    std::cout << "N mc tracks   = " << tracksPerMCParticle.size() << std::endl;
    std::cout << "N mc clusters = " << clustersPerMCParticle.size() << std::endl;

	// merge clusters for one MC particle
#if 1
    for (ClustersPerMCParticle::iterator iter = clustersPerMCParticle.begin(), iterEnd = clustersPerMCParticle.end(); iter != iterEnd; ++iter)
	{
      //const MCParticle *const pMCParticle = iter->first;
      ClusterList &clusterList = iter->second;

	  if(clusterList.size()<=1) continue;

	  const pandora::Cluster *const firstCluster = *(clusterList.begin());

	  ClusterList::const_iterator itCluster = clusterList.begin();
	  ++ itCluster;

      for (ClusterList::const_iterator itClusterEnd = clusterList.end(); itCluster != itClusterEnd; ++itCluster)
	  {
		  const pandora::Cluster* cluster = *itCluster;
		  PandoraContentApi::MergeAndDeleteClusters(*this, firstCluster, cluster);
		  std::cout << "----> merged a cluster" << std::endl;
	  }

	  clusterList.clear();
	  clusterList.insert(firstCluster);
	}
#endif

	std::set<const Cluster*> matchedClusters;

    // Make the track to cluster associations
    for (TracksPerMCParticle::const_iterator iter = tracksPerMCParticle.begin(), iterEnd = tracksPerMCParticle.end(); iter != iterEnd; ++iter)
    {
      const MCParticle *const pMCParticle = iter->first;
      const TrackList &trackList = iter->second;

      ClustersPerMCParticle::const_iterator itClustersPerMCParticle(clustersPerMCParticle.find(pMCParticle));

      if (clustersPerMCParticle.end() == itClustersPerMCParticle)
        continue;

      const ClusterList &clusterList = itClustersPerMCParticle->second;

      if (trackList.empty() || clusterList.empty())
        continue;

	  //std::cout << "---- MCP energy: " << pMCParticle->GetEnergy() << ", track list size: " << trackList.size() << std::endl;

      for (TrackList::const_iterator itTrack = trackList.begin(), itTrackEnd = trackList.end(); itTrack != itTrackEnd; ++itTrack)
      {
        // If the mc particle is associated with multiple clusters, can only associate to highest energy cluster (clusters should be merged)
		//std::cout << "track energy: " << (*itTrack)->GetEnergyAtDca() << std::endl;
        const Cluster *pHighestEnergyCluster = NULL;
        float highestEnergy(-std::numeric_limits<float>::max());

        for (ClusterList::const_iterator itCluster = clusterList.begin(), itClusterEnd = clusterList.end(); itCluster != itClusterEnd; ++itCluster)
        {
          const float clusterEnergy((*itCluster)->GetHadronicEnergy());

          if (clusterEnergy > highestEnergy)
          {
            highestEnergy = clusterEnergy;
            pHighestEnergyCluster = *itCluster;
          }
        }

        if (NULL == pHighestEnergyCluster)
          throw StatusCodeException(STATUS_CODE_FAILURE);

		if(matchedClusters.end() != matchedClusters.find(pHighestEnergyCluster)) continue;

        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, *itTrack, pHighestEnergyCluster));

		matchedClusters.insert(pHighestEnergyCluster);

		//std::cout << "----> added relation: " << *itTrack << ", " << pHighestEnergyCluster << std::endl;
      }
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode CheatingTrackToClusterMatching::ReadSettings(const TiXmlHandle /*xmlHandle*/)
  {
    return STATUS_CODE_SUCCESS;
  }

} // namespace arbor_content
