/**
 *  @file   CheatingClusterMergingNewAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster cleaning algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"
#include "PandoraMonitoring.h"

#include "APRILCheating/CheatingClusterMergingNewAlgorithm.h"
#include "APRILApi/APRILContentApi.h"
#include "APRILUtility/EventPreparationAlgorithm.h"
#include "APRILHelpers/HistogramHelper.h"
#include "APRILHelpers/ClusterHelper.h"

//#include "APRILHelpers/ClusterHelper.h"
//#include "APRILUtility/EventPreparationAlgorithm.h"
//#include "APRILHelpers/HistogramHelper.h"

namespace april_content
{

pandora::StatusCode CheatingClusterMergingNewAlgorithm::Run()
{
	MergeClusters();
	
    return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode CheatingClusterMergingNewAlgorithm::MergeClusters()
{
	////////////////////////////////////////////////////////////////////////////////
	// check the cluster and its mcp:
	// if one mcp has several clusters, then they should be merged into one cluster
	// it is especially mandatory for charged clusters
	////////////////////////////////////////////////////////////////////////////////
	
    const pandora::ClusterList* pClusterList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "cluster befor merging: " << pClusterList->size() << std::endl;

	std::string tupleName("CheatingClusterMerging_");
	tupleName += GetInstanceName();

	std::map<const pandora::MCParticle* const, pandora::ClusterList> mcpClusterListMap;
	
	for(auto it = pClusterList->begin(); it != pClusterList->end(); ++it)
	{
		const pandora::Cluster* const clu = *it;

		const pandora::MCParticle* pClusterMCParticle  = nullptr;

        try
        {
        	 pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle( clu );
        }
        catch (pandora::StatusCodeException &)
        {
		    continue;
		}

		if(pClusterMCParticle != nullptr && mcpClusterListMap.find( pClusterMCParticle ) == mcpClusterListMap.end())
		{
		    pandora::ClusterList clusterList;
		    clusterList.push_back( clu );
		    mcpClusterListMap[pClusterMCParticle] = clusterList;
		}
		else
		{
		    mcpClusterListMap[pClusterMCParticle].push_back( clu );
		}

		///////////////////////////////////////////////////////////////////////////////////////////////////
#if 0
	    std::vector<float> vars;
	    vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	    vars.push_back( float(clu->GetNCaloHits()) );
	    vars.push_back( float(clu->GetElectromagneticEnergy()) );
	    vars.push_back( float(clu->GetHadronicEnergy()) );

		HistogramManager::CreateFill("ClusterBeforeMerging", 
		"evtNumber:nCaloHit:clusterEMEnergy:clusterHadEnergy", vars);
#endif
	}

	for(auto it = mcpClusterListMap.begin(); it != mcpClusterListMap.end(); ++it)
	{
		// merge charged clusters
		auto mcp = it->first;
		int clusterMCPCharge = pandora::PdgTable::GetParticleCharge(mcp->GetParticleId());
		bool isPhoton = mcp->GetParticleId() == 22; 

		bool canMerge = (m_mergePhoton && isPhoton) || (m_mergeCharged && clusterMCPCharge != 0) || 
		                (m_mergeNeutral && clusterMCPCharge == 0) || ( m_mergeNeutralHadron && (clusterMCPCharge == 0 && isPhoton == false) );

	    if(!canMerge) continue;

		auto clusterList = it->second;

		pandora::ClusterVector clusterVector;
		clusterVector.insert(clusterVector.begin(), clusterList.begin(), clusterList.end());
	
		std::sort(clusterVector.begin(), clusterVector.end(), pandora_monitoring::PandoraMonitoring::SortClustersByHadronicEnergy);

	    if(clusterVector.size()>1) 
		{
		    //auto mcp = it->first;
			//std::cout << "------>>> particle cluster # greater than 1: " << clusterList.size() << ", PDG: "
			//	      << mcp->GetParticleId() << std::endl;

			// merge clusters
			auto firstCluster = clusterVector.at(0);

			//std::cout << "--- firstCluster energy: " << firstCluster->GetHadronicEnergy() << std::endl;

			//for(auto cluIt = clusterList.begin(); cluIt != clusterList.end(); ++cluIt)
			for(int i = 1; i < clusterVector.size(); ++i)
			{
				//if(cluIt == clusterList.begin()) continue;

		        const pandora::Cluster* cluToMerge = clusterVector.at(i);

                if(firstCluster->GetAssociatedTrackList().size() > 0 && cluToMerge->GetAssociatedTrackList().size() > 0) continue;
				if(cluToMerge->GetHadronicEnergy() < m_minClusterEnergyToMerge) continue;

				///
		        float closestDistance = -1.e6;

		        try
		        {
		            ClusterHelper::GetClosestDistanceApproach(firstCluster, cluToMerge, closestDistance, false);
		        }
                catch(pandora::StatusCodeException &)
		        {
		            std::cout << "GetClosestDistanceApproach failed" << std::endl;
		        }

				bool firstClusterHasTrack = !(firstCluster->GetAssociatedTrackList().empty());
				bool cluToMergeHasTrack = !(cluToMerge->GetAssociatedTrackList().empty());

				// test
				bool useStrictCut = true;

				if(useStrictCut)
				{
					if(closestDistance>300.) continue;
				    if(!firstClusterHasTrack && !cluToMergeHasTrack) continue;
				    if(firstCluster->GetHadronicEnergy() > 5 && cluToMerge->GetHadronicEnergy() > 5) continue;
				}

	            std::vector<float> vars;
	            vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	            vars.push_back( float(firstCluster->GetHadronicEnergy()) );
	            vars.push_back( float(cluToMerge->GetHadronicEnergy()) );
				vars.push_back( float(clusterMCPCharge));
				vars.push_back( float(isPhoton));
				vars.push_back( closestDistance );
				vars.push_back( float(firstClusterHasTrack) );
				vars.push_back( float(cluToMergeHasTrack) );

				//std::cout << "  -> cluster energy to merge: " << cluToMerge->GetHadronicEnergy() << std::endl;

		        HistogramManager::CreateFill(tupleName.c_str(), "evtNumber:mainClusterEnergy:clusterEnergy:clusterMCPCharge:isMCPPhoton:closestDistance:firstClusterHasTrack:cluToMergeHasTrack", vars);

				auto pAPRILFirstCluster = APRILContentApi::Modifiable(dynamic_cast<const april_content::APRILCluster*>(firstCluster));
				auto pAPRILCluToMerge = APRILContentApi::Modifiable(dynamic_cast<const april_content::APRILCluster*>(cluToMerge));

				float clusterAngle1 = ClusterHelper::GetClusterAxisStartingPointAngle(pAPRILFirstCluster);
				float clusterAngle2 = ClusterHelper::GetClusterAxisStartingPointAngle(pAPRILCluToMerge);

				float cluster1Time = ClusterHelper::GetAverageTime(pAPRILFirstCluster);
				float cluster2Time = ClusterHelper::GetAverageTime(pAPRILCluToMerge);

				std::cout << "     === Cluster to merge: " << std::endl
					      << "         mainCluster " << firstCluster << ", Ehad: " << firstCluster->GetHadronicEnergy() << ", region: " << ClusterHelper::GetRegion(firstCluster) << ", angle: " << clusterAngle1 << ", time: " << cluster1Time << std::endl
					      << "         cluToMerge " << cluToMerge    << ", Ehad: " << cluToMerge->GetHadronicEnergy() << ", region: " << ClusterHelper::GetRegion(cluToMerge) << ", angle: " << clusterAngle2 << ", time: " << cluster2Time << std::endl
					      << "         cluster charge: " << clusterMCPCharge << std::endl;

				if(clusterMCPCharge != 0)
				{
					std::cout << "     \033[1;31m=== merge two CHARGED clusters: " << firstCluster << ", E: " << firstCluster->GetHadronicEnergy() 
						      << " --- " << cluToMerge << ", E: " << cluToMerge->GetHadronicEnergy() 
							  << ", distance: " << closestDistance << "\033[0m" << std::endl;
				}

				APRILContentApi::MergeAndDeleteClusters(*this, firstCluster, cluToMerge);
			}

		}// end if
	} // end for

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));
	std::cout << "cluster after merging: " << pClusterList->size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode CheatingClusterMergingNewAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	m_mergePhoton        = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergePhoton", m_mergePhoton));

   	m_mergeCharged       = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeCharged", m_mergeCharged));

    m_mergeNeutral       = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeNeutral", m_mergeNeutral));

	m_mergeNeutralHadron = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeNeutralHadron", m_mergeNeutralHadron));

	m_minClusterEnergyToMerge = 0.0;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MinClusterEnergyToMerge", m_minClusterEnergyToMerge));

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace april_content
