/// \file ClustersMergingAlgorithm.cc
/*
 *
 * ClustersMergingAlgorithm.cc source template automatically generated by a class generator
 * Creation date : jeu. avr. 9 2015
 *
 * This file is part of APRILContent libraries.
 * 
 * APRILContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * APRILContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with APRILContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILTopologicalAssociation/ClustersMergingAlgorithm.h"

#include "APRILHelpers/SortingHelper.h"
#include "APRILHelpers/GeometryHelper.h"
#include "APRILHelpers/ReclusterHelper.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/HistogramHelper.h"
#include "APRILHelpers/ClusterPropertiesHelper.h"

#include "APRILApi/APRILContentApi.h"

#include "APRILUtility/EventPreparationAlgorithm.h"

#include "APRILTools/TrackDrivenSeedingTool.h"
#include "APRILObjects/CaloHit.h"

#include <algorithm>

#define __DEBUG__ 0
#define __DEBUG1__ 0
#define __DEBUG2__ 1

namespace april_content
{
  pandora::StatusCode ClustersMergingAlgorithm::Run()
  {
	std::vector<APRILCluster*> clusterVector;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetEligibleClusters(clusterVector));
	
	std::map<APRILCluster*, std::set<APRILCluster*>> mergingMap;

	for(int i = 0; i < clusterVector.size(); ++i)
	{
		auto& startingCluster = clusterVector.at(i);

		if(!startingCluster->IsRoot()) continue;
		if( m_onlyMergeCharged && startingCluster->GetAssociatedTrackList().size() == 0) continue;

		std::set<APRILCluster*> allClustersToMerge;
		startingCluster->GetAllClustersToMerge(allClustersToMerge);

#if __DEBUG__
		std::cout << "cluster: " << startingCluster << ", allClustersToMerge.size: " << allClustersToMerge.size() << std::endl;
#endif
		if(allClustersToMerge.empty()) continue;

		mergingMap[startingCluster] = allClustersToMerge;

#if __DEBUG__
		std::cout << " --- clu " << i << ": " << startingCluster << ", E: " << startingCluster->GetHadronicEnergy() 
			      << ", clusters to merge: " << allClustersToMerge.size() << std::endl;
#endif

#if 0
		for(auto cluToMerge : allClustersToMerge)
		{
			if(cluToMerge != nullptr)
			{
				std::cout << "  ---> merge E: " << cluToMerge->GetHadronicEnergy() << std::endl;
			}
		}
#endif
	}

#if __DEBUG__
	std::cout << "merging map size: " << mergingMap.size() << std::endl;
#endif

	std::string tupleNameIssue("ClustersMergingAlgorithmIssue_");
	std::string tupleNameError("ClustersMergingAlgorithmError_");
	std::string tupleNameGood("ClustersMergingAlgorithmGood_");
	tupleNameIssue += GetInstanceName();
	tupleNameError += GetInstanceName();
	tupleNameGood  += GetInstanceName();

#if __DEBUG__
	std::cout << "==========================================" << std::endl;
#endif

	for(auto& mergingMapIter :  mergingMap)
	{
		auto clusterToEnlarge = mergingMapIter.first;
		auto& clustersToMerge = mergingMapIter.second;

#if __DEBUG__		
                std::cout << std::endl;
#endif

		for(auto& clusterToMerge : clustersToMerge)
		{
			float oldChi = 1.e6;
			float newChi = 1.e6;
			float trackEnergy = 0.;
#if __DEBUG__	
			std::cout << " --- clusterToEnlarge:" << clusterToEnlarge << ", clusterToMerge: " << clusterToMerge << std::endl;
#endif

			bool mergeWithTrack = true;

			if(m_useEnergyChi &&
			   pandora::STATUS_CODE_SUCCESS != 
			        ClusterHelper::GetChiClusterMerging(this->GetPandora(), clusterToEnlarge, clusterToMerge, trackEnergy, oldChi, newChi))
			{
				//std::cout << "      ===> GetChiClusterMerging issue..." << std::endl;
				mergeWithTrack = false;
				//continue;
			}

			try
			{
#if 0
				if( pandora::PdgTable::GetParticleCharge(pClusterToMergeMCParticle->GetParticleId()) == 0. && 
					clusterToMerge->GetHadronicEnergy() > 0. )
				{
					continue;
				}
#endif

				float maxChi = -1.e6;
				
				if(trackEnergy < 2. )
				{
					maxChi = m_lowEnergyMaxChi;
				}
				else
				{
					maxChi = m_maxChi;
				}

				if( m_useEnergyChi && mergeWithTrack && newChi > maxChi ) 
				{
#if __DEBUG__	
					std::cout << "      ===> no merging, newChi: " << newChi << ", m_maxChi: " << m_maxChi << std::endl;
#endif
					continue;
				}

				if(clusterToMerge->GetHadronicEnergy() > m_maxMergingEnergy) 
				{
#if __DEBUG__	
					std::cout << "      ===> no merging, " << clusterToMerge->GetHadronicEnergy() << ", m_maxMergingEnergy: " << m_maxMergingEnergy << std::endl;
#endif
					continue;
				}

				if(m_useMCTruth)
				{
					const pandora::Cluster* const pandoraTrackStartClu = dynamic_cast<const pandora::Cluster* const>(clusterToEnlarge);
	                auto pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pandoraTrackStartClu);

	                const pandora::Cluster* const pandoraClusterToMerge = dynamic_cast<const pandora::Cluster* const>(clusterToMerge);
	                auto pClusterToMergeMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pandoraClusterToMerge);

	                std::vector<float> vars;
	                vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	                vars.push_back( clusterToEnlarge->GetHadronicEnergy() );
	                vars.push_back( clusterToMerge->GetHadronicEnergy() );
	                vars.push_back( float(clusterToMerge->GetNCaloHits()) );
	                vars.push_back( float(pClusterMCParticle == pClusterToMergeMCParticle) );
	                vars.push_back( float(pClusterMCParticle->GetParticleId()) );
				    vars.push_back( float(pandora::PdgTable::GetParticleCharge(pClusterMCParticle->GetParticleId())) );
	                vars.push_back( float(pClusterToMergeMCParticle->GetParticleId()) );
				    vars.push_back( float(pandora::PdgTable::GetParticleCharge(pClusterToMergeMCParticle->GetParticleId())) );
				    vars.push_back( oldChi );
				    vars.push_back( newChi );

					if(pClusterMCParticle != pClusterToMergeMCParticle)
			        {
				    	if(pandora::PdgTable::GetParticleCharge(pClusterMCParticle->GetParticleId()) != 0 &&
				    	   pandora::PdgTable::GetParticleCharge(pClusterToMergeMCParticle->GetParticleId()) != 0)
				    	{
				    		if(!m_mergeIssue) continue;
				    		
#if __DEBUG2__	
					        auto clusterRegion = ClusterHelper::GetRegion(clusterToMerge);

			        	    std::cout << "   === merging issue" << std::endl
							          << "       main cluster: " << clusterToEnlarge << ", E: " << clusterToEnlarge->GetHadronicEnergy() << std::endl
			        		          << "       merging cluster: " << clusterToMerge << ", E: " << clusterToMerge->GetHadronicEnergy() << std::endl
									  << "       MC charge: " << pandora::PdgTable::GetParticleCharge(pClusterToMergeMCParticle->GetParticleId()) << std::endl
									  << "       cluster region: " << clusterRegion << std::endl
				    		          << "       oldChi: " << oldChi << ", newChi: " << newChi << std::endl;
#endif
		            
				    		HistogramManager::CreateFill(tupleNameIssue.c_str(), 
				    			"evtNum:clusterEnergy:mergeEnergy:nCaloHits:isRight:pidMain:chgMain:pidMerge:chgMerge:oldChi:newChi", vars);
				    	}
				    	else
				    	{
				    		if(!m_mergeError) continue;
				    		
#if __DEBUG2__	
					        auto clusterRegion = ClusterHelper::GetRegion(clusterToMerge);
		  
							auto clusterStartingPoint = clusterToEnlarge->GetStartingPoint();
		                    auto nearbyClusterStartingPoint = clusterToMerge->GetStartingPoint();

			        	    std::cout << "   \033[1;31m === merging ERROR" << std::endl
							   	      << "       main cluster: " << clusterToEnlarge << ", E: " << clusterToEnlarge->GetHadronicEnergy() << ", layer: " << clusterToEnlarge->GetInnerPseudoLayer() << std::endl
			        		          << "       merging cluster: " << clusterToMerge << ", E: " << clusterToMerge->GetHadronicEnergy() << ", layer: " << clusterToMerge->GetInnerPseudoLayer() << std::endl

									  << "       starting point: " << clusterStartingPoint.GetX() << ", " << clusterStartingPoint.GetY() << ", " << clusterStartingPoint.GetZ() << std::endl
									  << "       nearby point: " << nearbyClusterStartingPoint.GetX() << ", " << nearbyClusterStartingPoint.GetY() << ", " << nearbyClusterStartingPoint.GetZ() << ", point dist: " << (clusterStartingPoint - nearbyClusterStartingPoint).GetMagnitude() << std::endl
									  << "       MC charge: " << pandora::PdgTable::GetParticleCharge(pClusterToMergeMCParticle->GetParticleId()) << std::endl
									  << "       cluster region: " << clusterRegion << std::endl
				    		          << "       oldChi: " << oldChi << ", newChi: " << newChi << "\033[0m" << std::endl;
#endif

				    		HistogramManager::CreateFill(tupleNameError.c_str(), 
				    			"evtNum:clusterEnergy:mergeEnergy:nCaloHits:isRight:pidMain:chgMain:pidMerge:chgMerge:oldChi:newChi", vars);
				    		
				    	}
			        }
				    else
				    {
				    		HistogramManager::CreateFill(tupleNameGood.c_str(), 
				    			"evtNum:clusterEnergy:mergeEnergy:nCaloHits:isRight:pidMain:chgMain:pidMerge:chgMerge:oldChi:newChi", vars);
				    }
				}
			}
			catch(pandora::StatusCodeException &)
			{
				std::cout << "- MCP getting problem..." << std::endl;
			}

#if __DEBUG1__	
		    const pandora::Cluster* const pandoraCluEnlarge = dynamic_cast<const pandora::Cluster* const>(clusterToEnlarge);
		    const pandora::Cluster* const pandoraCluMerge = dynamic_cast<const pandora::Cluster* const>(clusterToMerge);
		    bool isPhotonClusterToEnlarge = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(pandoraCluEnlarge);
		    bool isPhotonClusterToMerge = PandoraContentApi::GetPlugins(*this)->GetParticleId()->IsPhoton(pandoraCluMerge);

			std::cout << " @@@@@@ merge: " << clusterToEnlarge << ", E: " << clusterToEnlarge->GetHadronicEnergy() 
				<< ", isPhoton: " << isPhotonClusterToEnlarge
				<< " ---> " << clusterToMerge << ", E: " << clusterToMerge->GetHadronicEnergy()
				<< ", isPhoton: " << isPhotonClusterToMerge
				<< std::endl;
#endif
			APRILContentApi::MergeAndDeleteClusters(*this, clusterToEnlarge, clusterToMerge);
		}
	}

    return pandora::STATUS_CODE_SUCCESS;
  }

  pandora::StatusCode ClustersMergingAlgorithm::GetEligibleClusters(std::vector<APRILCluster*>& clusterVector) const
  {
	clusterVector.clear();

    const pandora::ClusterList *pClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	//std::cout << ">>>cluster number: " << pClusterList->size() << std::endl;

    if(pClusterList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    for(pandora::ClusterList::const_iterator clusterIter = pClusterList->begin(), clusterEndIter = pClusterList->end() ;
        clusterEndIter != clusterIter ; ++clusterIter)
    {
      const pandora::Cluster *const pCluster = *clusterIter;

      //if(!this->CanMergeCluster(pCluster))
      //  continue;

	  auto aprilCluster = APRILContentApi::Modifiable(dynamic_cast<const april_content::APRILCluster*>(pCluster));
      clusterVector.push_back(aprilCluster);
    }

    // sort them by inner layer
    std::sort(clusterVector.begin(), clusterVector.end(), SortingHelper::SortClustersByInnerLayer);

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ClustersMergingAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle )
  {
	m_maxChi = 1.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxChi", m_maxChi));

	m_lowEnergyMaxChi = 1.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "LowEnergyMaxChi", m_lowEnergyMaxChi));

	m_maxMergingEnergy = 100.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxMergingEnergy", m_maxMergingEnergy));

	m_useEnergyChi = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "UseEnergyChi", m_useEnergyChi));
	
	m_useMCTruth = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "UseMCTruth", m_useMCTruth));

	m_mergeIssue = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeIssue", m_mergeIssue));

	m_mergeError = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MergeError", m_mergeError));

	m_onlyMergeCharged = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "OnlyMergeCharged", m_onlyMergeCharged));

    return pandora::STATUS_CODE_SUCCESS;
  }


} 
