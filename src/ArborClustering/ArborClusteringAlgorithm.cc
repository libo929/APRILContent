/// \file ArborClusteringAlgorithm.cc
/*
 *
 * ArborClusteringAlgorithm.cc source template automatically generated by a class generator
 * Creation date : ven. d�c. 4 2015
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
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#include "ArborClustering/ArborClusteringAlgorithm.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborHelpers/CaloHitHelper.h"
#include "ArborTools/ConnectorAlgorithmTool.h"
#include "ArborTools/ConnectorSeedingTool.h"
#include "ArborHelpers/CaloHitRangeSearchHelper.h"
#include "ArborHelpers/CaloHitNeighborSearchHelper.h"

namespace arbor_content
{
    const pandora::CaloHitList*  ArborClusteringAlgorithm::m_pCaloHitList = nullptr; 
    pandora::CaloHitList   ArborClusteringAlgorithm::m_ecalCaloHitList; 
	pandora::CaloHitList   ArborClusteringAlgorithm::m_hcalCaloHitList; 
	pandora::CaloHitList   ArborClusteringAlgorithm::m_muonCaloHitList;

  pandora::StatusCode ArborClusteringAlgorithm::Run()
  {
	m_pCaloHitList = nullptr;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, m_pCaloHitList));

    if(m_pCaloHitList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->SplitCaloHitList(m_pCaloHitList, m_ecalCaloHitList, m_hcalCaloHitList, m_muonCaloHitList));

    clock_t t0, t1;

    t0 = clock();
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ConnectCaloHits(m_toolList));

    t1 = clock();


    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->CreateClusters());

    std::cout << "connnecting time (ms): " << 1000*(t1 - t0)/CLOCKS_PER_SEC << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ArborClusteringAlgorithm::SplitCaloHitList(const pandora::CaloHitList *const pCaloHitList, pandora::CaloHitList &ecalCaloHitList,
      pandora::CaloHitList &hcalCaloHitList, pandora::CaloHitList &muonCaloHitList) const
  {
	std::cout << "Calo hits: " << pCaloHitList->size() << std::endl;

	ecalCaloHitList.clear();
	hcalCaloHitList.clear();
	muonCaloHitList.clear();

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(), endIter = pCaloHitList->end() ;
        endIter !=iter ; ++iter)
    {
      const pandora::CaloHit *const pCaloHit= *iter;

      if(!PandoraContentApi::IsAvailable(*this, pCaloHit))
        continue;

      if(pCaloHit->GetHitType() == pandora::ECAL)
        m_ecalCaloHitList.push_back(pCaloHit);
      else if(pCaloHit->GetHitType() == pandora::HCAL)
        m_hcalCaloHitList.push_back(pCaloHit);
      else if(pCaloHit->GetHitType() == pandora::MUON)
        m_muonCaloHitList.push_back(pCaloHit);
    }

	std::cout << "spliting calo hits: " << std::endl;
	std::cout << "  ---> ecalCaloHitList: " << m_ecalCaloHitList.size() << std::endl;
	std::cout << "  ---> hcalCaloHitList: " << m_hcalCaloHitList.size() << std::endl;
	std::cout << "  ---> muonCaloHitList: " << m_muonCaloHitList.size() << std::endl;

	CaloHitRangeSearchHelper::BuildRangeSearch(pCaloHitList);
	CaloHitRangeSearchHelper::BuildHitCollectionOfLayers(pCaloHitList);
	CaloHitRangeSearchHelper::BuildHitCollectionOfEcalLayers(&m_ecalCaloHitList);
	CaloHitRangeSearchHelper::BuildHitCollectionOfHcalLayers(&m_hcalCaloHitList);
	CaloHitRangeSearchHelper::BuildHitCollectionOfMuonLayers(&m_muonCaloHitList);

	//
	CaloHitNeighborSearchHelper::BuildNeighborSearch(pCaloHitList);

	std::cout << "ordered hit layer size: " << std::endl;
	std::cout << " total: " << CaloHitRangeSearchHelper::GetOrderedCaloHitList()->size() << std::endl;
	std::cout << " ecal: " << CaloHitRangeSearchHelper::GetOrderedEcalCaloHitList()->size() << std::endl;
	std::cout << " hcal: " << CaloHitRangeSearchHelper::GetOrderedHcalCaloHitList()->size() << std::endl;
	std::cout << " muon: " << CaloHitRangeSearchHelper::GetOrderedMuonCaloHitList()->size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ArborClusteringAlgorithm::ConnectCaloHits(const ConnectorAlgorithmToolVector &toolVector) const
  {
	  std::cout << "Connect tools size: " << toolVector.size() << std::endl;
#if 1
    for(ConnectorAlgorithmToolVector::const_iterator iter = toolVector.begin(), endIter = toolVector.end() ;
        endIter != iter ; ++iter)
    {
      ConnectorAlgorithmTool *pTool = *iter;
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, pTool->Process(*this));
    }
#endif

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ArborClusteringAlgorithm::CreateClusters() const
  {
    if( m_toolList.empty() ) return pandora::STATUS_CODE_SUCCESS;

    // Find seeds and build clusters from them looking for forward connected hits in the tree structure
    pandora::CaloHitList seedCaloHitList;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::ExtractCurrentSeedCaloHitList(*this, seedCaloHitList, !m_allowSingleHitClusters));
      
    std::string listName;
    const pandora::ClusterList* clusterList = NULL;
    
    if(m_useAsIndependent)
    {
        PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, clusterList, listName);
    }

    for(pandora::CaloHitList::iterator iter = seedCaloHitList.begin(), endIter = seedCaloHitList.end() ;
        endIter != iter ; ++iter)
    {
      const arbor_content::CaloHit *const pCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(*iter);

      // wrong framework baby !
      if(NULL == pCaloHit)
        continue;

      if( ! PandoraContentApi::IsAvailable<pandora::CaloHit>(*this, pCaloHit) )
        continue;

      pandora::CaloHitList clusterCaloHitList;
      clusterCaloHitList.push_back(pCaloHit);

      // get the whole tree calo hit list
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, CaloHitHelper::BuildCaloHitList(pCaloHit, FORWARD_DIRECTION, clusterCaloHitList));

      // create a cluster with this list
      const pandora::Cluster *pCluster = NULL;
      //PandoraContentApi::ClusterParameters clusterParameters;
	  object_creation::ClusterParameters clusterParameters;
      clusterParameters.m_caloHitList = clusterCaloHitList;

      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Create(*this, clusterParameters, pCluster));

#if 0
	  std::cout << "check cluster ptr: "  << pCluster << std::endl;
      pandora::CaloHitList checkCaloHitList;
      pCluster->GetOrderedCaloHitList().FillCaloHitList(checkCaloHitList);

	  for(auto& caloHit : clusterCaloHitList)
	  {
        const arbor_content::CaloHit *const pArborCaloHit = dynamic_cast<const arbor_content::CaloHit *const>(caloHit);
		  std::cout << " calo hit mother cluser: " << pArborCaloHit->GetMother() << ", cluster energy: "  
			  << pArborCaloHit->GetMother()->GetHadronicEnergy() << std::endl;
	  }
#endif
    }
      
    if(m_useAsIndependent)
    {
        //std::cout << "new tmp list name: " << listName << std::endl;
          
        std::string newClusterName("ArborClusteringAlgClusters");
        std::stringstream ss;
        ss << this;
        newClusterName = newClusterName + "_" + ss.str();
          
        // save the current list to the list with new name
        PandoraContentApi::SaveList<pandora::Cluster>(*this, newClusterName);
          
        // 
        PandoraContentApi::ReplaceCurrentList<pandora::Cluster>(*this, newClusterName);
          
    }

    const pandora::ClusterList *pCurrentClusterList = NULL;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCurrentClusterList));
    std::cout << "  ---> Current cluster list size: " << pCurrentClusterList->size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ArborClusteringAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
    pandora::AlgorithmToolVector algorithmToolList;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ProcessAlgorithmToolList(*this, xmlHandle,
        "ConnectionTools", algorithmToolList));

    for(pandora::AlgorithmToolVector::const_iterator iter = algorithmToolList.begin(), endIter = algorithmToolList.end() ;
        endIter != iter ; ++iter)
    {
      ConnectorAlgorithmTool *pTool = dynamic_cast<ConnectorAlgorithmTool*>(*iter);

      if(NULL == pTool)
        return pandora::STATUS_CODE_INVALID_PARAMETER;

      m_toolList.push_back(pTool);
    }

    m_useAsIndependent = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "UseAsIndependent", m_useAsIndependent));
      

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

