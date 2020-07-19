/**
 *  @file   LCContent/src/LCTrackClusterAssociation/TrackClusterAssociationAlgorithm.cc
 * 
 *  @brief  Implementation of the track-cluster association algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/SortingHelper.h"

#include "APRILTrackClusterAssociation/TrackClusterAssociationAlgorithm.h"

#include "APRILUtility/KDTreeLinkerAlgoT.h"

#include "APRILObjects/APRILTrack.h"

using namespace pandora;

// setup templates for tracking track:pseudolayer pair
namespace std
{
    template<>
    struct hash<std::pair<const pandora::Track*,unsigned> >
    {
        std::size_t operator()(std::pair<const pandora::Track*,unsigned> const& tps) const 
        {
            std::size_t h1 = std::hash<const pandora::Track*>()(tps.first);
            std::size_t h2 = std::hash<unsigned>()(tps.second);
            return h1 ^ (h2 << 1);
        }
    };
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

namespace april_content
{

pandora::StatusCode TrackClusterAssociationAlgorithm::Run()
{
    const pandora::TrackList *pTrackList = nullptr;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

	pandora::TrackVector trackVector(pTrackList->begin(), pTrackList->end());
    std::sort(trackVector.begin(), trackVector.end(), pandora::PointerLessThan<pandora::Track>());

    const pandora::ClusterList *pClusterList = nullptr;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // Clear any existing track - cluster associations
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveCurrentTrackClusterAssociations(*this));

    HitKDTree hits_kdtree;
    std::vector<HitKDNode> hit_nodes;
	pandora::CaloHitList hit_list, clusterHits;
    TracksToHitsInPseudoLayerMap tracks_to_hits;
    HitsToClustersMap hits_to_clusters;

    // build the kd-tree of hits from the input clusters and save the map of hits to clusters
    for (const pandora::Cluster *const pCluster : *pClusterList)
    {
        pCluster->GetOrderedCaloHitList().FillCaloHitList(clusterHits);

        for (const pandora::CaloHit *const pCaloHit : clusterHits)
        {
            hit_list.push_back(pCaloHit);
            hits_to_clusters.emplace(pCaloHit, pCluster);
        }
        clusterHits.clear();
    }

    KDTreeTesseract hitsBoundingRegion = fill_and_bound_4d_kd_tree(this, hit_list, hit_nodes, true);
    hit_list.clear();
    hits_kdtree.build(hit_nodes,hitsBoundingRegion);
    hit_nodes.clear();

    // move result caches out of the loop
	pandora::ClusterSet nearby_clusters;
    std::vector<HitKDNode> found_hits;

    // Look to make new associations
    for (const pandora::Track *const pTrack : trackVector)
    {
        // Use only tracks that can be used to form a pfo
        if (!pTrack->CanFormPfo())
            continue;

        if (!pTrack->GetDaughterList().empty())
            continue;

        const pandora::Cluster *pBestCluster = nullptr;
        const pandora::Cluster *pBestLowEnergyCluster = nullptr;

        float minDistance(m_maxTrackClusterDistance);
        float minLowEnergyDistance(m_maxTrackClusterDistance);

        float minEnergyDifference(std::numeric_limits<float>::max());
        float minLowEnergyDifference(std::numeric_limits<float>::max());

        APRILTrackStates trackStates;
        const APRILTrack* aprilTrack = dynamic_cast<const APRILTrack*>(pTrack);
        if( aprilTrack )
        {
            trackStates = aprilTrack->GetTrackStates();
        }
        else
        {
            const pandora::TrackState &trackState(pTrack->GetTrackStateAtCalorimeter());
            trackStates.push_back( trackState );
        }

        for (auto const& trackState : trackStates)
        {

            const pandora::CartesianVector &trackPosition(trackState.GetPosition());


            // short circuit this loop with a kd-tree search beforehand
            // iterating over a std::map is expensive, avoid where possible
            for (unsigned iPseudoLayer = 0; iPseudoLayer <= m_maxSearchLayer; ++iPseudoLayer)
            {
                // save the hash key since we may use it a few times
                auto hash_key = std::make_pair(pTrack, iPseudoLayer);
                // see if we have a cached search, otherwise do the search and cache
                auto cached_result = tracks_to_hits.equal_range(hash_key);
                if (cached_result.first != tracks_to_hits.end())
                {
                    for (auto iter = cached_result.first; iter != cached_result.second; ++iter )
                    {
                        // build a list of nearby clusters
                        nearby_clusters.insert(hits_to_clusters.find(iter->second)->second);
                    }
                }
                else
                {
                    // do search and cache result
                    KDTreeTesseract searchRegionHits = build_4d_kd_search_region(trackPosition, m_parallelDistanceCut, m_parallelDistanceCut, m_parallelDistanceCut, iPseudoLayer);
                    hits_kdtree.search(searchRegionHits, found_hits);

                    for (const auto &hit : found_hits)
                    {
                        auto assc_cluster = hits_to_clusters.find(hit.data);
                        if (assc_cluster != hits_to_clusters.end())
                        {
                            // cache all hits that are nearby the track
                            tracks_to_hits.emplace(hash_key,hit.data);
                            // add to the list of nearby clusters
                            nearby_clusters.insert(assc_cluster->second);
                        }
                    }
                    found_hits.clear();
                }
            }

			pandora::ClusterList nearbyClusterList(nearby_clusters.begin(), nearby_clusters.end());
            nearbyClusterList.sort(SortingHelper::SortClustersByNHits);
            nearby_clusters.clear();

            // Identify the closest cluster and also the closest cluster below a specified hadronic energy threshold
            for (const pandora::Cluster *const pCluster : nearbyClusterList)
            {
                if (0 == pCluster->GetNCaloHits())
                    continue;

                float trackClusterDistance(std::numeric_limits<float>::max());
                if (pandora::STATUS_CODE_SUCCESS != april_content::ClusterHelper::GetTrackClusterDistance(&trackState, pCluster, m_maxSearchLayer, m_parallelDistanceCut,
                    m_minTrackClusterCosAngle, trackClusterDistance))
                {
                    continue;
                }

                const float energyDifference(std::fabs(pCluster->GetHadronicEnergy() - pTrack->GetEnergyAtDca()));

                if (pCluster->GetHadronicEnergy() > m_lowEnergyCut)
                {
                    if ((trackClusterDistance < minDistance) || ((trackClusterDistance == minDistance) && (energyDifference < minEnergyDifference)))
                    {
                        minDistance = trackClusterDistance;
                        pBestCluster = pCluster;
                        minEnergyDifference = energyDifference;
                    }
                }
                else
                {
                    if ((trackClusterDistance < minLowEnergyDistance) || ((trackClusterDistance == minLowEnergyDistance) && (energyDifference < minLowEnergyDifference)))
                    {
                        minLowEnergyDistance = trackClusterDistance;
                        pBestLowEnergyCluster = pCluster;
                        minLowEnergyDifference = energyDifference;
                    }
                }
            } // for all clusters
        } //for all trackStates

        // Apply a final track-cluster association distance cut
        const pandora::Cluster *pMatchedCluster = nullptr;

        if (nullptr != pBestCluster)
        {
            pMatchedCluster = pBestCluster;
        }
        else if (nullptr != pBestLowEnergyCluster)
        {
            pMatchedCluster = pBestLowEnergyCluster;
        }

        // Now make the association
        if (nullptr != pMatchedCluster)
        {
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, pTrack, pMatchedCluster));
        }
    } //for all tracks
    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode TrackClusterAssociationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    m_lowEnergyCut = 0.2f;
    m_maxTrackClusterDistance = 10.f;
    m_maxSearchLayer = 9;
    m_parallelDistanceCut = 100.f;
    m_minTrackClusterCosAngle = 0.f;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "LowEnergyCut", m_lowEnergyCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackClusterDistance", m_maxTrackClusterDistance));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxSearchLayer", m_maxSearchLayer));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ParallelDistanceCut", m_parallelDistanceCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinTrackClusterCosAngle", m_minTrackClusterCosAngle));

    return pandora::STATUS_CODE_SUCCESS;
}

}
