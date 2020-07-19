/**
 *  @file   CheatingClusterMergingAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster merging algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/CheatingClusterMergingAlgorithm.h"

using namespace pandora;

namespace april_content
{

StatusCode CheatingClusterMergingAlgorithm::Run()
{
	// if clusers have the same PFO target merge them
	// consider cluster has assocated track
	
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "cluster size: " << pClusterList->size() << std::endl;

	std::map<const MCParticle*, ClusterList> mcpClusters;

    for (ClusterList::const_iterator itCluster = pClusterList->begin(), itClusterEnd = pClusterList->end(); itCluster != itClusterEnd; ++itCluster )
    {
        try
        {
            const Cluster *const pCluster = *itCluster;
            const MCParticle *const pMainMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
			const MCParticle* const pfoTarget = pMainMCParticle->GetPfoTarget();

			if( mcpClusters.find( pfoTarget ) == mcpClusters.end() )
			{
				ClusterList clusters;
				clusters.push_back( pCluster );
				mcpClusters[pfoTarget] = clusters;
			}
			else
			{
				mcpClusters[pfoTarget].push_back( pCluster );
			}
        }
        catch (StatusCodeException &)
        {
        }
    }

	///////////////////////////////////////////////////////////////////////////////////////////////
	for( auto mapIter : mcpClusters )
	{
		//auto mcp = mapIter.first;
		auto clusters = mapIter.second;

		if( clusters.size() > 1 )
		{
		    // remove the associated tracks of all clusters
			pandora::TrackList removedTracks;
		    for(auto clu : clusters)
		    {
		    	auto associatedTracks = clu->GetAssociatedTrackList();

		    	for( auto associatedTrack : associatedTracks)
		        {
                    PandoraContentApi::RemoveTrackClusterAssociation(*this, associatedTrack, clu);
					removedTracks.push_back( associatedTrack );
		        }
		    }

		    // merge clusters
			// simply chose the first as the main cluster
			auto mainCluster = *clusters.begin();

			for( auto cluIter = ++clusters.begin(); cluIter != clusters.end(); ++cluIter)
			{
				auto clusterToDelete = *cluIter;
				PandoraContentApi::MergeAndDeleteClusters(*this, mainCluster, clusterToDelete);
			}

			// add tracks to the new merged cluster
			for(auto track : removedTracks)
			{
				PandoraContentApi::AddTrackClusterAssociation(*this, track, mainCluster);
			}
		}
	}

	std::cout << "cluster size after merging: " << pClusterList->size() << std::endl;

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode CheatingClusterMergingAlgorithm::ReadSettings(const TiXmlHandle /*xmlHandle*/)
{
    return STATUS_CODE_SUCCESS;
}

} // namespace april_content
