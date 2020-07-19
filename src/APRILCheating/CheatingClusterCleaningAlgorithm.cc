/**
 *  @file   CheatingClusterCleaningAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster cleaning algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILCheating/CheatingClusterCleaningAlgorithm.h"
#include "APRILObjects/CaloHit.h"
#include "APRILHelpers/HistogramHelper.h"
#include "APRILHelpers/ClusterHelper.h"

using namespace pandora;

namespace april_content
{

StatusCode CheatingClusterCleaningAlgorithm::Run()
{
    const ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	//std::cout << " cluster size: " << pClusterList->size() << std::endl;

    // Examine clusters, reducing each to just the hits corresponding to the main mc particle
    for (ClusterList::const_iterator itCluster = pClusterList->begin(), itClusterEnd = pClusterList->end(); itCluster != itClusterEnd; ++itCluster )
    {
        try
        {
            const pandora::Cluster *const pCluster = *itCluster;
            const MCParticle *const pMainMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
			int clusterPDG = pMainMCParticle->GetParticleId();

			if(m_isOnlyCleanPhoton && clusterPDG != 22) continue;

            // Remove all calo hits that do not correspond to the cluster main mc particle
			pandora::OrderedCaloHitList orderedCaloHitList(pCluster->GetOrderedCaloHitList());

			pandora::CaloHitList savedHits;

            for (pandora::OrderedCaloHitList::const_iterator itLyr = orderedCaloHitList.begin(), itLyrEnd = orderedCaloHitList.end(); itLyr != itLyrEnd; ++itLyr)
            {
                for (pandora::CaloHitList::const_iterator hitIter = itLyr->second->begin(), hitIterEnd = itLyr->second->end(); hitIter != hitIterEnd; ++hitIter)
                {
                    try
                    {
                        const pandora::CaloHit *const pCaloHit = *hitIter;
                        const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));

                        if (pMainMCParticle != pMCParticle)
                        {
							//std::cout << " --- remove hit " << pCaloHit << " from cluster: " << pCluster << std::endl;
                            PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, APRILContentApi::RemoveFromCluster(*this, pCluster, pCaloHit));
        
							const april_content::CaloHit *const pAPRILCaloHit = dynamic_cast<const april_content::CaloHit *const>( pCaloHit );

                            if(NULL == pAPRILCaloHit) continue;
				
							float centroidDistance = 0.;
							float closestDistance = 0.;

							const pandora::CartesianVector hitPoint = pCaloHit->GetPositionVector();
				            ClusterHelper::GetCentroidDistance(pCluster, hitPoint, centroidDistance);
				            ClusterHelper::GetClosestDistanceApproach(pCluster, hitPoint, closestDistance);

							//pAPRILCaloHit->GetDensity();
							float fwdConnection = 0.;
							float bakConnection = 0.;

							const ConnectorList &fwdConnectorList(APRILContentApi::GetConnectorList(pAPRILCaloHit, FORWARD_DIRECTION));
							const ConnectorList &bakConnectorList(APRILContentApi::GetConnectorList(pAPRILCaloHit, BACKWARD_DIRECTION));

							for(auto fwdCon : fwdConnectorList)
							{
								float fwdConnectionLength = fwdCon->GetLength();

								if( fwdConnectionLength > fwdConnection ) fwdConnection = fwdConnectionLength;
							}

							for(auto bakCon : bakConnectorList)
							{
								float bakConnectionLength = bakCon->GetLength();

								if( bakConnectionLength > bakConnection ) bakConnection = bakConnectionLength;
							}

							float clusterEnergy = pCluster->GetHadronicEnergy();
							float hitEnergy = pAPRILCaloHit->GetInputEnergy();
			   
							float hitCharge = pandora::PdgTable::GetParticleCharge( pMCParticle->GetParticleId() );
							float cluCharge = pandora::PdgTable::GetParticleCharge( pMainMCParticle->GetParticleId() );

							std::vector<float> vars;
							vars.push_back( clusterEnergy );
							vars.push_back( hitEnergy );
							vars.push_back( fwdConnection );
							vars.push_back( bakConnection );
							vars.push_back( hitCharge );
							vars.push_back( cluCharge );
							vars.push_back( centroidDistance );
							vars.push_back( closestDistance );

							// FIXME
							if(closestDistance>200.) 
							{
								savedHits.push_back(pCaloHit);
								continue;
							}

							HistogramManager::CreateFill("CheatingClusterCleaning", 
									"clusterEnergy:hitEnergy:fwdConnection:bakConnection:hitCharge:cluCharge:centroidDistance:closestDistance", vars);
                        }
                    }
                    catch (StatusCodeException &)
                    {
                    }
                }
            }
                                
			PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, APRILContentApi::AddToCluster(*this, pCluster, &savedHits));

#if 0
            // Repeat for isolated hits
            CaloHitList isolatedCaloHitList(pCluster->GetIsolatedCaloHitList());

            for (CaloHitList::const_iterator hitIter = isolatedCaloHitList.begin(), hitIterEnd = isolatedCaloHitList.end(); hitIter != hitIterEnd; ++hitIter)
            {
                try
                {
                    const CaloHit *const pCaloHit = *hitIter;
                    const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCaloHit));

                    if (pMainMCParticle != pMCParticle)
                    {
				        std::cout << "------ cleaning isohits: " << std::endl;
                        PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::RemoveIsolatedFromCluster(*this, pCluster, pCaloHit));
                    }
                }
                catch (StatusCodeException &)
                {
                }
            }
#endif
        }
        catch (StatusCodeException &)
        {
        }
    }

    return STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

StatusCode CheatingClusterCleaningAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
{
    m_isOnlyCleanPhoton= false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "OnlyCleanPhoton", m_isOnlyCleanPhoton));

    return STATUS_CODE_SUCCESS;
}

} // namespace april_content
