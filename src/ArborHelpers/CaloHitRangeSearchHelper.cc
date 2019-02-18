/// \file CaloHitRangeSearchHelper.cc
/*
 *
 * CaloHitRangeSearchHelper.cc source template automatically generated by a class generator
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
 &* 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */


#include "Api/PandoraContentApi.h"
#include "Objects/OrderedCaloHitList.h"
#include "ArborHelpers/CaloHitRangeSearchHelper.h"
#include "ArborApi/ArborContentApi.h"
#include "ArborObjects/CaloHit.h"


using namespace mlpack;
using namespace mlpack::range;
using namespace mlpack::math;

namespace arbor_content
{
	arma::mat CaloHitRangeSearchHelper::m_caloHitsMatrix(3,1);

	//
	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pCaloHitList(nullptr);

	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pCaloHitListOfLayers(nullptr);
	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pEcalCaloHitListOfLayers(nullptr);
	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pHcalCaloHitListOfLayers(nullptr);
	const pandora::CaloHitList* CaloHitRangeSearchHelper::m_pMuonCaloHitListOfLayers(nullptr);

	//
	pandora::CaloHitVector CaloHitRangeSearchHelper::m_caloHitVector;

	std::vector<pandora::CaloHitVector> CaloHitRangeSearchHelper::m_caloHitVectorOfLayers;
	std::vector<pandora::CaloHitVector> CaloHitRangeSearchHelper::m_ecalCaloHitVectorOfLayers;
	std::vector<pandora::CaloHitVector> CaloHitRangeSearchHelper::m_hcalCaloHitVectorOfLayers;
	std::vector<pandora::CaloHitVector> CaloHitRangeSearchHelper::m_muonCaloHitVectorOfLayers;

	//
	mlpack::range::RangeSearch<> CaloHitRangeSearchHelper::m_rangeSearch(m_caloHitsMatrix);
	CaloRangeSearch CaloHitRangeSearchHelper::m_caloRangeSearch(arma::mat(4,1));

	std::vector< mlpack::range::RangeSearch<> > CaloHitRangeSearchHelper::m_rangeSearchOfLayers;
	std::vector< mlpack::range::RangeSearch<> > CaloHitRangeSearchHelper::m_ecalRangeSearchOfLayers;
	std::vector< mlpack::range::RangeSearch<> > CaloHitRangeSearchHelper::m_hcalRangeSearchOfLayers;
	std::vector< mlpack::range::RangeSearch<> > CaloHitRangeSearchHelper::m_muonRangeSearchOfLayers;
	
	//
	pandora::OrderedCaloHitList CaloHitRangeSearchHelper::m_orderedCaloHitList;
	pandora::OrderedCaloHitList CaloHitRangeSearchHelper::m_orderedEcalCaloHitList;
	pandora::OrderedCaloHitList CaloHitRangeSearchHelper::m_orderedHcalCaloHitList;
	pandora::OrderedCaloHitList CaloHitRangeSearchHelper::m_orderedMuonCaloHitList;

	double CaloHitRangeSearchHelper::m_fFillingTime = 0.;
	double CaloHitRangeSearchHelper::m_fGetttingTime = 0.;

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::FillMatixByPoints(const std::vector<pandora::CartesianVector>& points, arma::mat& caloHitsMatrix)
  {
	  // first hit 
      arma::mat matrix(3, 1);

	  const pandora::CartesianVector& position0 = points.at(0);
	  matrix.col(0) = arma::vec( { position0.GetX(), position0.GetY(), position0.GetZ() } );

	  // other hits
	  matrix.insert_cols(1, points.size() - 1); 

	  for(int i = 1; i < points.size(); ++i)
	  {
	      const pandora::CartesianVector& position = points.at(i);
	      matrix.col(i) = arma::vec( { position.GetX(), position.GetY(), position.GetZ() } );
	  }

	  caloHitsMatrix = matrix;
	  
	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::FillMatixFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix)
  {
	  std::vector<pandora::CartesianVector> hitPoints;

	  for(int i = 0; i < caloHitVector.size(); ++i)
	  {
	      const pandora::CartesianVector& caloHitPosition = caloHitVector.at(i)->GetPositionVector();
		  hitPoints.push_back(caloHitPosition);
	  }

	  FillMatixByPoints(hitPoints, caloHitsMatrix);
	  
	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::FillMatix4DFromCaloHits(const pandora::CaloHitVector& caloHitVector, arma::mat& caloHitsMatrix4D)
  {
      arma::mat matrix4D(4, 1);

	  // first hit 
	  pandora::CartesianVector caloHitPosition0 = caloHitVector.at(0)->GetPositionVector();
	  matrix4D.col(0) = arma::vec( { caloHitPosition0.GetX(), caloHitPosition0.GetY(), caloHitPosition0.GetZ(), 
			  caloHitVector.at(0)->GetPseudoLayer() } );

	  // other hits
	  //std::cout << "caloHitVector.size : " << caloHitVector.size() << std::endl;
	  matrix4D.insert_cols(1, caloHitVector.size() - 1); 

	  for(int i = 1; i < caloHitVector.size(); ++i)
	  {
	      pandora::CartesianVector caloHitPosition = caloHitVector.at(i)->GetPositionVector();
	      matrix4D.col(i) = arma::vec( { caloHitPosition.GetX(), caloHitPosition.GetY(), caloHitPosition.GetZ(),
				 caloHitVector.at(i)->GetPseudoLayer() } );
	  }

	  caloHitsMatrix4D = matrix4D;
	  
	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildRangeSearch(const pandora::CaloHitList *const pCaloHitList)
  {
	  if(pCaloHitList != m_pCaloHitList)
	  {
		  //std::cout << "build new matrix..." << std::endl;
		  m_caloHitVector.clear();
	      m_caloHitVector.insert(m_caloHitVector.begin(), pCaloHitList->begin(), pCaloHitList->end());
		  FillMatixFromCaloHits(m_caloHitVector, m_caloHitsMatrix);

	      // the relatively time-comsuming part
	      m_rangeSearch.Train(m_caloHitsMatrix);
	  
		  m_pCaloHitList = pCaloHitList;
	  }

	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildOrderedHitsAndSearchRange(const pandora::CaloHitList *const pCaloHitList,
		  pandora::OrderedCaloHitList& orderedCaloHitList, std::vector<pandora::CaloHitVector>& caloHitVectorOfLayers,
		  std::vector< mlpack::range::RangeSearch<> >& rangeSearchOfLayers)
  {
	  orderedCaloHitList.Reset();
      PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, orderedCaloHitList.Add(*pCaloHitList));

	  //std::cout << "layer number: " << orderedCaloHitList.size() << std::endl;

	  // n.b. the total layer may not equal to the number of layer having hit, so set a maxisum
	  const int nMaxLayer = 100;
	  caloHitVectorOfLayers.clear();
	  caloHitVectorOfLayers.resize( nMaxLayer );

	  rangeSearchOfLayers.clear();
	  rangeSearchOfLayers.resize( nMaxLayer );

	  //loop each layer
	  for(auto listIter = orderedCaloHitList.begin(); listIter != orderedCaloHitList.end(); ++listIter)
	  {
		  auto hitsOfLayer = listIter->second;
		  //std::cout << "layer: " << listIter->first << ", hit: " << hitsOfLayer->size() << std::endl;
		  //
		  int layerIndex = listIter->first - 1;

		  pandora::CaloHitVector& caloHitVectorOfLayer = caloHitVectorOfLayers.at( layerIndex );

	      caloHitVectorOfLayer.insert(caloHitVectorOfLayer.begin(), hitsOfLayer->begin(), hitsOfLayer->end());

		  if(caloHitVectorOfLayer.empty()) continue;
	
		  arma::mat caloHitsMatrix(3,1);

		  FillMatixFromCaloHits(caloHitVectorOfLayer, caloHitsMatrix);

	      rangeSearchOfLayers.at( layerIndex ).Train(caloHitsMatrix);
	  }
	    
   	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildHitCollectionForSearching(
		  const pandora::CaloHitList *const               pCaloHitList,
		  const pandora::CaloHitList*&                    pCaloHitListOfLayers,
		  pandora::OrderedCaloHitList&                    orderedCaloHitList, 
		  std::vector<pandora::CaloHitVector>&            caloHitVectorOfLayers, 
		  std::vector< mlpack::range::RangeSearch<> >&    rangeSearchOfLayers)
  {
#if 0
	  // the same input hit list as before
	  // local varible address does not change...
	  std::cout << "input list: " << pCaloHitList << ", old list: " << pCaloHitListOfLayers << std::endl;
	  if(pCaloHitListOfLayers == pCaloHitList)
	  {
		  return pandora::STATUS_CODE_SUCCESS;
	  }
#endif

	  CaloHitRangeSearchHelper::BuildOrderedHitsAndSearchRange(pCaloHitList, orderedCaloHitList, caloHitVectorOfLayers, rangeSearchOfLayers);

	  pCaloHitListOfLayers = pCaloHitList;

   	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildHitCollectionOfLayers(const pandora::CaloHitList *const pCaloHitList)
  {
	  CaloHitRangeSearchHelper::BuildHitCollectionForSearching(pCaloHitList, m_pCaloHitListOfLayers,
			  m_orderedCaloHitList, m_caloHitVectorOfLayers, m_rangeSearchOfLayers);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildHitCollectionOfEcalLayers(const pandora::CaloHitList *const pEcalCaloHitList)
  {
	  CaloHitRangeSearchHelper::BuildHitCollectionForSearching(pEcalCaloHitList, m_pEcalCaloHitListOfLayers,
			  m_orderedEcalCaloHitList, m_ecalCaloHitVectorOfLayers, m_ecalRangeSearchOfLayers);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildHitCollectionOfHcalLayers(const pandora::CaloHitList *const pHcalCaloHitList)
  {
	  CaloHitRangeSearchHelper::BuildHitCollectionForSearching(pHcalCaloHitList, m_pHcalCaloHitListOfLayers,
			  m_orderedHcalCaloHitList, m_hcalCaloHitVectorOfLayers, m_hcalRangeSearchOfLayers);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::BuildHitCollectionOfMuonLayers(const pandora::CaloHitList *const pMuonCaloHitList)
  {
	  CaloHitRangeSearchHelper::BuildHitCollectionForSearching(pMuonCaloHitList, m_pMuonCaloHitListOfLayers,
			  m_orderedMuonCaloHitList, m_muonCaloHitVectorOfLayers, m_muonRangeSearchOfLayers);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::SearchHitsInRange(
		  mlpack::range::RangeSearch<>& rangeSearch, pandora::CaloHitVector& caloHitVector, 
		  pandora::CartesianVector testPosition, float distance, pandora::CaloHitList& hitsInRange) 
  {
	  hitsInRange.clear();

	  const Range range(0., distance);

	  /////
      arma::mat testPoint(3, 1);

	  testPoint.col(0)[0] = testPosition.GetX();
	  testPoint.col(0)[1] = testPosition.GetY();
	  testPoint.col(0)[2] = testPosition.GetZ();

	  //std::cout << "point: " << testPoint.col(0)[0] << ", " << testPoint.col(0)[1] << ", " << testPoint.col(0)[2] << std::endl;

      std::vector<std::vector<size_t> > resultingNeighbors;
      std::vector<std::vector<double> > resultingDistances;
      rangeSearch.Search(testPoint, range, resultingNeighbors, resultingDistances);

	  //std::cout << "resultingNeighbors size: " <<  resultingNeighbors.size() << ", resultingDistances size: " << resultingDistances.size() << std::endl;

      if(resultingNeighbors.size() != 1 ) 
	  {
		  std::cout << "error " << std::endl;

   	      return pandora::STATUS_CODE_SUCCESS;
	  }

      std::vector<size_t>& neighbors = resultingNeighbors.at(0);
      std::vector<double>& distances = resultingDistances.at(0);

	  // TODO
	  // may be sorted by distance
   
      for(size_t j=0; j < neighbors.size(); ++j)
      {
      	size_t neighbor = neighbors.at(j);
      	double hitsDist = distances.at(j);

#if 0
   	    auto posVec = m_caloHitVector.at(neighbor)->GetPositionVector();
   
      	std::cout <<  "    -> neighbor " << neighbor << ", hits distance: " << hitsDist 
   	  	 << ", X = " << posVec.GetX() << ", " << posVec.GetY() << ", " << posVec.GetZ() << std::endl;
#endif

		hitsInRange.push_back( caloHitVector.at(neighbor) );
      }

   	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------
  pandora::StatusCode CaloHitRangeSearchHelper::BuildCaloRangeSearch(const pandora::CaloHitVector& caloHitVector)
  {
	  arma::mat caloHitsMatrix4D;
	  FillMatix4DFromCaloHits(caloHitVector, caloHitsMatrix4D);

      m_caloRangeSearch.Train(caloHitsMatrix4D);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::SearchHitsInRange4D(const pandora::CaloHitVector& caloHitVector, 
		  const std::vector<float>& testPosition, float distance, pandora::CaloHitList& hitsInRange)
  {
	  hitsInRange.clear();

	  const Range range(0., distance);

      arma::mat matTestPoint(4, 1);

	  matTestPoint.col(0)[0] = testPosition.at(0);
	  matTestPoint.col(0)[1] = testPosition.at(1);
	  matTestPoint.col(0)[2] = testPosition.at(2);
	  matTestPoint.col(0)[3] = testPosition.at(3);

	  std::cout << " --- point: " << matTestPoint.col(0)[0] << ", " << matTestPoint.col(0)[1] << ", " << matTestPoint.col(0)[2] 
		  <<  ", " << matTestPoint.col(0)[3] << std::endl;

      std::vector<std::vector<size_t> > resultingNeighbors;
      std::vector<std::vector<double> > resultingDistances;

      m_caloRangeSearch.Search(matTestPoint, range, resultingNeighbors, resultingDistances);

      if(resultingNeighbors.size() != 1 ) 
	  {
		  std::cout << "error " << std::endl;

   	      return pandora::STATUS_CODE_SUCCESS;
	  }

      std::vector<size_t>& neighbors = resultingNeighbors.at(0);
      std::vector<double>& distances = resultingDistances.at(0);

      for(size_t j=0; j < neighbors.size(); ++j)
      {
      	size_t neighbor = neighbors.at(j);
      	double hitsDist = distances.at(j);

#if 0
   	    auto posVec = m_caloHitVector.at(neighbor)->GetPositionVector();
   
      	std::cout <<  "    -> neighbor " << neighbor << ", hits distance: " << hitsDist 
   	  	 << ", X = " << posVec.GetX() << ", " << posVec.GetY() << ", " << posVec.GetZ() << std::endl;
#endif

		hitsInRange.push_back( caloHitVector.at(neighbor) );
      }

   	  return pandora::STATUS_CODE_SUCCESS;
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::SearchNeighbourHitsInRange(pandora::CartesianVector testPosition, 
		  float distance, pandora::CaloHitList& hitsInRange)
  {
	  CaloHitRangeSearchHelper::SearchHitsInRange(m_rangeSearch, m_caloHitVector, testPosition, distance, hitsInRange);
  }

  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::SearchHitsInLayer(pandora::CartesianVector testPosition, int pseudoLayer,
		  float distance, pandora::CaloHitList& hitsInRange)
  {
	  int layerIndex = pseudoLayer - 1;

	  mlpack::range::RangeSearch<>& rangeSearch = m_rangeSearchOfLayers.at( layerIndex );
	  pandora::CaloHitVector& caloHitVectorOfLayer = m_caloHitVectorOfLayers.at( layerIndex );

	  CaloHitRangeSearchHelper::SearchHitsInRange(rangeSearch, caloHitVectorOfLayer, testPosition, distance, hitsInRange);
  }
  
  //--------------------------------------------------------------------------------------------------------------------

  pandora::StatusCode CaloHitRangeSearchHelper::SearchEcalHitsInLayer(pandora::CartesianVector testPosition, int pseudoLayer,
		  float distance, pandora::CaloHitList& hitsInRange)
  {
	  int layerIndex = pseudoLayer - 1;

	  mlpack::range::RangeSearch<>& rangeSearch = m_ecalRangeSearchOfLayers.at( layerIndex );
	  pandora::CaloHitVector& caloHitVectorOfLayer = m_ecalCaloHitVectorOfLayers.at( layerIndex );

	  CaloHitRangeSearchHelper::SearchHitsInRange(rangeSearch, caloHitVectorOfLayer, testPosition, distance, hitsInRange);
  }

  //--------------------------------------------------------------------------------------------------------------------
  
  pandora::StatusCode CaloHitRangeSearchHelper::SearchHcalHitsInLayer(pandora::CartesianVector testPosition, int pseudoLayer,
		  float distance, pandora::CaloHitList& hitsInRange)
  {
	  int layerIndex = pseudoLayer - 1;

	  mlpack::range::RangeSearch<>& rangeSearch = m_hcalRangeSearchOfLayers.at( layerIndex );
	  pandora::CaloHitVector& caloHitVectorOfLayer = m_hcalCaloHitVectorOfLayers.at( layerIndex );

	  CaloHitRangeSearchHelper::SearchHitsInRange(rangeSearch, caloHitVectorOfLayer, testPosition, distance, hitsInRange);
  }

  //--------------------------------------------------------------------------------------------------------------------
  
  pandora::StatusCode CaloHitRangeSearchHelper::SearchMuonHitsInLayer(pandora::CartesianVector testPosition, int pseudoLayer,
		  float distance, pandora::CaloHitList& hitsInRange)
  {
	  int layerIndex = pseudoLayer - 1;

	  mlpack::range::RangeSearch<>& rangeSearch = m_muonRangeSearchOfLayers.at( layerIndex );
	  pandora::CaloHitVector& caloHitVectorOfLayer = m_muonCaloHitVectorOfLayers.at( layerIndex );

	  CaloHitRangeSearchHelper::SearchHitsInRange(rangeSearch, caloHitVectorOfLayer, testPosition, distance, hitsInRange);
  }

} 

