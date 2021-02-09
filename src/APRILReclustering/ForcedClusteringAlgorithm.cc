/**
 *  @file   LCContent/src/LCClustering/ForcedClusteringAlgorithm.cc
 * 
 *  @brief  Implementation of the forced clustering algorithm class.
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILReclustering/ForcedClusteringAlgorithm.h"

namespace april_content
{

ForcedClusteringAlgorithm::ForcedClusteringAlgorithm() :
    m_shouldRunStandardClusteringAlgorithm(false),
    m_shouldClusterIsolatedHits(false),
    m_shouldAssociateIsolatedHits(false)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ForcedClusteringAlgorithm::Run()
{
    // Read current track list
    const pandora::TrackList *pTrackList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pTrackList));

    if (pTrackList->empty())
        return pandora::STATUS_CODE_INVALID_PARAMETER;

    // Read current calo hit list
    const pandora::CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    if (pCaloHitList->empty())
        return pandora::STATUS_CODE_INVALID_PARAMETER;

    // Make new track-seeded clusters and populate track distance info vector
    TrackDistanceInfoVector trackDistanceInfoVector;
    const float bField(PandoraContentApi::GetPlugins(*this)->GetBFieldPlugin()->GetBField(pandora::CartesianVector(0.f, 0.f, 0.f)));

    for (pandora::TrackList::const_iterator iter = pTrackList->begin(), iterEnd = pTrackList->end(); iter != iterEnd; ++iter)
    {
        const pandora::Track *const pTrack = *iter;
			
        const float trackEnergy(pTrack->GetEnergyAtDca());
        const pandora::Helix helix(pTrack->GetTrackStateAtCalorimeter().GetPosition(), pTrack->GetTrackStateAtCalorimeter().GetMomentum(), pTrack->GetCharge(), bField);

        const pandora::Cluster *pCluster = NULL;
        PandoraContentApi::Cluster::Parameters parameters;
        parameters.m_pTrack = pTrack;
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));

        for (pandora::CaloHitList::const_iterator hitIter = pCaloHitList->begin(), hitIterEnd = pCaloHitList->end(); hitIter != hitIterEnd; ++hitIter)
        {
            const pandora::CaloHit *const pCaloHit = *hitIter;

            if ((m_shouldClusterIsolatedHits || !pCaloHit->IsIsolated()) && PandoraContentApi::IsAvailable(*this, pCaloHit))
            {
				pandora::CartesianVector helixSeparation(0.f, 0.f, 0.f);
                PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, helix.GetDistanceToPoint(pCaloHit->GetPositionVector(), helixSeparation));

                trackDistanceInfoVector.push_back(TrackDistanceInfo(pCaloHit, pCluster, pTrack, trackEnergy, helixSeparation.GetMagnitude()));
            }
        }
    }

    std::sort(trackDistanceInfoVector.begin(), trackDistanceInfoVector.end(), ForcedClusteringAlgorithm::SortByDistanceToTrack);

    // Work along ordered list of calo hits, adding to the clusters until each cluster energy matches associated track energy.
    for (TrackDistanceInfoVector::const_iterator iter = trackDistanceInfoVector.begin(), iterEnd = trackDistanceInfoVector.end(); iter != iterEnd; ++iter)
    {
        const pandora::Cluster *const pCluster = iter->GetCluster();
        const pandora::Track   *const pTrack   = iter->GetTrack  ();
        const pandora::CaloHit *const pCaloHit = iter->GetCaloHit();

        const float trackEnergy = iter->GetTrackEnergy();

		// Get track MCP
		const pandora::MCParticle *pTrackMCParticle = nullptr;

        try
        {
		    pTrackMCParticle = pandora::MCParticleHelper::GetMainMCParticle( pTrack );
        }
        catch (pandora::StatusCodeException &)
        {
			continue;
        }

		// Get hit MCP
		const pandora::MCParticle *pHitMCParticle = nullptr;

        try
        {
		    pHitMCParticle = pandora::MCParticleHelper::GetMainMCParticle( pCaloHit );
        }
        catch (pandora::StatusCodeException &)
        {
			continue;
        }

		bool canAddbyMC = pTrackMCParticle != nullptr && pHitMCParticle != nullptr && pTrackMCParticle == pHitMCParticle;

        //if ((pCluster->GetHadronicEnergy() < trackEnergy) && PandoraContentApi::IsAvailable(*this, pCaloHit))
        if (canAddbyMC && PandoraContentApi::IsAvailable(*this, pCaloHit))
        {
			//std::cout << "cluster " << pCluster << " add hit " << pCaloHit << ", trkMC: " << pTrackMCParticle << ", hitMC: " << pHitMCParticle << std::endl;
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddToCluster(*this, pCluster, pCaloHit));
        }
    }

    // Deal with remaining hits. Either run standard clustering algorithm, or crudely collect together into one cluster
    if (m_shouldRunStandardClusteringAlgorithm)
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_standardClusteringAlgorithmName));
    }
    else
    {
        PandoraContentApi::Cluster::Parameters remnantParameters;
		pandora::CaloHitList &remnantCaloHitList(remnantParameters.m_caloHitList);

        for (pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), iterEnd = pCaloHitList->end(); iter != iterEnd; ++iter)
        {
            if ((m_shouldClusterIsolatedHits || !(*iter)->IsIsolated()) && PandoraContentApi::IsAvailable(*this, *iter))
                remnantCaloHitList.push_back(*iter);
        }

        if (!remnantCaloHitList.empty())
        {
            const pandora::Cluster *pRemnantCluster = NULL;
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, remnantParameters, pRemnantCluster));
        }
    }

    // If specified, associate isolated hits with the newly formed clusters
    if (m_shouldAssociateIsolatedHits)
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::RunDaughterAlgorithm(*this, m_isolatedHitAssociationAlgorithmName));
    }

    // Delete any empty clusters
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->RemoveEmptyClusters());

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ForcedClusteringAlgorithm::RemoveEmptyClusters() const
{
    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	pandora::ClusterList clusterDeletionList;

    for (pandora::ClusterList::const_iterator iter = pClusterList->begin(), iterEnd = pClusterList->end(); iter != iterEnd; ++iter)
    {
        if (0 == (*iter)->GetNCaloHits())
            clusterDeletionList.push_back(*iter);
    }

    if (!clusterDeletionList.empty())
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::Delete(*this, &clusterDeletionList));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ForcedClusteringAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldRunStandardClusteringAlgorithm", m_shouldRunStandardClusteringAlgorithm));

    if (m_shouldRunStandardClusteringAlgorithm)
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithm(*this, xmlHandle, "StandardClustering",
            m_standardClusteringAlgorithmName));
    }

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldClusterIsolatedHits", m_shouldClusterIsolatedHits));

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldAssociateIsolatedHits", m_shouldAssociateIsolatedHits));

    if (m_shouldAssociateIsolatedHits)
    {
        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pandora::XmlHelper::ProcessAlgorithm(*this, xmlHandle, "IsolatedHitAssociation",
            m_isolatedHitAssociationAlgorithmName));
    }

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace lc_content
