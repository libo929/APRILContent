/**
 *  @file   LCContent/src/LCHelpers/FragmentRemovalHelper.cc
 * 
 *  @brief  Implementation of the fragment removal helper class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILHelpers/FragmentRemovalHelper.h"

using namespace pandora;

namespace april_content
{

float FragmentRemovalHelper::GetFractionOfCloseHits(const Cluster *const pClusterI, const Cluster *const pClusterJ, const float distanceThreshold)
{
    const unsigned int nCaloHitsI(pClusterI->GetNCaloHits());
    const float distanceThresholdSquared(distanceThreshold * distanceThreshold);

    if (0 == nCaloHitsI)
        return 0.;

    const OrderedCaloHitList &orderedCaloHitListI(pClusterI->GetOrderedCaloHitList());
    const OrderedCaloHitList &orderedCaloHitListJ(pClusterJ->GetOrderedCaloHitList());

    unsigned int nCloseHits(0);

    // Loop over hits in cluster I
    for (OrderedCaloHitList::const_iterator iterI = orderedCaloHitListI.begin(), iterIEnd = orderedCaloHitListI.end(); iterI != iterIEnd; ++iterI)
    {
        for (CaloHitList::const_iterator hitIterI = iterI->second->begin(), hitIterIEnd = iterI->second->end(); hitIterI != hitIterIEnd; ++hitIterI)
        {
            bool isCloseHit(false);
            const CartesianVector &positionVectorI((*hitIterI)->GetPositionVector());

            // For each hit in cluster I, check whether there is a sufficiently close hit in cluster J
            for (OrderedCaloHitList::const_iterator iterJ = orderedCaloHitListJ.begin(), iterJEnd = orderedCaloHitListJ.end(); iterJ != iterJEnd; ++iterJ)
            {
                for (CaloHitList::const_iterator hitIterJ = iterJ->second->begin(), hitIterJEnd = iterJ->second->end(); hitIterJ != hitIterJEnd; ++hitIterJ)
                {
                    const float distanceSquared((positionVectorI - (*hitIterJ)->GetPositionVector()).GetMagnitudeSquared());

                    if (distanceSquared < distanceThresholdSquared)
                    {
                        isCloseHit = true;
                        nCloseHits++;
                        break;
                    }
                }

                if (isCloseHit)
                    break;
            }
        }
    }

    return static_cast<float>(nCloseHits) / static_cast<float>(nCaloHitsI);
}

//------------------------------------------------------------------------------------------------------------------------------------------

float FragmentRemovalHelper::GetFractionOfHitsInCone(const Pandora &pandora, const Cluster *const pClusterI, const Cluster *const pClusterJ,
    const float coneCosineHalfAngle)
{
    CartesianVector coneApex(0.f, 0.f, 0.f), coneDirection(0.f, 0.f, 0.f);
    const TrackList &associatedTrackList(pClusterJ->GetAssociatedTrackList());

    if (associatedTrackList.empty())
    {
        // As there is no associated track, use hits in the shower start layer to specify cone direction
        const OrderedCaloHitList &orderedCaloHitList(pClusterJ->GetOrderedCaloHitList());
        const unsigned int showerStartLayer(pClusterJ->GetShowerStartLayer(pandora));

        OrderedCaloHitList::const_iterator iter = orderedCaloHitList.find(showerStartLayer);

        if (orderedCaloHitList.end() == iter)
            throw StatusCodeException(STATUS_CODE_FAILURE);

        CartesianVector hitDirection(0.f, 0.f, 0.f);

        for (CaloHitList::const_iterator hitIter = iter->second->begin(), hitIterEnd = iter->second->end(); hitIter != hitIterEnd; ++hitIter)
            hitDirection += (*hitIter)->GetExpectedDirection();

        coneDirection = hitDirection.GetUnitVector();
        coneApex = pClusterJ->GetCentroid(showerStartLayer);
    }
    else
    {
        const Track *const pTrack(*(associatedTrackList.begin()));

        coneApex = pTrack->GetTrackStateAtCalorimeter().GetPosition();
        coneDirection = pTrack->GetTrackStateAtCalorimeter().GetMomentum().GetUnitVector();
    }

    return FragmentRemovalHelper::GetFractionOfHitsInCone(pClusterI, coneApex, coneDirection, coneCosineHalfAngle);
}

//------------------------------------------------------------------------------------------------------------------------------------------

float FragmentRemovalHelper::GetFractionOfHitsInCone(const Cluster *const pCluster, const Track *const pTrack, const float coneCosineHalfAngle)
{
    const CartesianVector coneApex(pTrack->GetTrackStateAtCalorimeter().GetPosition());
    const CartesianVector coneDirection(pTrack->GetTrackStateAtCalorimeter().GetMomentum().GetUnitVector());

    return FragmentRemovalHelper::GetFractionOfHitsInCone(pCluster, coneApex, coneDirection, coneCosineHalfAngle);
}

//------------------------------------------------------------------------------------------------------------------------------------------

float FragmentRemovalHelper::GetFractionOfHitsInCone(const Cluster *const pCluster, const CartesianVector &coneApex,
    const CartesianVector &coneDirection, const float coneCosineHalfAngle)
{
    const unsigned int nCaloHits(pCluster->GetNCaloHits());

    if (0 == nCaloHits)
        return 0.;

    unsigned int nHitsInCone(0);
    const OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());

    for (OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), iterEnd = orderedCaloHitList.end(); iter != iterEnd; ++iter)
    {
        const CaloHitList *const pCaloHitList(iter->second);

        for (CaloHitList::const_iterator hitIter = pCaloHitList->begin(), hitIterEnd = pCaloHitList->end(); hitIter != hitIterEnd; ++hitIter)
        {
            const CartesianVector &hitPosition((*hitIter)->GetPositionVector());
            const CartesianVector positionDifference(hitPosition - coneApex);

            try
            {
                const float cosTheta(coneDirection.GetDotProduct(positionDifference.GetUnitVector()));

                if (cosTheta > coneCosineHalfAngle)
                    nHitsInCone++;
            }
            catch (StatusCodeException &)
            {
                if (hitPosition == coneApex)
                    nHitsInCone++;
            }
        }
    }

    return static_cast<float>(nHitsInCone) / static_cast<float>(nCaloHits);
}

//------------------------------------------------------------------------------------------------------------------------------------------

unsigned int FragmentRemovalHelper::GetNLayersCrossed(const Pandora &pandora, const Helix &helix, const float zStart,
    const float zEnd, const unsigned int nSamplingPoints)
{
    if ((0 == nSamplingPoints) || (1000 < nSamplingPoints))
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);

    const float deltaZ((zEnd - zStart) / static_cast<float>(nSamplingPoints));

    if (std::fabs(deltaZ) < 0.001f)
        return 0;

    CartesianVector intersectionPoint(0.f, 0.f, 0.f);
    const CartesianVector &referencePoint(helix.GetReferencePoint());
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, != , helix.GetPointInZ(zStart, referencePoint, intersectionPoint));

    const PseudoLayerPlugin *const pPseudoLayerPlugin(pandora.GetPlugins()->GetPseudoLayerPlugin());
    const unsigned int MAX_LAYER(std::numeric_limits<unsigned int>::max());
    unsigned int startLayer(MAX_LAYER);

    try
    {
        startLayer = pPseudoLayerPlugin->GetPseudoLayer(intersectionPoint);
    }
    catch (StatusCodeException &)
    {
        return MAX_LAYER;
    }

    unsigned int currentLayer(startLayer), layerCount(0);

    for (float z = zStart; std::fabs(z) < std::fabs(zEnd + 0.5 * deltaZ); z += deltaZ)
    {
        PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, != , helix.GetPointInZ(z, referencePoint, intersectionPoint));

        try
        {
            const unsigned int iLayer(pPseudoLayerPlugin->GetPseudoLayer(intersectionPoint));

            if (iLayer != currentLayer)
            {
                layerCount += ((iLayer > currentLayer) ? iLayer - currentLayer : currentLayer - iLayer);
                currentLayer = iLayer;
            }
        }
        catch (StatusCodeException &)
        {
            continue;
        }
    }

    return layerCount;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode FragmentRemovalHelper::GetClusterHelixDistance(const Cluster *const pCluster, const Helix &helix, const unsigned int startLayer,
    const unsigned int endLayer, const unsigned int maxOccupiedLayers, float &closestDistanceToHit, float &meanDistanceToHits)
{
    if (startLayer > endLayer)
        return STATUS_CODE_INVALID_PARAMETER;

    unsigned int nHits(0), nOccupiedLayers(0);
    float sumDistanceToHits(0.), minDistanceToHit(std::numeric_limits<float>::max());
    const OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());

    for (unsigned int iLayer = startLayer; iLayer <= endLayer; ++iLayer)
    {
        OrderedCaloHitList::const_iterator iter = orderedCaloHitList.find(iLayer);

        if (orderedCaloHitList.end() == iter)
            continue;

        if (++nOccupiedLayers > maxOccupiedLayers)
            break;

        for (CaloHitList::const_iterator hitIter = iter->second->begin(), hitIterEnd = iter->second->end(); hitIter != hitIterEnd; ++hitIter)
        {
            CartesianVector distanceVector(0.f, 0.f, 0.f);
            PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, helix.GetDistanceToPoint((*hitIter)->GetPositionVector(), distanceVector));

            const float distance(distanceVector.GetZ());

            if (distance < minDistanceToHit)
                minDistanceToHit = distance;

            sumDistanceToHits += distance;
            nHits++;
        }
    }

    if (0 != nHits)
    {
        meanDistanceToHits = sumDistanceToHits / static_cast<float>(nHits);
        closestDistanceToHit = minDistanceToHit;
        return STATUS_CODE_SUCCESS;
    }

    return STATUS_CODE_NOT_FOUND;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode FragmentRemovalHelper::GetClusterContactDetails(const Cluster *const pClusterI, const Cluster *const pClusterJ,
    const float distanceThreshold, unsigned int &nContactLayers, float &contactFraction)
{
    const unsigned int startLayer(std::max(pClusterI->GetInnerPseudoLayer(), pClusterJ->GetInnerPseudoLayer()));
    const unsigned int endLayer(std::min(pClusterI->GetOuterPseudoLayer(), pClusterJ->GetOuterPseudoLayer()));
    const OrderedCaloHitList &orderedCaloHitListI(pClusterI->GetOrderedCaloHitList());
    const OrderedCaloHitList &orderedCaloHitListJ(pClusterJ->GetOrderedCaloHitList());

    unsigned int nLayersCompared(0), nLayersInContact(0);

    for (unsigned int iLayer = startLayer; iLayer <= endLayer; ++iLayer)
    {
        OrderedCaloHitList::const_iterator iterI = orderedCaloHitListI.find(iLayer), iterJ = orderedCaloHitListJ.find(iLayer);

        if ((orderedCaloHitListI.end() == iterI) || (orderedCaloHitListJ.end() == iterJ))
            continue;

        nLayersCompared++;
        bool isLayerDone(false);

        for (CaloHitList::const_iterator hitIterI = iterI->second->begin(), hitIterIEnd = iterI->second->end(); hitIterI != hitIterIEnd; ++hitIterI)
        {
            const CartesianVector &positionI((*hitIterI)->GetPositionVector());
            const float separationCut(1.5f * (*hitIterI)->GetCellLengthScale() * distanceThreshold);
            const float separationCutSquared(separationCut * separationCut);

            for (CaloHitList::const_iterator hitIterJ = iterJ->second->begin(), hitIterJEnd = iterJ->second->end(); hitIterJ != hitIterJEnd; ++hitIterJ)
            {
                const CartesianVector &positionJ((*hitIterJ)->GetPositionVector());
                const CartesianVector positionDifference(positionI - positionJ);
                const float separationSquared(positionDifference.GetMagnitudeSquared());

                if (separationSquared < separationCutSquared)
                {
                    nLayersInContact++;
                    isLayerDone = true;
                    break;
                }
            }

            if (isLayerDone)
                break;
        }
    }

    if (nLayersCompared > 0)
    {
        contactFraction = static_cast<float>(nLayersInContact) / static_cast<float>(nLayersCompared);
        nContactLayers = nLayersInContact;
        return STATUS_CODE_SUCCESS;
    }

    return STATUS_CODE_NOT_FOUND;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float FragmentRemovalHelper::GetEMEnergyWeightedLayerSeparation(const Cluster *const pClusterI, const Cluster *const pClusterJ)
{
    const unsigned int startPseudoLayer(std::max(pClusterJ->GetInnerPseudoLayer(), pClusterI->GetInnerPseudoLayer()));
    const unsigned int endPseudoLayer(std::min(pClusterJ->GetOuterPseudoLayer(), pClusterI->GetOuterPseudoLayer()));

    const OrderedCaloHitList &orderedCaloHitListI(pClusterI->GetOrderedCaloHitList());
    const OrderedCaloHitList &orderedCaloHitListJ(pClusterJ->GetOrderedCaloHitList());

    float energySum(0.f), weightedDistance(0.f);

    for (unsigned int pseudoLayer = startPseudoLayer; pseudoLayer <= endPseudoLayer; ++pseudoLayer)
    {
        bool isDistanceFound(false);
        float clusterJLayerEnergy(0.f), closestDistanceSquared(std::numeric_limits<float>::max());

        OrderedCaloHitList::const_iterator iterI = orderedCaloHitListI.find(pseudoLayer);
        OrderedCaloHitList::const_iterator iterJ = orderedCaloHitListJ.find(pseudoLayer);

        if ((orderedCaloHitListI.end() == iterI) || (orderedCaloHitListJ.end() == iterJ))
            continue;

        for (CaloHitList::const_iterator hIterJ = iterJ->second->begin(), hIterJEnd = iterJ->second->end(); hIterJ != hIterJEnd; ++hIterJ)
        {
            const CaloHit *const pCaloHitJ(*hIterJ);
            const CartesianVector &positionJ(pCaloHitJ->GetPositionVector());

            for (CaloHitList::const_iterator hIterI = iterI->second->begin(), hIterIEnd = iterI->second->end(); hIterI != hIterIEnd; ++hIterI)
            {
                const CaloHit *const pCaloHitI(*hIterI);
                const CartesianVector &positionI(pCaloHitI->GetPositionVector());

                const float distanceSquared((positionI - positionJ).GetMagnitudeSquared()); 

                if (distanceSquared < closestDistanceSquared)
                {
                    isDistanceFound = true;
                    closestDistanceSquared = distanceSquared;
                }
            }

            clusterJLayerEnergy += pCaloHitJ->GetElectromagneticEnergy();
        }

        if (!isDistanceFound)
            continue;

        // ATTN only uses cluster J for energy weighting per-layer
        energySum += clusterJLayerEnergy;
        weightedDistance += std::sqrt(closestDistanceSquared) * clusterJLayerEnergy;
    }

    if (energySum < std::numeric_limits<float>::epsilon())
        throw StatusCodeException(STATUS_CODE_NOT_FOUND);

    return (weightedDistance / energySum);
}

//------------------------------------------------------------------------------------------------------------------------------------------

CartesianVector FragmentRemovalHelper::GetEMEnergyWeightedPosition(const Cluster *const pCluster)
{
    float energySum(0.f);
    CartesianVector weightedPosition(0.f, 0.f, 0.f);

    const OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());

    for (OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), iterEnd = orderedCaloHitList.end(); iter != iterEnd; ++iter)
    {
        for (CaloHitList::const_iterator hIter = iter->second->begin(), hIterEnd = iter->second->end(); hIter != hIterEnd; ++hIter)
        {
            const CaloHit *const pCaloHit(*hIter);
            energySum += pCaloHit->GetElectromagneticEnergy();
            weightedPosition += pCaloHit->GetPositionVector() * pCaloHit->GetElectromagneticEnergy();
        }
    }

    if (energySum < std::numeric_limits<float>::epsilon())
        throw StatusCodeException(STATUS_CODE_NOT_FOUND);

    return (weightedPosition * (1.f / energySum));
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

ClusterContact::ClusterContact(const Pandora &pandora, const Cluster *const pDaughterCluster, const Cluster *const pParentCluster, const Parameters &parameters) :
    m_pDaughterCluster(pDaughterCluster),
    m_pParentCluster(pParentCluster),
    m_nContactLayers(0),
    m_contactFraction(0.f),
    m_coneFraction1(FragmentRemovalHelper::GetFractionOfHitsInCone(pandora, pDaughterCluster, pParentCluster, parameters.m_coneCosineHalfAngle1)),
    m_closeHitFraction1(0.f),
    m_closeHitFraction2(0.f),
    m_distanceToClosestHit(std::numeric_limits<float>::max())
{
    (void) FragmentRemovalHelper::GetClusterContactDetails(pDaughterCluster, pParentCluster, parameters.m_distanceThreshold,
        m_nContactLayers, m_contactFraction);

    this->HitDistanceComparison(pDaughterCluster, pParentCluster, parameters);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void ClusterContact::HitDistanceComparison(const Cluster *const pDaughterCluster, const Cluster *const pParentCluster, const Parameters &parameters)
{
    const float closeHitDistance1Squared(parameters.m_closeHitDistance1 * parameters.m_closeHitDistance1);
    const float closeHitDistance2Squared(parameters.m_closeHitDistance2 * parameters.m_closeHitDistance2);

    // Apply simple preselection using cosine of opening angle between the clusters
    const float cosOpeningAngle(pDaughterCluster->GetInitialDirection().GetCosOpeningAngle(pParentCluster->GetInitialDirection()));

    if (cosOpeningAngle < parameters.m_minCosOpeningAngle)
        return;

    // Calculate all hit distance properties in a single loop, for efficiency
    unsigned int nCloseHits1(0), nCloseHits2(0);
    float minDistanceSquared(std::numeric_limits<float>::max());

    const OrderedCaloHitList &orderedCaloHitListI(pDaughterCluster->GetOrderedCaloHitList());
    const OrderedCaloHitList &orderedCaloHitListJ(pParentCluster->GetOrderedCaloHitList());

    // Loop over hits in daughter cluster
    for (OrderedCaloHitList::const_iterator iterI = orderedCaloHitListI.begin(), iterIEnd = orderedCaloHitListI.end(); iterI != iterIEnd; ++iterI)
    {
        for (CaloHitList::const_iterator hitIterI = iterI->second->begin(), hitIterIEnd = iterI->second->end(); hitIterI != hitIterIEnd; ++hitIterI)
        {
            bool isCloseHit1(false), isCloseHit2(false);
            const CartesianVector &positionVectorI((*hitIterI)->GetPositionVector());

            // Compare each hit in daughter cluster with those in parent cluster
            for (OrderedCaloHitList::const_iterator iterJ = orderedCaloHitListJ.begin(), iterJEnd = orderedCaloHitListJ.end(); iterJ != iterJEnd; ++iterJ)
            {
                for (CaloHitList::const_iterator hitIterJ = iterJ->second->begin(), hitIterJEnd = iterJ->second->end(); hitIterJ != hitIterJEnd; ++hitIterJ)
                {
                    const float distanceSquared((positionVectorI - (*hitIterJ)->GetPositionVector()).GetMagnitudeSquared());

                    if (!isCloseHit1 && (distanceSquared < closeHitDistance1Squared))
                        isCloseHit1 = true;

                    if (!isCloseHit2 && (distanceSquared < closeHitDistance2Squared))
                        isCloseHit2 = true;

                    if (distanceSquared < minDistanceSquared)
                        minDistanceSquared = distanceSquared;
                }
            }

            if (isCloseHit1)
                nCloseHits1++;

            if (isCloseHit2)
                nCloseHits2++;
        }
    }

    const unsigned int nDaughterCaloHits(pDaughterCluster->GetNCaloHits());

    if (0 == nDaughterCaloHits)
        throw StatusCodeException(STATUS_CODE_INVALID_PARAMETER);

    m_distanceToClosestHit = std::sqrt(minDistanceSquared);
    m_closeHitFraction1 = static_cast<float>(nCloseHits1) / static_cast<float>(nDaughterCaloHits);
    m_closeHitFraction2 = static_cast<float>(nCloseHits2) / static_cast<float>(nDaughterCaloHits);
}

}
