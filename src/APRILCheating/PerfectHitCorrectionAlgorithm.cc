/**
 *  @file   LCContent/src/LCCheating/PerfectHitCorrectionAlgorithm.cc
 * 
 *  @brief  Implementation of the perfect particle flow algorithm class
 * 
 *  @author J. Marshall
 *  @comment Copied from https://github.com/PandoraPFA/LCContent
 *
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/PerfectHitCorrectionAlgorithm.h" // changed path

using namespace pandora;

namespace april_content // changed namespace
{

PerfectHitCorrectionAlgorithm::PerfectHitCorrectionAlgorithm() :
    m_simpleCaloHitCollection(true),
    m_minWeightFraction(0.01f)
{
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode PerfectHitCorrectionAlgorithm::Run()
{
    const MCParticleList *pMCParticleList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pMCParticleList));

    if (pMCParticleList->empty())
        return STATUS_CODE_SUCCESS;

	std::map<const MCParticle*, const Cluster*> mcpClusterMap;

    for (MCParticleList::const_iterator iterMC = pMCParticleList->begin(), iterMCEnd = pMCParticleList->end(); iterMC != iterMCEnd; ++iterMC)
    {
        try
        {
            const MCParticle *const pPfoTarget = *iterMC;
            //PfoParameters pfoParameters;

			if(mcpClusterMap.find(pPfoTarget) == mcpClusterMap.end())
			{
				mcpClusterMap.insert(std::map<const MCParticle*, const Cluster*>::value_type(pPfoTarget, 0));
			}

            //this->CaloHitCollection(pPfoTarget, pfoParameters);
        }
        catch (StatusCodeException &)
        {
        }
    }


	////////////////////////////////////////
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	std::cout << "Hit Correction -----> list : " << pClusterList->size() << std::endl;
    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());

    for (ClusterList::const_iterator clusterIter = localClusterList.begin(), clusterIterEnd = localClusterList.end(); 
			clusterIter != clusterIterEnd; ++clusterIter)
    {
      try
      {
        const Cluster *const pCluster = *clusterIter;

		const MCParticle *pClusterMCParticle = MCParticleHelper::GetMainMCParticle(pCluster);
		std::cout << "cluster: " << pCluster << ", MCP : " << pClusterMCParticle 
			      << ", size: " << ", energy: " << pCluster->GetHadronicEnergy() << std::endl;

		auto mcpClusterIter = mcpClusterMap.find(pClusterMCParticle);

		if(mcpClusterIter != mcpClusterMap.end())
		{
			mcpClusterIter->second = pCluster;
		}


		// clear hit which has wrong relationship
		const OrderedCaloHitList& orderedCaloHitList = pCluster->GetOrderedCaloHitList();

        pandora::CaloHitList hitList;
    	orderedCaloHitList.FillCaloHitList(hitList);

		for(CaloHitList::const_iterator hitIter = hitList.begin(); hitIter != hitList.end(); ++hitIter)
		{
			//std::cout << " --------->>>>> try a calo hit: " << *hitIter << std::endl;
			const CaloHit* pCaloHit = *hitIter;

	        try
	        {
               const MCParticle *const pHitMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));
			   const MCParticle *const pHitPfoTarget(pHitMCParticle->GetPfoTarget());


			   if(pHitPfoTarget != pClusterMCParticle)
			   {
				   // FIXME
			       std::cout << "  *** remove hit " << pCaloHit << " from cluster " << pCluster << std::endl;
				   PandoraContentApi::RemoveFromCluster(*this, pCluster, pCaloHit);
			   }
			}
            catch (StatusCodeException &)
            {
		    	continue;
            }
		}
	  }
      catch (StatusCodeException &)
      {
		  continue;
      }
	}


	///////////// Isolated hits and unlinked hits
	//
    const CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    CaloHitList localCaloHitList(pCaloHitList->begin(), pCaloHitList->end());

	double isoHitEnergy = 0.;

    for (CaloHitList::const_iterator caloHitIter = localCaloHitList.begin(), caloHitIterEnd = localCaloHitList.end(); 
			caloHitIter != caloHitIterEnd; ++caloHitIter)
    {
      try
      {
        const CaloHit *const pCaloHit = *caloHitIter;

        if (!PandoraContentApi::IsAvailable(*this, pCaloHit))
          continue;

        const MCParticle *const pHitMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));
		//const MCParticle *const pHitPfoTarget(pHitMCParticle->GetPfoTarget());

		auto mcpClusterIter = mcpClusterMap.find(pHitMCParticle);

		if(mcpClusterIter != mcpClusterMap.end())
		{
			auto pCluster = mcpClusterIter->second;

			// FIXME
			if(pCluster!=NULL) 
			{
			    std::cout << "  *** add hit " << pCaloHit << " to cluster " << pCluster << std::endl;
				PandoraContentApi::AddToCluster(*this, pCluster, pCaloHit);
			}
			else
			{
				isoHitEnergy += pCaloHit->GetHadronicEnergy();
			}
		}

        //this->SimpleMCParticleCaloHitListCollection(pCaloHit, mcParticleToCaloHitListMap);
      }
      catch (StatusCodeException &)
      {
      }
    }

	std::cout << "energy of iso hits: " << isoHitEnergy << std::endl;

#if 0
    ClusterList localClusterList(pClusterList->begin(), pClusterList->end());
    MCParticleToClusterListMap mcParticleToClusterListMap;
	
	if(!localClusterList.empty())
	{
		for(ClusterList::const_iterator iter = localClusterList.begin(); iter != localClusterList.end(); ++iter)
		{
			const Cluster* const pCluster = *iter;
			SimpleMCParticleClusterListCollection(pCluster, mcParticleToClusterListMap);
		}
	}
#endif
    return STATUS_CODE_SUCCESS;

}

//------------------------------------------------------------------------------------------------------------------------------------------

void PerfectHitCorrectionAlgorithm::CaloHitCollection(const MCParticle *const pPfoTarget, PfoParameters &pfoParameters) const
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

void PerfectHitCorrectionAlgorithm::SimpleCaloHitCollection(const MCParticle *const pPfoTarget, const CaloHit *const pCaloHit, CaloHitList &caloHitList) const
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


    const MCParticle *const pHitMCParticle(pBestMCParticle);
    const MCParticle *const pHitPfoTarget(pHitMCParticle->GetPfoTarget());

    if (pHitPfoTarget != pPfoTarget)
        return;

    caloHitList.push_back(pCaloHit);
}

//------------------------------------------------------------------------------------------------------------------------------------------

void PerfectHitCorrectionAlgorithm::FullCaloHitCollection(const MCParticle *const pPfoTarget, const CaloHit *const pCaloHit, CaloHitList &caloHitList) const
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
StatusCode PerfectHitCorrectionAlgorithm::ReadSettings(const TiXmlHandle /* xmlHandle */)
{
    //PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, XmlHelper::ReadValue(xmlHandle,
    //    "OutputClusterListName", m_outputClusterListName));

    return STATUS_CODE_SUCCESS;
}

} // namespace lc_content
