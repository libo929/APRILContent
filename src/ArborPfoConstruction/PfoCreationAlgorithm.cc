  /// \file PfoCreationAlgorithm.cc
/*
 *
 * PfoCreationAlgorithm.cc source template automatically generated by a class generator
 * Creation date : sam. mars 21 2015
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
 * @author John Marshall
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborPfoConstruction/PfoCreationAlgorithm.h"

namespace arbor_content
{

PfoCreationAlgorithm::PfoCreationAlgorithm() :
    m_shouldCreateTrackBasedPfos(true),
    m_shouldCreateNeutralPfos(true),
    m_minClusterHadronicEnergy(0.1f),
    m_minClusterElectromagneticEnergy(0.1f),
    m_minHitsInCluster(5),
    m_allowSingleLayerClusters(false),
    m_photonPositionAlgorithm(2)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::Run()
{
    const pandora::PfoList *pPfoList = NULL; std::string pfoListName;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pPfoList, pfoListName));

    if (m_shouldCreateTrackBasedPfos)
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CreateTrackBasedPfos());

    if (m_shouldCreateNeutralPfos)
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CreateNeutralPfos());

    if (!pPfoList->empty())
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<pandora::Pfo>(*this, m_outputPfoListName));
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::ReplaceCurrentList<pandora::Pfo>(*this, m_outputPfoListName));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::CreateTrackBasedPfos() const
{
    // Current track list should contain those tracks selected as "good" by the track preparation algorithm
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList<pandora::TrackList>(*this, pTrackList));

    for (pandora::TrackList::const_iterator iter = pTrackList->begin(), iterEnd = pTrackList->end(); iter != iterEnd; ++iter)
    {
        const pandora::Track *const pTrack = *iter;
        PandoraContentApi::ParticleFlowObject::Parameters pfoParameters;

        // Walk along list of associated daughter/sibling tracks and their cluster associations
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->PopulateTrackBasedPfo(pTrack, pfoParameters));

        // Specify the pfo parameters
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->SetTrackBasedPfoParameters(pTrack, pfoParameters));

        // Create the pfo
        const pandora::ParticleFlowObject *pPfo(NULL);
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::ParticleFlowObject::Create(*this, pfoParameters, pPfo));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::PopulateTrackBasedPfo(const pandora::Track *const pTrack, PfoParameters &pfoParameters, const bool readSiblingInfo) const
{
    // Add track to the pfo
    pfoParameters.m_trackList.insert(pTrack);

    // Add any cluster associated with this track to the pfo
    try
    {
        const pandora::Cluster *const pAssociatedCluster(pTrack->GetAssociatedCluster());
        pfoParameters.m_clusterList.insert(pAssociatedCluster);
    }
    catch (pandora::StatusCodeException &)
    {
    }

    // Consider any sibling tracks
    if (readSiblingInfo)
    {
        const pandora::TrackList &siblingTrackList(pTrack->GetSiblingTrackList());

        for (pandora::TrackList::const_iterator iter = siblingTrackList.begin(), iterEnd = siblingTrackList.end(); iter != iterEnd; ++iter)
        {
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->PopulateTrackBasedPfo(*iter, pfoParameters, false));
        }
    }

    // Consider any daughter tracks
    const pandora::TrackList &daughterTrackList(pTrack->GetDaughterTrackList());

    for (pandora::TrackList::const_iterator iter = daughterTrackList.begin(), iterEnd = daughterTrackList.end(); iter != iterEnd; ++iter)
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->PopulateTrackBasedPfo(*iter, pfoParameters));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::SetTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const
{
    const bool hasParent(!pTrack->GetParentTrackList().empty());

    if (hasParent)
        return pandora::STATUS_CODE_NOT_ALLOWED;

    const bool hasSibling(!pTrack->GetSiblingTrackList().empty());
    const bool hasDaughter(!pTrack->GetDaughterTrackList().empty());

    if (hasSibling && hasDaughter)
        return pandora::STATUS_CODE_NOT_ALLOWED;

    if (hasSibling)
        return this->SetSiblingTrackBasedPfoParameters(pTrack, pfoParameters);

    if (hasDaughter)
        return this->SetDaughterTrackBasedPfoParameters(pTrack, pfoParameters);

    return this->SetSimpleTrackBasedPfoParameters(pTrack, pfoParameters);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::SetSiblingTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const
{
    int charge(0);
    float energy(0.f);
    pandora::CartesianVector momentum(0.f, 0.f, 0.f);

    pandora::TrackList fullSiblingTrackList(pTrack->GetSiblingTrackList());
    fullSiblingTrackList.insert(pTrack);

    for (pandora::TrackList::const_iterator iter = fullSiblingTrackList.begin(), iterEnd = fullSiblingTrackList.end(); iter != iterEnd; ++iter)
    {
        const pandora::Track *const pSiblingTrack = *iter;
        charge += pSiblingTrack->GetCharge();

        if (!pSiblingTrack->CanFormPfo() && !pSiblingTrack->CanFormClusterlessPfo())
            continue;

        // ATTN Assume sibling-track-based pfos represent pair-production
        const float electronMass(pandora::PdgTable::GetParticleMass(pandora::E_MINUS));
        energy += std::sqrt(electronMass * electronMass + pSiblingTrack->GetMomentumAtDca().GetMagnitudeSquared());
        momentum += pSiblingTrack->GetMomentumAtDca();
    }

    if (energy < std::numeric_limits<float>::epsilon())
        return pandora::STATUS_CODE_NOT_INITIALIZED;

    pfoParameters.m_energy = energy;
    pfoParameters.m_momentum = momentum;
    pfoParameters.m_mass = std::sqrt(std::max(energy * energy - momentum.GetDotProduct(momentum), 0.f));
    pfoParameters.m_charge = charge;
    pfoParameters.m_particleId = pandora::PHOTON;

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::SetDaughterTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const
{
    int daughterCharge(0);
    float energy(0.f);
    pandora::CartesianVector momentum(0.f, 0.f, 0.f);

    const pandora::TrackList &daughterTrackList(pTrack->GetDaughterTrackList());
    const unsigned int nDaughters(daughterTrackList.size());

    for (pandora::TrackList::const_iterator iter = daughterTrackList.begin(), iterEnd = daughterTrackList.end(); iter != iterEnd; ++iter)
    {
        const pandora::Track *const pDaughterTrack = *iter;

        if (!pDaughterTrack->CanFormPfo() && !pDaughterTrack->CanFormClusterlessPfo())
            continue;

        daughterCharge += pDaughterTrack->GetCharge();
        energy += pDaughterTrack->GetEnergyAtDca();
        momentum += pDaughterTrack->GetMomentumAtDca();
    }

    pfoParameters.m_energy = energy;
    pfoParameters.m_momentum = momentum;
    pfoParameters.m_mass = std::sqrt(std::max(energy * energy - momentum.GetDotProduct(momentum), 0.f));
    pfoParameters.m_charge = (nDaughters > 1) ? pTrack->GetCharge() : daughterCharge;
    pfoParameters.m_particleId = (pfoParameters.m_charge.Get() > 0) ? pandora::PI_PLUS : pandora::PI_MINUS;

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::SetSimpleTrackBasedPfoParameters(const pandora::Track *const pTrack, PfoParameters &pfoParameters) const
{
    pfoParameters.m_energy = pTrack->GetEnergyAtDca();
    pfoParameters.m_momentum = pTrack->GetMomentumAtDca();
    pfoParameters.m_mass = pTrack->GetMass();
    pfoParameters.m_charge = pTrack->GetCharge();
    pfoParameters.m_particleId = (pTrack->GetCharge() > 0) ? pandora::PI_PLUS : pandora::PI_MINUS;

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::CreateNeutralPfos() const
{
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

    // Examine clusters with no associated tracks to form neutral pfos
    for (pandora::ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
        const pandora::Cluster *const pCluster = *iter;

        if (!pCluster->GetAssociatedTrackList().empty())
            continue;

        if (pCluster->GetNCaloHits() < m_minHitsInCluster)
            continue;

        const bool isPhoton(pCluster->IsPhotonFast(this->GetPandora()));
        float clusterEnergy(isPhoton ? pCluster->GetCorrectedElectromagneticEnergy(this->GetPandora()) : pCluster->GetCorrectedHadronicEnergy(this->GetPandora()));

        // Veto non-photon clusters below hadronic energy threshold and those occupying a single layer
        if (!isPhoton)
        {
            if (clusterEnergy < m_minClusterHadronicEnergy)
                continue;

            if (!m_allowSingleLayerClusters && (pCluster->GetInnerPseudoLayer() == pCluster->GetOuterPseudoLayer()))
                continue;
        }
        else
        {
            if (clusterEnergy < m_minClusterElectromagneticEnergy)
                continue;
        }

        // Specify the pfo parameters
        PandoraContentApi::ParticleFlowObject::Parameters pfoParameters;
        pfoParameters.m_particleId = (isPhoton ? pandora::PHOTON : pandora::NEUTRON);
        pfoParameters.m_charge = 0;
        pfoParameters.m_mass = (isPhoton ? pandora::PdgTable::GetParticleMass(pandora::PHOTON) : pandora::PdgTable::GetParticleMass(pandora::NEUTRON));
        pfoParameters.m_energy = clusterEnergy;
        pfoParameters.m_clusterList.insert(pCluster);

        // Photon position: 0) unweighted inner centroid, 1) energy-weighted inner centroid, 2+) energy-weighted centroid for all layers
        pandora::CartesianVector positionVector(0.f, 0.f, 0.f);
        const unsigned int clusterInnerLayer(pCluster->GetInnerPseudoLayer());

        if (!isPhoton || (0 == m_photonPositionAlgorithm))
        {
            positionVector = pCluster->GetCentroid(clusterInnerLayer);
        }
        else if (1 == m_photonPositionAlgorithm)
        {
            positionVector = this->GetEnergyWeightedCentroid(pCluster, clusterInnerLayer, clusterInnerLayer);
        }
        else
        {
            positionVector = this->GetEnergyWeightedCentroid(pCluster, clusterInnerLayer, pCluster->GetOuterPseudoLayer());
        }

        const pandora::CartesianVector momentum(positionVector.GetUnitVector() * clusterEnergy);
        pfoParameters.m_momentum = momentum;

        const pandora::ParticleFlowObject *pPfo(NULL);
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::ParticleFlowObject::Create(*this, pfoParameters, pPfo));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

const pandora::CartesianVector PfoCreationAlgorithm::GetEnergyWeightedCentroid(const pandora::Cluster *const pCluster, const unsigned int innerPseudoLayer,
    const unsigned int outerPseudoLayer) const
{
    float energySum(0.f);
    pandora::CartesianVector energyPositionSum(0.f, 0.f, 0.f);
    const pandora::OrderedCaloHitList &orderedCaloHitList(pCluster->GetOrderedCaloHitList());

    for (pandora::OrderedCaloHitList::const_iterator iter = orderedCaloHitList.begin(), iterEnd = orderedCaloHitList.end(); iter != iterEnd; ++iter)
    {
        if (iter->first > outerPseudoLayer)
            break;

        if (iter->first < innerPseudoLayer)
            continue;

        for (pandora::CaloHitList::const_iterator hitIter = iter->second->begin(), hitIterEnd = iter->second->end(); hitIter != hitIterEnd; ++hitIter)
        {
            const float electromagneticEnergy((*hitIter)->GetElectromagneticEnergy());
            energySum += electromagneticEnergy;
            energyPositionSum += ((*hitIter)->GetPositionVector() * electromagneticEnergy);
        }
    }

    if (energySum < std::numeric_limits<float>::epsilon())
        throw pandora::StatusCodeException(pandora::STATUS_CODE_NOT_INITIALIZED);

    return (energyPositionSum * (1.f / energySum));
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode PfoCreationAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "OutputPfoListName", m_outputPfoListName));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldCreateTrackBasedPfos", m_shouldCreateTrackBasedPfos));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldCreateNeutralPfos", m_shouldCreateNeutralPfos));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterHadronicEnergy", m_minClusterHadronicEnergy));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterElectromagneticEnergy", m_minClusterElectromagneticEnergy));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinHitsInCluster", m_minHitsInCluster));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "AllowSingleLayerClusters", m_allowSingleLayerClusters));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "PhotonPositionAlgorithm", m_photonPositionAlgorithm));

    return pandora::STATUS_CODE_SUCCESS;
}

}
