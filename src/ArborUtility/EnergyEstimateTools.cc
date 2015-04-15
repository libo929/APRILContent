  /// \file EnergyEstimateTools.cc
/*
 *
 * EnergyEstimateTools.cc source template automatically generated by a class generator
 * Creation date : ven. avr. 10 2015
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

#include "Pandora/AlgorithmHeaders.h"

#include "ArborUtility/EnergyEstimateTools.h"

namespace arbor_content
{

pandora::StatusCode EnergyEstimateToolBase::ComputeEnergy(const pandora::Cluster *const pCluster, float &energyEstimate) const
{
	if(NULL == pCluster)
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	pandora::CaloHitList clusterCaloHitList;
	pCluster->GetOrderedCaloHitList().GetCaloHitList(clusterCaloHitList);

	return this->ComputeEnergy(clusterCaloHitList, energyEstimate);
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode EnergyEstimateToolBase::ComputeEnergySum(const pandora::Cluster *const pCluster1, const pandora::Cluster *const pCluster2,
		float &energySumEstimate) const
{
	if(NULL == pCluster1 || NULL == pCluster2)
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	pandora::CaloHitList clustersCaloHitList;
	pCluster1->GetOrderedCaloHitList().GetCaloHitList(clustersCaloHitList);
	pCluster2->GetOrderedCaloHitList().GetCaloHitList(clustersCaloHitList);

	return this->ComputeEnergy(clustersCaloHitList, energySumEstimate);
}

//------------------------------------------------------------------------------------------------------------------------------------------

float EnergyEstimateToolBase::GetTrackClusterCompatibility(const pandora::Cluster *const pCluster,
    const pandora::TrackList &trackList, float energyResolutionFactor) const
{
    float trackEnergySum(0.);

    for (pandora::TrackList::const_iterator trackIter = trackList.begin(), trackIterEnd = trackList.end(); trackIter != trackIterEnd; ++trackIter)
        trackEnergySum += (*trackIter)->GetEnergyAtDca();

    float clusterEnergy = 0.f;
    float energyResolution = 0.f;

    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->ComputeEnergy(pCluster, clusterEnergy));
    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetEnergyResolution(clusterEnergy, energyResolution));

    if ((trackEnergySum < std::numeric_limits<float>::epsilon())
     || (energyResolution < std::numeric_limits<float>::epsilon())
     || (clusterEnergy < std::numeric_limits<float>::epsilon()))
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    const float sigmaE(energyResolution * trackEnergySum / std::sqrt(trackEnergySum));
    const float chi((clusterEnergy - trackEnergySum) / (energyResolutionFactor * sigmaE));

    return chi;
}

//------------------------------------------------------------------------------------------------------------------------------------------

float EnergyEstimateToolBase::GetTrackClusterCompatibility(const float clusterEnergy, const float trackEnergy,
		float energyResolutionFactor) const
{
    float energyResolution = 0.f;
    PANDORA_THROW_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, this->GetEnergyResolution(clusterEnergy, energyResolution));

    if ((trackEnergy < std::numeric_limits<float>::epsilon())
     || (energyResolution < std::numeric_limits<float>::epsilon())
     || (clusterEnergy < std::numeric_limits<float>::epsilon()))
        throw pandora::StatusCodeException(pandora::STATUS_CODE_FAILURE);

    const float sigmaE(energyResolution * trackEnergy / std::sqrt(trackEnergy));
    const float chi((clusterEnergy - trackEnergy) / (energyResolutionFactor * sigmaE));

    return chi;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearInputEnergyEstimate::ComputeEnergy(const pandora::CaloHitList &caloHitList, float &energyEstimate) const
{
	energyEstimate = 0.f;

	for(pandora::CaloHitList::const_iterator iter = caloHitList.begin(), endIter = caloHitList.end() ;
			endIter != iter ; ++iter)
	{
		const pandora::CaloHit *const pCaloHit = *iter;
		energyEstimate += pCaloHit->GetInputEnergy();
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearInputEnergyEstimate::GetEnergyResolution(float energyPoint, float &energyResolution) const
{
	energyResolution = std::sqrt( m_resolutionEnergyFactor*m_resolutionEnergyFactor/energyPoint
					         	+ m_resolutionConstantFactor*m_resolutionConstantFactor
					         	+ m_resolutionEnergySquareFactor*m_resolutionEnergySquareFactor/(energyPoint*energyPoint));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearInputEnergyEstimate::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	m_resolutionEnergyFactor = 0.6;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergyFactor", m_resolutionEnergyFactor));

	m_resolutionConstantFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionConstantFactor", m_resolutionConstantFactor));

	m_resolutionEnergySquareFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergySquareFactor", m_resolutionEnergySquareFactor));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearEnergyEstimate::ComputeEnergy(const pandora::CaloHitList &caloHitList, float &energyEstimate) const
{
	energyEstimate = 0.f;

	for(pandora::CaloHitList::const_iterator iter = caloHitList.begin(), endIter = caloHitList.end() ;
			endIter != iter ; ++iter)
	{
		const pandora::CaloHit *const pCaloHit = *iter;
		energyEstimate += pCaloHit->GetElectromagneticEnergy() + pCaloHit->GetHadronicEnergy();
	}

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearEnergyEstimate::GetEnergyResolution(float energyPoint, float &energyResolution) const
{
	energyResolution = std::sqrt( m_resolutionEnergyFactor*m_resolutionEnergyFactor/energyPoint
					         	+ m_resolutionConstantFactor*m_resolutionConstantFactor
					         	+ m_resolutionEnergySquareFactor*m_resolutionEnergySquareFactor/(energyPoint*energyPoint));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode LinearEnergyEstimate::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	m_resolutionEnergyFactor = 0.6;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergyFactor", m_resolutionEnergyFactor));

	m_resolutionConstantFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionConstantFactor", m_resolutionConstantFactor));

	m_resolutionEnergySquareFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergySquareFactor", m_resolutionEnergySquareFactor));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode CombinedQuadraticEnergyEstimate::ComputeEnergy(const pandora::CaloHitList &caloHitList, float &energyEstimate) const
{
	energyEstimate = 0.f;

	float electromagneticEnergy = 0.f;
	float hadronicEnergy        = 0.f;
	unsigned int NHadronicHit  = 0;
	unsigned int NHadronicHit1 = 0;
	unsigned int NHadronicHit2 = 0;
	unsigned int NHadronicHit3 = 0;

	for(pandora::CaloHitList::const_iterator iter = caloHitList.begin(), endIter = caloHitList.end() ;
			endIter != iter ; ++iter)
	{
		const pandora::CaloHit *const pCaloHit = *iter;

		if(pandora::ECAL == pCaloHit->GetHitType())
		{
			electromagneticEnergy += pCaloHit->GetElectromagneticEnergy() + pCaloHit->GetHadronicEnergy();
			continue;
		}

		if(pandora::HCAL == pCaloHit->GetHitType())
		{
			if(m_energyThresholdValues.at(0) - pCaloHit->GetInputEnergy() < std::numeric_limits<float>::epsilon())
				NHadronicHit1++;
			else if(m_energyThresholdValues.at(1) - pCaloHit->GetInputEnergy() < std::numeric_limits<float>::epsilon())
				NHadronicHit2++;
			else if(m_energyThresholdValues.at(2) - pCaloHit->GetInputEnergy() < std::numeric_limits<float>::epsilon())
				NHadronicHit3++;
			else
				return pandora::STATUS_CODE_INVALID_PARAMETER;

			NHadronicHit++;
		}
	}

	const float alpha = m_energyConstantParameters.at(0) + m_energyConstantParameters.at(1)*NHadronicHit + m_energyConstantParameters.at(2)*NHadronicHit*NHadronicHit;
	const float beta  = m_energyConstantParameters.at(3) + m_energyConstantParameters.at(4)*NHadronicHit + m_energyConstantParameters.at(5)*NHadronicHit*NHadronicHit;
	const float gamma = m_energyConstantParameters.at(6) + m_energyConstantParameters.at(7)*NHadronicHit + m_energyConstantParameters.at(8)*NHadronicHit*NHadronicHit;

	hadronicEnergy = NHadronicHit1*alpha + NHadronicHit2*beta + NHadronicHit3*gamma;

	energyEstimate = electromagneticEnergy + hadronicEnergy;

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode CombinedQuadraticEnergyEstimate::GetEnergyResolution(float energyPoint, float &energyResolution) const
{
	energyResolution = std::sqrt( m_resolutionEnergyFactor*m_resolutionEnergyFactor/energyPoint
					         	+ m_resolutionConstantFactor*m_resolutionConstantFactor
					         	+ m_resolutionEnergySquareFactor*m_resolutionEnergySquareFactor/(energyPoint*energyPoint));

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode CombinedQuadraticEnergyEstimate::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
	pandora::StatusCode statusCodeValues = pandora::XmlHelper::ReadVectorOfValues(xmlHandle, "EnergyConstantParameters", m_energyConstantParameters);

	if(statusCodeValues != pandora::STATUS_CODE_SUCCESS)
	{
		if(statusCodeValues == pandora::STATUS_CODE_NOT_FOUND)
		{
			m_energyConstantParameters.push_back(0.0385315);
			m_energyConstantParameters.push_back(4.22584e-05);
			m_energyConstantParameters.push_back(-7.54657e-09);
			m_energyConstantParameters.push_back(0.0784297);
		    m_energyConstantParameters.push_back(-5.69439e-05);
		    m_energyConstantParameters.push_back(-4.95924e-08);
		    m_energyConstantParameters.push_back(0.127212);
		    m_energyConstantParameters.push_back(4.56414e-05);
		    m_energyConstantParameters.push_back(1.41142e-08);
		}
		else
			return statusCodeValues;
	}

	if(9 != m_energyConstantParameters.size())
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	statusCodeValues = pandora::XmlHelper::ReadVectorOfValues(xmlHandle, "EnergyThresholdValues", m_energyThresholdValues);

	if(statusCodeValues != pandora::STATUS_CODE_SUCCESS)
	{
		if(statusCodeValues == pandora::STATUS_CODE_NOT_FOUND)
		{
			m_energyThresholdValues.push_back(1);
			m_energyThresholdValues.push_back(2);
			m_energyThresholdValues.push_back(3);
		}
		else
			return statusCodeValues;
	}

	if(3 != m_energyThresholdValues.size())
		return pandora::STATUS_CODE_INVALID_PARAMETER;

	m_resolutionEnergyFactor = 0.6;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergyFactor", m_resolutionEnergyFactor));

	m_resolutionConstantFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionConstantFactor", m_resolutionConstantFactor));

	m_resolutionEnergySquareFactor = 0.f;
	PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
			"ResolutionEnergySquareFactor", m_resolutionEnergySquareFactor));

	return pandora::STATUS_CODE_SUCCESS;
}


} 

