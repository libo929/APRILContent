/*
 *
 * Connector.h header template automatically generated by a class generator
 * Creation date : ven. mars 20 2015
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


#ifndef CONNECTOR_H
#define CONNECTOR_H

#include "Pandora/StatusCodes.h"
#include "Objects/CartesianVector.h"

#include "ArborApi/ArborInputTypes.h"
#include "ArborApi/ArborContentApi.h"

namespace arbor_content
{

/** 
 *  @brief  Connector class
 */ 
class Connector
{
public:
	/**
	 *  @brief  Get the 'from' calo hit
	 */
	const arbor_content::CaloHit *GetFrom() const;

	/**
	 *  @brief  Get the 'to' calo hit
	 */
	const arbor_content::CaloHit *GetTo() const;

	/**
	 *  @brief  Get the 'to' or 'from' calo hit (from (to) for BACKWARD_DIRECTION (FORWARD_DIRECTION))
	 */
	const arbor_content::CaloHit *Get(ConnectorDirection direction) const;

	/**
	 *  @brief  Get the connector length
	 */
	float GetLength() const;

	/**
	 *  @brief  Get the reference length
	 */
	float GetReferenceLength() const;

	/**
	 *  @brief  Get the normalized length
	 */
	float GetNormalizedLength() const;

	/**
	 *  @brief  Get the connector vector depending on the asked direction
	 */
	pandora::CartesianVector GetVector(ConnectorDirection direction = FORWARD_DIRECTION) const;

	/**
	 *  @brief  Whether the calo hit is the 'from' calo hit of this connector
	 */
	bool IsFrom(const arbor_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Whether the calo hit is the 'to' calo hit of this connector
	 */
	bool IsTo(const arbor_content::CaloHit *const pCaloHit) const;

	/**
	 *  @brief  Get the opening angle with an other connector (radian)
	 */
	float GetOpeningAngle(const Connector *const pConnector) const;

	/**
	 *  @brief  Whether the connector is available to be added in connector lists
	 */
	bool IsAvailable() const;

private:
	/**
	 *  @brief  Constructor with two calo hits and a reference length
	 */
	Connector(const arbor_content::CaloHit *const pFromCaloHit, const arbor_content::CaloHit *const pToCaloHit,
			float referenceLength = 1.f);

	/**
	 *  @brief  Destructor
	 */
	~Connector();

	/**
	 *  @brief  Set the connector availability
	 */
	void SetAvailability(bool availability);

	const CaloHit                        *m_pFromCaloHit;
	const CaloHit                        *m_pToCaloHit;
	float                                 m_referenceLength;
	bool                                  m_isAvailable;

	// friendship
	friend class ConnectorMetaData;
	friend class ::ArborContentApi;
};

} 

#endif  //  CONNECTOR_H
