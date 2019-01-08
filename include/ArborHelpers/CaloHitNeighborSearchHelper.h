  /// \file CaloHitNeighborSearchHelper.h
/*
 *
 * CaloHitNeighborSearchHelper.h header template automatically generated by a class generator
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


#ifndef CALOHITNEIGHBORSEARCHHELPER_H
#define CALOHITNEIGHBORSEARCHHELPER_H

#include <vector>

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"

#include "ArborObjects/CaloHit.h"
#include "ArborObjects/Connector.h"

#include "ArborHelpers/CaloMetric.h"

#include <mlpack/core.hpp>
#include <mlpack/core/math/range.hpp>
#include <mlpack/methods/neighbor_search/neighbor_search.hpp>

#include <mlpack/methods/dbscan/dbscan.hpp>

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace arbor_content
{

typedef mlpack::neighbor::NeighborSearch<mlpack::neighbor::NearestNeighborSort, CaloMetric, arma::mat, mlpack::tree::MeanSplitBallTree> CaloKNN;

typedef mlpack::dbscan::DBSCAN< mlpack::range::RangeSearch<CaloMetric, arma::mat, mlpack::tree::MeanSplitBallTree> > CaloDBSCAN;

/** 
 * @brief CaloHitNeighborSearchHelper class
 */ 
class CaloHitNeighborSearchHelper 
{
public:
	// build 
    static pandora::StatusCode BuildNeighborSearch(const pandora::CaloHitList *const pCaloHitList);
    static pandora::StatusCode BuildCaloNeighborSearch(const pandora::CaloHitVector& caloHitVector);
	
    // search 
    static pandora::StatusCode SearchNeighbourHits(pandora::CartesianVector testPosition, int nNeighbor, pandora::CaloHitList& neighborHits);
  
    static pandora::StatusCode SearchNeighbourHits4D(const pandora::CaloHitVector& caloHitVector, 
		  std::vector<float> testPosition, int nNeighbor, pandora::CaloHitList& neighborHits);
  
	static pandora::StatusCode ClusteringByDBSCAN(const pandora::CaloHitVector& caloHitVector, std::vector<pandora::CaloHitVector>& hitsForCluster);

private:

    static pandora::StatusCode FillMatixFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix);
	static pandora::StatusCode FillMatix4DFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix4D);

	static arma::mat m_caloHitsMatrix;
	static arma::mat m_caloHitsMatrix4D; // store calohit in 4D (x, y, z, layer)

	// all hits
	static const pandora::CaloHitList* m_pCaloHitList;

	//-----------------
	static pandora::CaloHitVector m_caloHitVector;

	static mlpack::neighbor::KNN m_neighborSearch;
	static CaloKNN m_neighborSearch4D;

	static CaloDBSCAN m_caloDBSCAN;
};

} 

#endif  
