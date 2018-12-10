/// \file ConnectorSeedingTool.cc
/*
 *
 * ConnectorSeedingTool.cc source template automatically generated by a class generator
 * Creation date : mar. d�c. 8 2015
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


#include "ArborTools/ConnectorSeedingTool.h"

#include "Pandora/AlgorithmHeaders.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"
#include "ArborHelpers/CaloHitHelper.h"

namespace arbor_content
{

  pandora::StatusCode ConnectorSeedingTool::Process(const pandora::Algorithm &algorithm, const pandora::CaloHitList *const pCaloHitList)
  {
    if(pCaloHitList->empty())
      return pandora::STATUS_CODE_SUCCESS;

    pandora::OrderedCaloHitList orderedCaloHitList;
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, orderedCaloHitList.Add(*pCaloHitList));

    for(pandora::OrderedCaloHitList::const_iterator layerIter = orderedCaloHitList.begin(), layerEndIter = orderedCaloHitList.end() ;
        layerEndIter != layerIter ; ++layerIter)
    {
      for(pandora::CaloHitList::const_iterator iterI = layerIter->second->begin(), endIterI = layerIter->second->end() ;
          endIterI != iterI ; ++iterI)
      {
        const arbor_content::CaloHit *const pCaloHitI = dynamic_cast<const arbor_content::CaloHit *const>(*iterI);

        if(NULL == pCaloHitI)
          continue;

        // check for availability
        if(m_connectOnlyAvailable && !PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pCaloHitI))
          continue;

        if(!m_shouldUseIsolatedHits && pCaloHitI->IsIsolated())
          continue;

        if(m_shouldDiscriminateConnectedHits && !ArborContentApi::GetConnectorList(pCaloHitI, FORWARD_DIRECTION).empty())
          continue;

        const unsigned int pseudoLayerI = pCaloHitI->GetPseudoLayer();
        const pandora::CartesianVector &positionVectorI(pCaloHitI->GetPositionVector());

	// TODO
	// k-d tree

        for(unsigned int pl = pseudoLayerI+1 ; pl <= pseudoLayerI + m_maxPseudoLayerConnection ; pl++)
        {
          pandora::OrderedCaloHitList::const_iterator findIter = orderedCaloHitList.find(pl);

          if(orderedCaloHitList.end() == findIter)
            continue;

          for(pandora::CaloHitList::const_iterator iterJ = findIter->second->begin(), endIterJ = findIter->second->end() ;
              endIterJ != iterJ ; ++iterJ)
          {
            const arbor_content::CaloHit *const pCaloHitJ = dynamic_cast<const arbor_content::CaloHit *const>(*iterJ);

            if(NULL == pCaloHitJ)
              continue;

            // check for availability
            if(m_connectOnlyAvailable && !PandoraContentApi::IsAvailable<pandora::CaloHit>(algorithm, pCaloHitJ))
              continue;

            const pandora::CartesianVector &positionVectorJ(pCaloHitJ->GetPositionVector());
            const pandora::HitType hitTypeJ(pCaloHitJ->GetHitType());
            const float difference = (positionVectorJ - positionVectorI).GetMagnitude();
            const float angle = (positionVectorJ - positionVectorI).GetOpeningAngle(positionVectorI);

	    // FIXME
	    // is this transverse ? 
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

            // check if already connected
            if(ArborContentApi::IsConnected(pCaloHitI, pCaloHitJ, FORWARD_DIRECTION))
              continue;

            // connect !
            PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ArborContentApi::Connect(pCaloHitI, pCaloHitJ, FORWARD_DIRECTION));
          }
        }
      }
    }

    return pandora::STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode ConnectorSeedingTool::ReadSettings(const pandora::TiXmlHandle xmlHandle)
  {
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

    return pandora::STATUS_CODE_SUCCESS;
  }

} 

