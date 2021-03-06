/// \file ConnectorSeedingTool.cc
/*
 *
 * ConnectorSeedingTool.cc source template automatically generated by a class generator
 * Creation date : mar. d�c. 8 2015
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


#include "APRILTools/ConnectorSeedingTool.h"

#include "Pandora/AlgorithmHeaders.h"

#include "APRILObjects/CaloHit.h"
#include "APRILObjects/Connector.h"
#include "APRILHelpers/CaloHitHelper.h"
#include "APRILHelpers/CaloHitRangeSearchHelper.h"

namespace april_content
{
  pandora::StatusCode ConnectorSeedingTool::Process(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const /* pCaloHitList */)
  {
	//std::cout << " ------ ConnectorSeedingTool::Process " << std::endl;
    // ordered calo hit list of ECAL
	pandora::OrderedCaloHitList* pOrderedCaloHitList;

	switch(m_hitCollectionToUse)
	{
		case 1: 
			pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedEcalCaloHitList();
			break;

		case 2: 
			pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedHcalCaloHitList();
			break;

		case 3: 
			pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedMuonCaloHitList();
			break;

		default:
			pOrderedCaloHitList = CaloHitRangeSearchHelper::GetOrderedCaloHitList();
	}

	pandora::OrderedCaloHitList& orderedCaloHitList = *pOrderedCaloHitList;

	//std::cout << "orderedCaloHitList size: " << orderedCaloHitList.size() << std::endl;

    for(pandora::OrderedCaloHitList::const_iterator layerIter = orderedCaloHitList.begin(), layerEndIter = orderedCaloHitList.end() ;
        layerEndIter != layerIter ; ++layerIter)
    {
      for(pandora::CaloHitList::const_iterator iterI = layerIter->second->begin(), endIterI = layerIter->second->end() ;
          endIterI != iterI ; ++iterI)
      {
        const april_content::CaloHit *const pCaloHitI = dynamic_cast<const april_content::CaloHit *const>(*iterI);

        if(NULL == pCaloHitI)
          continue;

        // check for availability
        if(m_connectOnlyAvailable && !PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pCaloHitI))
          continue;

        if(!m_shouldUseIsolatedHits && pCaloHitI->IsIsolated())
          continue;

        if(m_shouldDiscriminateConnectedHits && !APRILContentApi::GetConnectorList(pCaloHitI, FORWARD_DIRECTION).empty())
          continue;

        const unsigned int pseudoLayerI = pCaloHitI->GetPseudoLayer();
        const pandora::CartesianVector &positionVectorI(pCaloHitI->GetPositionVector());

        for(unsigned int pl = pseudoLayerI+1 ; pl <= pseudoLayerI + m_maxPseudoLayerConnection ; pl++)
        {
          pandora::OrderedCaloHitList::const_iterator findIter = orderedCaloHitList.find(pl);

          if(orderedCaloHitList.end() == findIter)
            continue;

		  const pandora::CartesianVector &position(pCaloHitI->GetPositionVector());

          const float range = m_hitSearchRange; 

          int pseudoLayer = pl;
          pandora::CaloHitList hitsInRange;

          CaloHitRangeSearchHelper::SearchHitsInLayer(position, pseudoLayer, range, hitsInRange);

          // the pseudo layer of LHCAL seems having a problem: in some cases, two nearby readout layer may the same pseudo layer
          // Here is a ad hoc way to sovle it
          if(pCaloHitI->GetHitType() == pandora::HCAL && pCaloHitI->GetHitRegion() == pandora::ENDCAP 
             && (pseudoLayer - pseudoLayerI) == 1 )
          {
              pandora::CaloHitList addtionalHitsInRange;
              CaloHitRangeSearchHelper::SearchHitsInLayer(position, pseudoLayer-1, range, addtionalHitsInRange);

              hitsInRange.insert(hitsInRange.begin(), addtionalHitsInRange.begin(), addtionalHitsInRange.end());
          }

          for(pandora::CaloHitList::const_iterator iterJ = hitsInRange.begin(), endIterJ = hitsInRange.end() ;
              endIterJ != iterJ ; ++iterJ)
          {
            const april_content::CaloHit *const pCaloHitJ = dynamic_cast<const april_content::CaloHit *const>(*iterJ);

            if(NULL == pCaloHitJ)
              continue;

            if(pCaloHitJ->GetLayer() == pCaloHitI->GetLayer() ) continue;

            const pandora::CartesianVector &positionVectorJ(pCaloHitJ->GetPositionVector());

            // check if already connected
            if(APRILContentApi::IsConnected(pCaloHitI, pCaloHitJ, FORWARD_DIRECTION))
              continue;

            // check for availability
            if(m_connectOnlyAvailable && !PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pCaloHitJ))
              continue;

            if(m_shouldDiscriminateConnectedHits && !APRILContentApi::GetConnectorList(pCaloHitJ, BACKWARD_DIRECTION).empty())
              continue;

            const pandora::HitType hitTypeJ(pCaloHitJ->GetHitType());
            const float difference = (positionVectorJ - positionVectorI).GetMagnitude();
	  
            if(difference > m_maxCollectorLength) continue;

            const float angle = (positionVectorJ - positionVectorI).GetOpeningAngle(positionVectorI);

            const float transverseDistance = std::sin( angle ) * difference;
            const pandora::Granularity &granularity(PandoraContentApi::GetGeometry(algorithm)->GetHitTypeGranularity(hitTypeJ));

            const float maxTransverseDistance = granularity <= pandora::FINE ?
                m_maxTransverseDistanceFine : m_maxTransverseDistanceCoarse;

            const float maxConnectionAngle = granularity <= pandora::FINE ?
                m_maxConnectionAngleFine : m_maxConnectionAngleCoarse;

            // check transverse distance
            if(transverseDistance > maxTransverseDistance)
              continue;

            // check angle
            if(angle > maxConnectionAngle)
              continue;

            // connect !
            unsigned int creationStage = m_connectorCreationStage;
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, APRILContentApi::Connect(pCaloHitI, pCaloHitJ, FORWARD_DIRECTION, 1., creationStage));
          }
        }
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ConnectorSeedingTool::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
	m_hitSearchRange = 80.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "HitSearchRange", m_hitSearchRange));

	m_maxCollectorLength = 300.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxCollectorLength", m_maxCollectorLength));

    m_hitCollectionToUse = 0;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "CaloHitCollection", m_hitCollectionToUse));

    m_maxPseudoLayerConnection = 4;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxPseudoLayerConnection", m_maxPseudoLayerConnection));

    m_connectOnlyAvailable = true;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ConnectOnlyAvailable", m_connectOnlyAvailable));

    m_maxConnectionAngleFine = 0.6;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxConnectionAngleFine", m_maxConnectionAngleFine));

    m_maxConnectionAngleCoarse = 0.9;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxConnectionAngleCoarse", m_maxConnectionAngleCoarse));

    m_maxTransverseDistanceFine = 20.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTransverseDistanceFine", m_maxTransverseDistanceFine));

    m_maxTransverseDistanceCoarse = 65.f;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxTransverseDistanceCoarse", m_maxTransverseDistanceCoarse));

    m_shouldUseIsolatedHits = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldUseIsolatedHits", m_shouldUseIsolatedHits));

    m_shouldDiscriminateConnectedHits = false;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ShouldDiscriminateConnectedHits", m_shouldDiscriminateConnectedHits));

	m_connectorCreationStage = -1;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ConnectorCreationStage", m_connectorCreationStage));

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

