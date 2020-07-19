/**
 *  @file   LCContent/src/LCTrackClusterAssociation/LoopingTrackAssociationAlgorithm.cc
 * 
 *  @brief  Implementation of the looping track association algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILHelpers/ReclusterHelper.h"
#include "APRILHelpers/SortingHelper.h"

#include "APRILTrackClusterAssociation/LoopingTrackAssociationAlgorithm.h"

namespace april_content
{

pandora::StatusCode LoopingTrackAssociationAlgorithm::Run()
{
    const pandora::TrackList *pTrackList = NULL;
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    pandora::TrackVector trackVector(pTrackList->begin(), pTrackList->end());
    std::sort(trackVector.begin(), trackVector.end(), pandora::PointerLessThan<pandora::Track>());

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));

    // Loop over all unassociated tracks in the current track list
    for (pandora::TrackVector::const_iterator iterT = trackVector.begin(), iterTEnd = trackVector.end(); iterT != iterTEnd; ++iterT)
    {
        const pandora::Track *const pTrack = *iterT;

        // Use only unassociated tracks that can be used to form a pfo
        if (pTrack->HasAssociatedCluster() || !pTrack->CanFormPfo())
            continue;

        if (!pTrack->GetDaughterList().empty())
            continue;

        // Use only tracks that reach endcap, not barrel
        if (!pTrack->IsProjectedToEndCap())
            continue;

        const float trackCalorimeterZPosition(pTrack->GetTrackStateAtCalorimeter().GetPosition().GetZ());

        // Extract information from the track
        const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(), pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);
        const float helixOmega(helix.GetOmega());

        if (std::fabs(helixOmega) < std::numeric_limits<float>::epsilon())
            continue;

        const float helixRadius(1.f / helixOmega);
        const float helixTanLambda(helix.GetTanLambda());
        const float helixPhi0(helix.GetPhi0());

        const float pi_2(0.5f * std::acos(-1.f));
        const float helixXCentre(helixRadius * std::cos(helixPhi0 - pi_2));
        const float helixYCentre(helixRadius * std::sin(helixPhi0 - pi_2));

        const float helixDCosZ(helixTanLambda / std::sqrt(1.f + helixTanLambda * helixTanLambda));
        const float trackEnergy(pTrack->GetEnergyAtDca());

        // Identify best cluster to be associated with this track, using projection of track helix onto endcap
        const pandora::Cluster *pBestCluster(NULL);
        float minEnergyDifference(std::numeric_limits<float>::max());
        float smallestDeltaR(std::numeric_limits<float>::max());

        for (pandora::ClusterList::const_iterator iterC = pClusterList->begin(), iterCEnd = pClusterList->end(); iterC != iterCEnd; ++iterC)
        {
            const pandora::Cluster *const pCluster = *iterC;

            if (!pCluster->GetAssociatedTrackList().empty())
                continue;

            if ((pCluster->GetNCaloHits() < m_minHitsInCluster) || (pCluster->GetOrderedCaloHitList().size() < m_minOccupiedLayersInCluster))
                continue;

            // Demand that cluster starts in first few layers
            const unsigned int innerLayer(pCluster->GetInnerPseudoLayer());

            if (innerLayer > m_maxClusterInnerLayer)
                continue;

            // Ensure that cluster is in same endcap region as track
            const float clusterZPosition(pCluster->GetCentroid(innerLayer).GetZ());

            if (std::fabs(trackCalorimeterZPosition) - std::fabs(clusterZPosition) > m_maxTrackClusterDeltaZ)
                continue;

            if (clusterZPosition * trackCalorimeterZPosition < 0.f)
                continue;

            // Check consistency of track momentum and cluster energy
            const float chi(ReclusterHelper::GetTrackClusterCompatibility(this->GetPandora(), pCluster->GetTrackComparisonEnergy(this->GetPandora()), trackEnergy));

            if (std::fabs(chi) > m_maxAbsoluteTrackClusterChi)
                continue;

            // Calculate distance of cluster from centre of helix for i) cluster inner layer and ii) first m_nClusterDeltaRLayers layers
            const pandora::CartesianVector innerCentroid(pCluster->GetCentroid(innerLayer));

            const float innerLayerDeltaX(innerCentroid.GetX() - helixXCentre);
            const float innerLayerDeltaY(innerCentroid.GetY() - helixYCentre);
            const float innerLayerDeltaR(std::sqrt((innerLayerDeltaX * innerLayerDeltaX) + (innerLayerDeltaY * innerLayerDeltaY)) - std::fabs(helixRadius));
            const float meanDeltaR(this->GetMeanDeltaR(pCluster, helixXCentre, helixYCentre, helixRadius));

            // Check that cluster is sufficiently close to helix path
            const bool isInRangeInner((innerLayerDeltaR < m_maxDeltaR) && (innerLayerDeltaR > m_minDeltaR));
            const bool isInRangeMean((meanDeltaR < m_maxDeltaR) && (meanDeltaR > m_minDeltaR));

            if (!isInRangeInner && !isInRangeMean)
                continue;

            const float deltaR(std::min(std::fabs(innerLayerDeltaR), std::fabs(meanDeltaR))); // ATTN: Changed order of min and fabs here

            // Calculate projected helix direction at endcap
			pandora::CartesianVector helixDirection(0.f, 0.f, 0.f);

            if (0.f != innerLayerDeltaY)
            {
                float helixDCosX((1.f - helixDCosZ * helixDCosZ) / (1.f + ((innerLayerDeltaX * innerLayerDeltaX) / (innerLayerDeltaY * innerLayerDeltaY))));
                helixDCosX = std::sqrt(std::max(helixDCosX, 0.f));

                if (innerLayerDeltaY * helixRadius < 0)
                    helixDCosX *= -1.f;

                helixDirection.SetValues(helixDCosX, -(innerLayerDeltaX / innerLayerDeltaY) * helixDCosX, helixDCosZ);
            }
            else
            {
                float helixDCosY(1.f - helixDCosZ * helixDCosZ);
                helixDCosY = std::sqrt(std::max(helixDCosY, 0.f));

                if (innerLayerDeltaX * helixRadius > 0)
                    helixDCosY *= -1.f;

                helixDirection.SetValues(0.f, helixDCosY, helixDCosZ);
            }
 
            // Calculate direction of first n layers of cluster
			pandora::ClusterFitResult clusterFitResult;
            if (pandora::STATUS_CODE_SUCCESS != pandora::ClusterFitHelper::FitStart(pCluster, m_nClusterFitLayers, clusterFitResult))
                continue;

            // Compare cluster direction with the projected helix direction
            const float directionCosine(helixDirection.GetDotProduct(clusterFitResult.GetDirection()));

            if ((directionCosine < m_directionCosineCut) && (pCluster->GetMipFraction() < m_clusterMipFractionCut))
                continue;

            // Use position and direction results to identify track/cluster match
            bool isPossibleMatch(false);

            if (directionCosine > m_directionCosineCut1)
            {
                isPossibleMatch = true;
            }
            else if ((directionCosine > m_directionCosineCut2) && (deltaR < m_deltaRCut2))
            {
                isPossibleMatch = true;
            }
            else if ((directionCosine > m_directionCosineCut3) && (deltaR < m_deltaRCut3))
            {
                isPossibleMatch = true;
            }
            else if ((directionCosine > m_directionCosineCut4) && (deltaR < m_deltaRCut4))
            {
                isPossibleMatch = true;
            }

            if (isPossibleMatch)
            {
                const float energyDifference(std::fabs(pCluster->GetHadronicEnergy() - pTrack->GetEnergyAtDca()));

                if ((deltaR < smallestDeltaR) || ((deltaR == smallestDeltaR) && (energyDifference < minEnergyDifference)))
                {
                    smallestDeltaR = deltaR;
                    pBestCluster = pCluster;
                    minEnergyDifference = energyDifference;
                }
            }
        }

        if (NULL != pBestCluster)
        {
			PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddTrackClusterAssociation(*this, pTrack, pBestCluster));
        }
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float LoopingTrackAssociationAlgorithm::GetMeanDeltaR(const pandora::Cluster *const pCluster, const float helixXCentre, const float helixYCentre,
    const float helixRadius) const
{
    float deltaRSum(0.f);
    unsigned int nContributions(0);

    const unsigned int endLayer(pCluster->GetInnerPseudoLayer() + m_nClusterDeltaRLayers);
    const pandora::OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());

    for (pandora::OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), iterEnd = orderedCaloHitList.end(); iter != iterEnd; ++iter)
    {
        if (iter->first > endLayer)
            break;

        for (pandora::CaloHitList::const_iterator hitIter = iter->second->begin(), hitIterEnd = iter->second->end(); hitIter != hitIterEnd; ++hitIter)
        {
            const pandora::CartesianVector &hitPosition((*hitIter)->GetPositionVector());
            const float hitDeltaX(hitPosition.GetX() - helixXCentre);
            const float hitDeltaY(hitPosition.GetY() - helixYCentre);

            deltaRSum += std::sqrt((hitDeltaX * hitDeltaX) + (hitDeltaY * hitDeltaY));
            nContributions++;
        }
    }

    if (0 == nContributions)
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    const float meanDeltaR((deltaRSum / static_cast<float>(nContributions)) - std::fabs(helixRadius));

    return meanDeltaR;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LoopingTrackAssociationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    m_maxTrackClusterDeltaZ = 50.f;
    m_minHitsInCluster = 4;
    m_minOccupiedLayersInCluster = 4;
    m_maxClusterInnerLayer = 9;
    m_maxAbsoluteTrackClusterChi = 2.f;
    m_maxDeltaR = 50.f;
    m_minDeltaR = -100.f;
    m_nClusterFitLayers = 10;
    m_nClusterDeltaRLayers = 9;
    m_directionCosineCut = 0.975f;
    m_clusterMipFractionCut = 0.5f;
    m_directionCosineCut1 = 0.925f;
    m_directionCosineCut2 = 0.85f;
    m_deltaRCut2 = 50.f;
    m_directionCosineCut3 = 0.75f;
    m_deltaRCut3 = 25.f;
    m_directionCosineCut4 = 0.f;
    m_deltaRCut4 = 10.f;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTrackClusterDeltaZ", m_maxTrackClusterDeltaZ));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinHitsInCluster", m_minHitsInCluster));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinOccupiedLayersInCluster", m_minOccupiedLayersInCluster));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxClusterInnerLayer", m_maxClusterInnerLayer));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxAbsoluteTrackClusterChi", m_maxAbsoluteTrackClusterChi));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxDeltaR", m_maxDeltaR));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinDeltaR", m_minDeltaR));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NClusterFitLayers", m_nClusterFitLayers));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NClusterDeltaRLayers", m_nClusterDeltaRLayers));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut", m_directionCosineCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterMipFractionCut", m_clusterMipFractionCut));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut1", m_directionCosineCut1));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut2", m_directionCosineCut2));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DeltaRCut2", m_deltaRCut2));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut3", m_directionCosineCut3));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DeltaRCut3", m_deltaRCut3));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DirectionCosineCut4", m_directionCosineCut4));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "DeltaRCut4", m_deltaRCut4));

    return pandora::STATUS_CODE_SUCCESS;
}

} 
