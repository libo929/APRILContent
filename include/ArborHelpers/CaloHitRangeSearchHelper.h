  /// \file CaloHitRangeSearchHelper.h
/*
 *
 * CaloHitRangeSearchHelper.h header template automatically generated by a class generator
 * Creation date : lun. mars 30 2015
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


#ifndef CALOHITHELPER_H
#define CALOHITHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"

#include <mlpack/core.hpp>
#include <mlpack/core/math/range.hpp>
#include <mlpack/methods/range_search/range_search.hpp>

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace arbor_content
{

/** 
 * @brief CaloHitRangeSearchHelper class
 */ 
class CaloHitRangeSearchHelper 
{
public:
    static pandora::StatusCode GetNeighbourHitsInRange(const pandora::CaloHitList *const pCaloHitList, 
		  const pandora::CaloHit *const pCaloHit, float distance, pandora::CaloHitVector& hitsInRange);

private:

    static pandora::StatusCode FillMatixFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix);

	static const pandora::CaloHitList* m_pCaloHitList;
	static pandora::CaloHitVector m_caloHitVector;
	static arma::mat m_caloHitsMatrix;

};

} 

#endif  