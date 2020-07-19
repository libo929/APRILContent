/**
 *  @file   LCContent/src/LCTrackClusterAssociation/TrackRecoveryInteractionsAlgorithm.cc
 * 
 *  @brief  Implementation of the track recovery interactions algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/ReclusterHelper.h"
#include "APRILHelpers/SortingHelper.h"

#include "APRILTrackClusterAssociation/TrackRecoveryInteractionsAlgorithm.h"

namespace april_content
{

pandora::StatusCode TrackRecoveryInteractionsAlgorithm::Run()
{
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

	pandora::TrackVector trackVector(pTrackList->begin(), pTrackList->end());
    std::sort(trackVector.begin(), trackVector.end(), pandora::PointerLessThan<pandora::Track>());

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // Loop over all unassociated tracks in the current track list
    for (pandora::TrackVector::const_iterator iterT = trackVector.begin(), iterTEnd = trackVector.end(); iterT != iterTEnd; ++iterT)
    {
        const pandora::Track *const pTrack = *iterT;

        // Use only unassociated tracks that can be used to form a pfo
        if (pTrack->HasAssociatedCluster() || !pTrack->CanFormPfo())
            continue;

        if (!pTrack->GetDaughterList().empty())
            continue;

        // Identify best cluster to be associated with this track, based on energy consistency and proximity
        const pandora::Cluster *pBestCluster(NULL);
        float minEnergyDifference(std::numeric_limits<float>::max());
        float smallestTrackClusterDistance(std::numeric_limits<float>::max());

        for (pandora::ClusterList::const_iterator iterC = pClusterList->begin(), iterCEnd = pClusterList->end(); iterC != iterCEnd; ++iterC)
        {
            const pandora::Cluster *const pCluster = *iterC;

            if (!pCluster->GetAssociatedTrackList().empty() || (0 == pCluster->GetNCaloHits()))
                continue;

            float trackClusterDistance(std::numeric_limits<float>::max());

            if (pandora::STATUS_CODE_SUCCESS != ClusterHelper::GetTrackClusterDistance(&(pTrack->GetTrackStateAtCalorimeter()), pCluster, m_maxSearchLayer, m_parallelDistanceCut,
                m_minTrackClusterCosAngle, trackClusterDistance))
            {
                continue;
            }

            const float energyDifference(std::fabs(pCluster->GetHadronicEnergy() - pTrack->GetEnergyAtDca()));

            if ((trackClusterDistance < smallestTrackClusterDistance) ||
                ((trackClusterDistance == smallestTrackClusterDistance) && (energyDifference < minEnergyDifference)))
            {
                smallestTrackClusterDistance = trackClusterDistance;
                pBestCluster = pCluster;
                minEnergyDifference = energyDifference;
            }
        }

        if ((NULL == pBestCluster) || (smallestTrackClusterDistance > m_maxTrackClusterDistance))
            continue;

        // Should track be associated with "best" cluster?
        const float clusterEnergy(pBestCluster->GetTrackComparisonEnergy(this->GetPandora()));

        if ((smallestTrackClusterDistance > m_trackClusterDistanceCut) && (clusterEnergy > m_clusterEnergyCut))
        {
            const pandora::CartesianVector &trackCalorimeterPosition(pTrack->GetTrackStateAtCalorimeter().GetPosition());
            const pandora::CartesianVector &trackerEndPosition(pTrack->GetTrackStateAtEnd().GetPosition());
            const pandora::CartesianVector innerLayerCentroid(pBestCluster->GetCentroid(pBestCluster->GetInnerPseudoLayer()));

            const pandora::CartesianVector trackerToTrackCalorimeterUnitVector((trackCalorimeterPosition - trackerEndPosition).GetUnitVector());
            const pandora::CartesianVector trackerToClusterUnitVector((innerLayerCentroid - trackerEndPosition).GetUnitVector());

            const float directionCosine(trackerToClusterUnitVector.GetDotProduct(trackerToTrackCalorimeterUnitVector));

            if (directionCosine < m_directionCosineCut)
                continue;
        }

        const float trackEnergy(pTrack->GetEnergyAtDca());
        const float chi(ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), clusterEnergy, trackEnergy));

        if (chi < m_maxTrackAssociationChi)
        {
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, pTrack, pBestCluster));
        }
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode TrackRecoveryInteractionsAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    m_maxTrackClusterDistance = 200.f;
    m_trackClusterDistanceCut = 100.f;
    m_clusterEnergyCut = 0.5f;
    m_directionCosineCut = 0.9f;
    m_maxTrackAssociationChi = 2.f;
    m_maxSearchLayer = 19;
    m_parallelDistanceCut = 100.f;
    m_minTrackClusterCosAngle = 0.f;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackClusterDistance", m_maxTrackClusterDistance));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "TrackClusterDistanceCut", m_trackClusterDistanceCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterEnergyCut", m_clusterEnergyCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut", m_directionCosineCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackAssociationChi", m_maxTrackAssociationChi));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxSearchLayer", m_maxSearchLayer));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ParallelDistanceCut", m_parallelDistanceCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinTrackClusterCosAngle", m_minTrackClusterCosAngle));

    return pandora::STATUS_CODE_SUCCESS;
}

} 
