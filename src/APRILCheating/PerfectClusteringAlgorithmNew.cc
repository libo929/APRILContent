/**
 *  @file   LCContent/src/LCCheating/PerfectClusteringAlgorithmNew.cc
 * 
 *  @brief  Implementation of the perfect particle flow algorithm class
 * 
 *  @author J. Marshall
 *  @comment Copied from https://github.com/PandoraPFA/LCContent
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/PerfectClusteringAlgorithmNew.h" // changed path

using namespace pandora;

namespace april_content // changed namespace
{

const MCParticle* GetCaloHitMainMCParticle(const CaloHit *const pCaloHit)
{
    float bestWeight(0.f);
    const MCParticle *pBestMCParticle(nullptr);
    const MCParticleWeightMap &hitMCParticleWeightMap(pCaloHit->GetMCParticleWeightMap());

    MCParticleVector mcParticleVector;
    for (const MCParticleWeightMap::value_type &mapEntry : hitMCParticleWeightMap) mcParticleVector.push_back(mapEntry.first);
    std::sort(mcParticleVector.begin(), mcParticleVector.end(), PointerLessThan<MCParticle>());

    for (const MCParticle *const pMCParticle : mcParticleVector)
    {
        const float weight(hitMCParticleWeightMap.at(pMCParticle));

        if (weight > bestWeight)
        {
            bestWeight = weight;
            pBestMCParticle = pMCParticle;
        }
    }

    if (!pBestMCParticle)
	{
		//std::cout << "hitMCParticleWeightMap size: " <<  hitMCParticleWeightMap.size() << std::endl;
        throw StatusCodeException(STATUS_CODE_NOT_INITIALIZED);
	}

    return pBestMCParticle;
}


PerfectClusteringAlgorithmNew::PerfectClusteringAlgorithmNew() :
    m_simpleCaloHitCollection(true),
    m_minWeightFraction(0.01f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PerfectClusteringAlgorithmNew::Run()
{
    const MCParticleList *pMCParticleList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pMCParticleList));

	std::cout << "PerfectClusteringAlgorithmNew, # MCP: " << pMCParticleList->size() << std::endl;

    if (pMCParticleList->empty())
        return STATUS_CODE_SUCCESS;

    const ClusterList *pClusterList = NULL; std::string clusterListName;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pClusterList, clusterListName));

	int pfoTargetNum = 0;

    for (MCParticleList::const_iterator iterMC = pMCParticleList->begin(), iterMCEnd = pMCParticleList->end(); iterMC != iterMCEnd; ++iterMC)
    {
        try
        {
			if((*iterMC)->IsPfoTarget()) ++pfoTargetNum;

            const MCParticle *const pPfoTarget = *iterMC;
            PfoParameters pfoParameters;

            this->CaloHitCollection(pPfoTarget, pfoParameters);
        }
        catch (StatusCodeException &)
        {
        }
    }

	std::cout << "mcp #: " << pMCParticleList->size() << std::endl;
	std::cout << "pfo target #: " << pfoTargetNum << std::endl;

    if (!pClusterList->empty())
    {
		std::string tupleName = "PerfectClusteringAlgorithmNew-" + string(__func__);
		std::string varListName = "clusterSize";
		std::vector<float> vars;

		vars.push_back(pClusterList->size());

		HistogramManager::CreateFill(tupleName, varListName, vars);

        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, m_outputClusterListName));

        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::ReplaceCurrentList<Cluster>(*this, m_outputClusterListName));
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PerfectClusteringAlgorithmNew::CaloHitCollection(const MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const
{
    const CaloHitList *pCaloHitList = NULL;
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    const Cluster *pCluster = NULL;
    const CaloHitList localCaloHitList(pCaloHitList->begin(), pCaloHitList->end());

    for (CaloHitList::const_iterator iter = localCaloHitList.begin(), iterEnd = localCaloHitList.end(); iter != iterEnd; ++iter)
    {
        try
        {
            const CaloHit *const pCaloHit = *iter;

            if (!PandoraContentApi::IsAvailable(*this, pCaloHit))
                continue;

            PandoraContentApi::Cluster::Parameters parameters;
            CaloHitList &caloHitList(parameters.m_caloHitList);

            if (m_simpleCaloHitCollection)
            {
                this->SimpleCaloHitCollection(pPfoTarget, pCaloHit, caloHitList);
            }
            else
            {
                this->FullCaloHitCollection(pPfoTarget, pCaloHit, caloHitList);
            }

            if (caloHitList.empty())
                continue;

            if (NULL == pCluster)
            {
                PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));
                pfoParameters.m_clusterList.push_back(pCluster);

				// FIXME
				// it seems we need to add this hit, but it throw the result when running ?
#if 0
                for (CaloHitList::const_iterator hitIter = caloHitList.begin(), hitIterEnd = caloHitList.end(); hitIter != hitIterEnd; ++hitIter)
                    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddToCluster(*this, pCluster, *hitIter));
#endif
            }
            else
            {
                for (CaloHitList::const_iterator hitIter = caloHitList.begin(), hitIterEnd = caloHitList.end(); hitIter != hitIterEnd; ++hitIter)
                    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::AddToCluster(*this, pCluster, *hitIter));
            }
        }
        catch (StatusCodeException &e)
        {
			//std::cout << e.ToString() << std::endl;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PerfectClusteringAlgorithmNew::SimpleCaloHitCollection(const MCParticle *const pPfoTarget, const CaloHit *const pCaloHit, CaloHitList &caloHitList) const
{
    const MCParticle *const pHitMCParticle(GetCaloHitMainMCParticle(pCaloHit));
    const MCParticle *pHitPfoTarget(pHitMCParticle->GetPfoTarget());

    //////////////////////////////////
    // use the direct MCP of calo hit
    //////////////////////////////////
    pHitPfoTarget = pHitMCParticle;


    if(pHitMCParticle != pHitPfoTarget ) 
    {
        std::cout << "hit mcp: " << pHitMCParticle << ", pfo: " << pHitPfoTarget << std::endl;
    }

    if (pHitPfoTarget != pPfoTarget)
        return;

    caloHitList.push_back(pCaloHit);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PerfectClusteringAlgorithmNew::FullCaloHitCollection(const MCParticle *const pPfoTarget, const CaloHit *const pCaloHit, CaloHitList &caloHitList) const
{
    const MCParticleWeightMap mcParticleWeightMap(pCaloHit->GetMCParticleWeightMap());

    if (mcParticleWeightMap.size() < 2)
        return this->SimpleCaloHitCollection(pPfoTarget, pCaloHit, caloHitList);

    float mcParticleWeightSum(0.f);

    MCParticleList mcParticleList;
    for (const auto &mapEntry : mcParticleWeightMap) mcParticleList.push_back(mapEntry.first);
    mcParticleList.sort(PointerLessThan<MCParticle>());

    for (const MCParticle *const pMCParticle : mcParticleList)
        mcParticleWeightSum += mcParticleWeightMap.at(pMCParticle);

    if (mcParticleWeightSum < std::numeric_limits<float>::epsilon())
        throw StatusCodeException(STATUS_CODE_FAILURE);

    const CaloHit *pLocalCaloHit = pCaloHit;

    for (const MCParticle *const pHitMCParticle : mcParticleList)
    {
        const MCParticle *const pHitPfoTarget(pHitMCParticle->GetPfoTarget());
        const float weight(mcParticleWeightMap.at(pHitMCParticle));

        if (pHitPfoTarget != pPfoTarget)
            continue;

        const CaloHit *pCaloHitToAdd = pLocalCaloHit;

        if (pCaloHitToAdd->GetWeight() < std::numeric_limits<float>::epsilon())
            throw StatusCodeException(STATUS_CODE_FAILURE);

        const float weightFraction(weight / (mcParticleWeightSum * pCaloHitToAdd->GetWeight()));
        const bool isBelowThreshold((weightFraction - m_minWeightFraction) < std::numeric_limits<float>::epsilon());

        if (isBelowThreshold)
            continue;

        const bool shouldFragment(weightFraction + m_minWeightFraction - 1.f < std::numeric_limits<float>::epsilon());

        if (shouldFragment)
            PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Fragment(*this, pLocalCaloHit, weightFraction, pCaloHitToAdd, pLocalCaloHit));

        caloHitList.push_back(pCaloHitToAdd);
    }
}

//------------------------------------------------------------------------------------------------------------------------------------------
StatusCode PerfectClusteringAlgorithmNew::ReadSettings(const TiXmlHandle xmlHandle)
{
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
        "OutputClusterListName", m_outputClusterListName));

    return STATUS_CODE_SUCCESS;
}

} // namespace lc_content
