/**
 *  @file   APRILContent/src/APRILCheating/CheatingTrackToClusterMatching.cc
 * 
 *  @brief  Implementation of the cheating track to cluster matching algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/CheatingTrackToClusterMatching.h"
#include "APRILHelpers/HistogramHelper.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILUtility/EventPreparationAlgorithm.h"
#include "APRILApi/APRILContentApi.h"

#define __DEBUG__ 1

namespace april_content
{

  bool CheatingTrackToClusterMatching::IsParent(const pandora::MCParticle* parent, const pandora::MCParticle* daughter)
  {
	  if(daughter == parent) return true;

	  if(daughter->IsPfoTarget()) return false;

	  auto parentList = daughter->GetParentList();

	  bool foundParentMCP = false;

	  if( std::find(parentList.begin(), parentList.end(), parent) != parentList.end() )
	  {
		  foundParentMCP = true;
	  }
	  else
	  {
		  for(auto mcp : parentList)
		  {
			  if (IsParent(parent, mcp) == true)
			  {
				  foundParentMCP = true;
				  break;
			  }
		  }

	  }
	
	  return foundParentMCP;
  }

  pandora::StatusCode CheatingTrackToClusterMatching::Run()
  {
	//std::cout << "CheatingTrackToCluster......" << std::endl;
    // Read current lists
    const pandora::TrackList *pCurrentTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCurrentTrackList));

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // Clear any existing track - cluster associations
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveCurrentTrackClusterAssociations(*this));

#if 0
	// remove all the wrong track-cluster associations
	for(auto& track : *pCurrentTrackList)
	{
		if( !(track->HasAssociatedCluster()) ) continue;

		auto cluster = track->GetAssociatedCluster();

		const pandora::MCParticle *pTrackMCParticle = nullptr;
		const pandora::MCParticle *pClusterMCParticle = nullptr;

		try
		{
			pTrackMCParticle = pandora::MCParticleHelper::GetMainMCParticle(track); 
			pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(cluster); 
		}
                catch (pandora::StatusCodeException &)
                {	
                }

		if(pTrackMCParticle != pClusterMCParticle)
		{
			PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveTrackClusterAssociation(*this, track, cluster));
		}
	}
#endif

    // Construct a map from mc particle to tracks
    typedef std::map<const pandora::MCParticle*, pandora::TrackList> TracksPerMCParticle;
    TracksPerMCParticle tracksPerMCParticle;
    TracksPerMCParticle tracksPfoTarget;

    for (pandora::TrackList::const_iterator iter = pCurrentTrackList->begin(), iterEnd = pCurrentTrackList->end(); iter != iterEnd; ++iter)
    {
      try
      {
        const pandora::Track *const pTrack = *iter;
        //const MCParticle *const pMCParticle(pTrack->GetMainMCParticle());
		const pandora::MCParticle *pMCParticle = nullptr;
		
		try
		{
			pMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pTrack); 
		}
        catch (pandora::StatusCodeException &)
        {	
	        std::cout << "============= mcp problem .... " << __LINE__ << " track: " << pTrack << " p: " 
				<< pTrack->GetMomentumAtDca().GetMagnitude() << std::endl;
			continue;
        }

		const pandora::MCParticle *const pPfoTarget(pMCParticle->GetPfoTarget());

        TracksPerMCParticle::iterator itTracksPerMCParticle(tracksPerMCParticle.find(pMCParticle));

        if (tracksPerMCParticle.end() == itTracksPerMCParticle)
        {
		  pandora::TrackList trackList;
          trackList.push_back(pTrack);

		  //FIXME:: check existence
		  if(!tracksPerMCParticle.insert(TracksPerMCParticle::value_type(pMCParticle, pandora::TrackList(1, pTrack))).second)
		  {
			  std::cout << "tracksPerMCParticle.insert FAILURE" << __LINE__ << std::endl;
              throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);
		  }
          //if (!tracksPerMCParticle.push_back(TracksPerMCParticle::value_type(pMCParticle, trackList)).second)
        }
        else
        {
          itTracksPerMCParticle->second.push_back(pTrack);
        }

		//////////////////
        TracksPerMCParticle::iterator itTrackPfoParget(tracksPfoTarget.find(pPfoTarget));

        if (tracksPfoTarget.end() == itTrackPfoParget)
        {
		  pandora::TrackList trackList;
          trackList.push_back(pTrack);

		  if(!tracksPfoTarget.insert(TracksPerMCParticle::value_type(pPfoTarget, pandora::TrackList(1, pTrack))).second)
		  {
			  std::cout << "tracksPerMCParticle.insert FAILURE" << __LINE__ << std::endl;
			  throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);
		  }
          //if (!tracksPerMCParticle.push_back(TracksPerMCParticle::value_type(pMCParticle, trackList)).second)
        }
        else
        {
          itTrackPfoParget->second.push_back(pTrack);
        }

      }
      catch (pandora::StatusCodeException &)
      {	
		  std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
      }
    }

	////////////////////////////////////////// matching track-cluster by direct MCP
    for (pandora::ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
      try
      {
        const pandora::Cluster *const pCluster = *iter;
        const pandora::MCParticle *const pMCParticle(pandora::MCParticleHelper::GetMainMCParticle(pCluster));
        //const MCParticle *const pClusterPfoTarget(pMCParticle->GetPfoTarget());

		auto foundMCPTracks = tracksPerMCParticle.find(pMCParticle);

		if(foundMCPTracks != tracksPerMCParticle.end() && PandoraContentApi::IsAvailable(*this, pCluster))
		{
			const pandora::TrackList& tracks = foundMCPTracks->second;

			for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
			{
				auto track = *trackIter;

				if(track->HasAssociatedCluster())
				{
					//std::cout << __LINE__ << ": the track has AssociatedCluster !!! Track p: " << track->GetMomentumAtDca().GetMagnitude()
					//	      << ", cluster energy to match: " << pCluster->GetHadronicEnergy() <<  ", AssociatedCluster energy: " <<
					//track->GetAssociatedCluster()->GetHadronicEnergy() << std::endl;

					// if this is true, it means cluster should be merged.

					continue;
				}

				if( !PandoraContentApi::IsAvailable(*this, track) || !PandoraContentApi::IsAvailable(*this, pCluster) ) 
				{
					//std::cout << " AddTrackClusterAssociation issue: track or cluster is not available " << 
					//	" track p: " << track->GetMomentumAtDca().GetMagnitude() << ", cluster Ehad: " << 
					//	pCluster->GetHadronicEnergy() << std::endl;

					continue;
				}

				//std::cout << "match track with cluster : " << track << " --- " << pCluster << std::endl;
				PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, track, pCluster));
			}
		}
	  }
      catch (pandora::StatusCodeException &)
	  {
		  std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
	  }
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// If find no matching track-cluster by direct MCP, use pfo target: if the track is parent of
	// the cluster, it means they can be matched
	// 1) if track has no associated cluster, just macth the track and cluster
	// 2) otherwise record the cluster to merge
	// 3) finally merge all clusters in the record
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
    for (pandora::ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
      try
      {
		//std::cout << "haha" << std::endl;
        const pandora::Cluster *const pCluster = *iter;
	    //std::cout << pCluster << std::endl;
        const pandora::MCParticle *const pMCParticle(pandora::MCParticleHelper::GetMainMCParticle(pCluster));
        const pandora::MCParticle *const pClusterPfoTarget(pMCParticle->GetPfoTarget());

		auto foundMCPTracksByPfo = tracksPfoTarget.find( pClusterPfoTarget );

		if( foundMCPTracksByPfo != tracksPfoTarget.end() && PandoraContentApi::IsAvailable(*this, pCluster) )
		{
			const pandora::TrackList& tracks = foundMCPTracksByPfo->second;
			for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
			{
				auto track = *trackIter;
				if( !PandoraContentApi::IsAvailable(*this, track) || !PandoraContentApi::IsAvailable(*this, pCluster) ) continue;

				// if track is parent of cluster, add the cluster
				bool isParent = false;

				try
				{
                    const pandora::MCParticle *const trackMCP(pandora::MCParticleHelper::GetMainMCParticle(track));
                    const pandora::MCParticle *const cluMCP(pandora::MCParticleHelper::GetMainMCParticle(pCluster));
				    isParent = IsParent(trackMCP, cluMCP); // recursive function
				}
                catch (pandora::StatusCodeException &)
				{
				    std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
					continue;
				}

				if(isParent)
				{
				    // if track has associated cluster, merge the clusters
				    if(! track->HasAssociatedCluster() ) 
					{
				        // add association for the track and cluster
						std::cout << "match track with cluster : " << track << " --- " << pCluster << std::endl;
						std::cout << "track energy: " << track->GetMomentumAtDca().GetMagnitude() << ", cluster energy: "
							      << pCluster->GetHadronicEnergy() << std::endl;
						// make a map
				        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, 
								PandoraContentApi::AddTrackClusterAssociation(*this, track, pCluster));
					}

				    // if the cluster is associated to track, stop the loop
				    break;
				}
			} // for each track
		}
	  }
      catch (pandora::StatusCodeException &)
	  {
		  std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
	  }
	}

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// merge the cluster fragments from the same track
	if(!m_shouldMergeTrackClusters) return pandora::STATUS_CODE_SUCCESS;

	std::map<const pandora::Cluster*, pandora::ClusterList> clustersToMerge;
	std::cout << "  --- Cluster number before merging: " << pClusterList->size() << std::endl;
	
    for (pandora::ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
      try
      {
        const pandora::Cluster *const pCluster = *iter;
        const pandora::MCParticle *const pMCParticle(pandora::MCParticleHelper::GetMainMCParticle(pCluster));
        const pandora::MCParticle *const pClusterPfoTarget(pMCParticle->GetPfoTarget());

		auto foundMCPTracksByPfo = tracksPfoTarget.find( pClusterPfoTarget );

		if( foundMCPTracksByPfo != tracksPfoTarget.end() && PandoraContentApi::IsAvailable(*this, pCluster) )
		{
			const pandora::TrackList& tracks = foundMCPTracksByPfo->second;
			for(auto trackIter = tracks.begin(); trackIter != tracks.end(); ++trackIter)
			{
				auto track = *trackIter;
				if( !PandoraContentApi::IsAvailable(*this, track) || !PandoraContentApi::IsAvailable(*this, pCluster) ) continue;

				// if track is parent of cluster, add the cluster
				bool isParent = false;

				try
				{
                    const pandora::MCParticle *const trackMCP(pandora::MCParticleHelper::GetMainMCParticle(track));
                    const pandora::MCParticle *const cluMCP(pandora::MCParticleHelper::GetMainMCParticle(pCluster));
				    isParent = IsParent(trackMCP, cluMCP); // recursive function
				}
                catch (pandora::StatusCodeException &)
				{
					std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
					continue;
				}

				if(isParent)
				{
				    // if track has associated cluster, merge the clusters
				    if(track->HasAssociatedCluster() ) 
				    {
				        //std::cout << __LINE__ << ": the track has AssociatedCluster !!! Track energy: " << track->GetEnergyAtDca() 
				        //	      << ", cluster energy to match: " << pCluster->GetHadronicEnergy() <<  ", AssociatedCluster energy: " << 
				        //track->GetAssociatedCluster()->GetHadronicEnergy() << std::endl;
						if(track->GetAssociatedCluster() != pCluster)
						{
							//std::cout << " merge clusters: " << track->GetAssociatedCluster() << " --- " << pCluster << std::endl;
							//std::cout << " clusters energy: " << track->GetAssociatedCluster()->GetHadronicEnergy() << " --- " 
							//	      << pCluster->GetHadronicEnergy() << std::endl;

							const pandora::Cluster* mainCluster = track->GetAssociatedCluster();
							auto clusterToMerge = pCluster;

							if(clustersToMerge.find(mainCluster) == clustersToMerge.end())
							{
								pandora::ClusterList clusterList;
								clusterList.push_back( clusterToMerge );
								clustersToMerge[mainCluster] = clusterList;
							}
							else
							{
								clustersToMerge[mainCluster].push_back(clusterToMerge);
							}

						    // make a map
		                    //PandoraContentApi::MergeAndDeleteClusters(*this, track->GetAssociatedCluster(), pCluster);
						}
				    }

				    // if the cluster is associated to track, stop the loop
				    break;
				}
			} // for each track
		}
	  }
      catch (pandora::StatusCodeException &)
	  {
			std::cout << "============= mcp problem ...." << __LINE__ << std::endl;
	  }
	}

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	for(auto cluIter : clustersToMerge)
	{
		auto mainCluster = cluIter.first;
		auto clusters = cluIter.second;

		if(clusters.empty()) continue;

		for( auto iter : clusters)
		{
			auto clu = iter;
		    bool isMainPhoton = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(mainCluster);
		    bool isPhoton = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(clu);

#if __DEBUG__
			float oldChi = -1.e6;
			float newChi = -1.e6;
			float trackEnergy = 0.;

			ClusterHelper::GetChiClusterMerging(this->GetPandora(), mainCluster, clu, trackEnergy, oldChi, newChi);

			float meanDensityMain = 1.e6;
			float meanDensity = 1.e6;;
			ClusterHelper::GetMeanDensity(mainCluster, meanDensityMain);
			ClusterHelper::GetMeanDensity(clu, meanDensity);

			std::cout << "cluster: " << mainCluster << ", E: " << mainCluster->GetHadronicEnergy() << ", density: " << meanDensityMain
				      << " - merge cluster: " << clu << ", E: " << clu->GetHadronicEnergy() << ", density: " << meanDensity
					  << ", isPhotonID: " << isPhoton
					  << ", oldChi: " << oldChi << ", newChi: " << newChi << std::endl;
		
#endif

	        std::vector<float> vars;
	        vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	        vars.push_back( mainCluster->GetHadronicEnergy() );
	        vars.push_back( clu->GetHadronicEnergy() );
	        vars.push_back( float(isMainPhoton) );
	        vars.push_back( float(isPhoton) );
		        
			HistogramManager::CreateFill("CheatingTrackToClusterMatching", "evtNum:clusterEnergy:mergeEnergy:isMainPhoton:isPhoton", vars);

		    APRILContentApi::MergeAndDeleteClusters(*this, mainCluster, clu);
		}
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CheatingTrackToClusterMatching::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    m_shouldMergeTrackClusters = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldMergeTrackClusters", m_shouldMergeTrackClusters));

    return pandora::STATUS_CODE_SUCCESS;
  }

} // namespace april_content
