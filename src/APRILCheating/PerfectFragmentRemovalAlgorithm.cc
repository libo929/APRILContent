/**
 *  @file   APRILContent/src/APRILCheating/PerfectFragmentRemovalAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating clustering algorithm class
 * 
 *  $Log: $
 *  @author J. Marshall.
 *  Copied from https://github.com/PandoraPFA/LCContent sources
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/PerfectFragmentRemovalAlgorithm.h"

using namespace pandora;

namespace april_content
{

  PerfectFragmentRemovalAlgorithm::PerfectFragmentRemovalAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectFragmentRemovalAlgorithm::Run()
  {
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
    MCParticleToClusterListMap mcParticleToClusterListMap;

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
    {
      try
      {
        const Cluster *const pCluster = *clusterIter;

		// assume we are at the stage that there are neutral and charged clusters, and fragments
        if (!PandoraContentApi::IsAvailable(*this, pCluster))
          continue;

        this->SimpleMCParticleCollection(pCluster, mcParticleToClusterListMap);
      }
      catch (StatusCodeException &)
      {
      }
    }

	std::cout << "mcParticleToClusterListMap size: " << mcParticleToClusterListMap.size() << std::endl;
    this->MergeClusters(mcParticleToClusterListMap);

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectFragmentRemovalAlgorithm::SimpleMCParticleCollection(const Cluster *const pCluster, 
		  MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
    const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
    this->AddToClusterListMap(pCluster, pMCParticle, mcParticleToClusterListMap);
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void PerfectFragmentRemovalAlgorithm::AddToClusterListMap(const Cluster *const pClusterToAdd, const MCParticle *const pMCParticle,
      MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
    MCParticleToClusterListMap::iterator iter(mcParticleToClusterListMap.find(pMCParticle));

    if (mcParticleToClusterListMap.end() == iter)
    {
      ClusterList *const pClusterList = new ClusterList();
      pClusterList->push_back(pClusterToAdd);
      (void) mcParticleToClusterListMap.insert(MCParticleToClusterListMap::value_type(pMCParticle, pClusterList));
    }
    else
    {
      iter->second->push_back(pClusterToAdd);
    }
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectFragmentRemovalAlgorithm::MergeClusters(const MCParticleToClusterListMap &mcParticleToClusterListMap) const
  {
	// make a map for (MCP, track)
	//std::cout << "try to make a map for MCP and track " << std::endl;
    const TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    TrackList localTrackList(pTrackList->begin(), pTrackList->end());
    MCParticleToTrackMap mcParticleToTrackMap;

    for (TrackList::const_iterator trackIter = localTrackList.begin(), trackIterEnd = localTrackList.end(); 
			trackIter != trackIterEnd; ++trackIter)
    {
        const Track *const pTrack = *trackIter;
		//std::cout << "track: " << pTrack << std::endl;
       
		try
        {
           const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pTrack));
	    
		   //std::cout << "track: " << pTrack << ", pMCParticle: " << pMCParticle << std::endl;
    
		   MCParticleToTrackMap::const_iterator iter = mcParticleToTrackMap.find(pMCParticle);

		   if(iter == mcParticleToTrackMap.end())
		   {
		   	mcParticleToTrackMap.insert(MCParticleToTrackMap::value_type(pMCParticle, pTrack));
		   }
		}
        catch (StatusCodeException &)
        {
			continue;
        }
	}

	//std::cout << "made a map for MCP and track " << std::endl;

	/////////

    for (MCParticleToClusterListMap::const_iterator mapIter = mcParticleToClusterListMap.begin(), 
	    mapIterEnd = mcParticleToClusterListMap.end(); mapIter != mapIterEnd; ++mapIter)
    {
      ClusterList *const pClusterList = mapIter->second;
	  if(pClusterList->size()==0) continue;

	  int nChargedCluster = 0;

      if (pClusterList->size()>1)
      {
        const Cluster *mainCluster = NULL;

		for(ClusterList::const_iterator clusterIter = pClusterList->begin(); clusterIter != pClusterList->end(); ++clusterIter)
		{
			const Cluster* cluster = *clusterIter;

			const TrackList& associatedTracks = cluster->GetAssociatedTrackList();
			if(!associatedTracks.empty())
			{
				++nChargedCluster;
				mainCluster = cluster;
			}
		}

		// the case of nChargedCluster == 1 or nChargedCluster >1 ???
		if(nChargedCluster>0)
		{
		   for(ClusterList::const_iterator clusterIter = pClusterList->begin(); clusterIter != pClusterList->end(); ++clusterIter)
		   {
		   	 const Cluster* fragment = *clusterIter;
			 const TrackList& associatedTracks = fragment->GetAssociatedTrackList();
			 if(!associatedTracks.empty()) continue;

		   	 if(fragment != mainCluster) PandoraContentApi::MergeAndDeleteClusters(*this, mainCluster, fragment);
		   }
		}
	  }

	  if(pClusterList->size()==1 || nChargedCluster==0)
	  {
          const MCParticle *const pMCParticle = mapIter->first;
		  MCParticleToTrackMap::iterator iter = mcParticleToTrackMap.find(pMCParticle);

		  if(iter != mcParticleToTrackMap.end())
		  {
			  const Track* track = iter->second;

		      for(ClusterList::const_iterator clusterIter = pClusterList->begin(); clusterIter != pClusterList->end(); ++clusterIter)
		      {
		          const Cluster* fragment = *clusterIter;
				  const TrackList& trackList = fragment->GetAssociatedTrackList();

				  if(trackList.size()>0) continue;

				  // FIXME
			      if(track->HasAssociatedCluster()) 
			      {
			          //std::cout << " !!!!!!!! track has associated to cluster..." << std::endl;
			          const Cluster* cluster = track->GetAssociatedCluster();
			          PandoraContentApi::RemoveTrackClusterAssociation(*this, track, cluster);
			          PandoraContentApi::MergeAndDeleteClusters(*this, cluster, fragment);
					  PandoraContentApi::AddTrackClusterAssociation(*this, track, cluster);
			      }
				  else
				  {
					  PandoraContentApi::AddTrackClusterAssociation(*this, track, fragment);
				  }
		      }
		  }
	  }

      delete pClusterList;
    }

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode PerfectFragmentRemovalAlgorithm::ReadSettings(const TiXmlHandle )
  {
    return STATUS_CODE_SUCCESS;
  }

}
