/**
 *  @file   NearbyHitRecoveryAlgorithm.cc
 * 
 *  @brief  Implementation of the cheating cluster cleaning algorithm class
 * 
 *  $Log: $
 */

#include "Pandora/AlgorithmHeaders.h"

#include "APRILTools/ConnectorAlgorithmTool.h"
#include "APRILUtility/NearbyHitRecoveryAlgorithm.h"
#include "APRILHelpers/ClusterHelper.h"
#include "APRILHelpers/CaloHitRangeSearchHelper.h"
#include "APRILHelpers/CaloHitNeighborSearchHelper.h"
#include "APRILHelpers/HistogramHelper.h"
#include "APRILUtility/EventPreparationAlgorithm.h"
#include "APRILApi/APRILContentApi.h"

namespace april_content
{

TMVA::Reader* NearbyHitRecoveryAlgorithm::m_reader = nullptr;

NearbyHitRecoveryAlgorithm::~NearbyHitRecoveryAlgorithm()
{
	if(m_reader != nullptr) {
		delete m_reader;
		m_reader = nullptr;
	
		std::cout << "delete reader: " << m_reader << std::endl;
	}
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::Initialize()
{
	if(m_reader==nullptr) 
	{
        m_reader = new TMVA::Reader( "!Color:!Silent" ); 
        //m_reader = new TMVA::Reader(); 

	    m_reader->AddVariable("clusterEMEnergy",         &m_clusterEMEnergy        );
	    m_reader->AddVariable("clusterHadEnergy",        &m_clusterHadEnergy       );
	    m_reader->AddVariable("hitEMEnergy",             &m_hitEMEnergy            );
	    m_reader->AddVariable("hitHadEnergy",            &m_hitHadEnergy           );
	    m_reader->AddVariable("hitLayer",                &m_hitLayer               );
	    m_reader->AddVariable("nhits",                   &m_nhits                  );
	    m_reader->AddVariable("centroidDistance",        &m_centroidDistance       );
	    m_reader->AddVariable("closestDistance",         &m_closestDistance        );
		m_reader->AddVariable("meanDensity",             &m_meanDensity            );
		m_reader->AddVariable("surroundingEnergy",       &m_surroundingEnergy      );  

        // --- Book the MVA methods
        TString methodName = TString("HitMergingMVA");
        TString weightfile = TString("/media/libo/HD1/workplace/APRILPFA/testNew/mva/dataset/weights/TMVAClassification_BDT.weights.xml");

        try 
        {
	    	m_reader->BookMVA(methodName, weightfile); 
        }
        catch (...)
        {
	    	std::cout << "Error: MVA reader !!!" << std::endl;
			delete m_reader;
			m_reader = nullptr;
			//return pandora::STATUS_CODE_NOT_INITIALIZED;
        }
	}

	std::cout << "reader: " << m_reader << std::endl;
	return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::Run()
{
#if 0
	MakeSamples();
#else

	ClusterCaloHitListMap clusterCaloHitListMap;

	//MVAMergeHitToCluster(clusterCaloHitListMap);

	MakeClusterHitsAssociation(clusterCaloHitListMap);

	AddHitToCluster(clusterCaloHitListMap);

	// nearby hits clustering
	if(m_pAlgorithmTool != nullptr)
	{
		ClusteringByTool(m_pAlgorithmTool);
	}

	// -----------------------------------------------------------------------------------------------------------------
	
	int nUnclusteredHits = 0;

    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
		   ++nUnclusteredHits;
		}
	}

	std::cout << "===unClusteredHits size: " << nUnclusteredHits << std::endl;
#endif
    
	return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::MakeSamples()
{
    const pandora::ClusterList *pClusterList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	std::cout << " ====== total cluster list size: " << pClusterList->size() << std::endl;

	///////////////////////////////////////////////////////////////////////////////////////////
	pandora::ClusterVector clusterVector;
	clusterVector.insert(clusterVector.begin(), pClusterList->begin(), pClusterList->end());
	std::cout << " ====== total cluster vector size: " << clusterVector.size() << std::endl;

	std::vector<ClusterParamatersPlus> clusterParaVec;
	std::vector<pandora::CartesianVector> clusterCentroids;
	///////////////////////////////////////////////////////////////////////////////////////////

	for(auto clu : clusterVector)
	{
		float surroundingEnergy, meanDensity;
		pandora::CartesianVector centroid(0., 0., 0);

		ClusterHelper::GetMeanSurroundingEnergy(clu, surroundingEnergy);
		ClusterHelper::GetMeanDensity(clu, meanDensity);
		ClusterHelper::GetCentroid(clu, centroid);
		clusterCentroids.push_back(centroid);

		ClusterParamatersPlus cluPara(clu, meanDensity, surroundingEnergy);
		clusterParaVec.push_back(cluPara);
	}

	arma::mat clusterCentroidsMatrix;
	CaloHitRangeSearchHelper::FillMatixByPoints(clusterCentroids, clusterCentroidsMatrix);
	
	mlpack::neighbor::KNN neighborSearch(clusterCentroidsMatrix);

    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

		pandora::ClusterList neighborClusters;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
			pandora::CartesianVector testPosition = pCaloHit->GetPositionVector();

            arma::mat testPoint(3, 1);
	        testPoint.col(0)[0] = testPosition.GetX();
	        testPoint.col(0)[1] = testPosition.GetY();
	        testPoint.col(0)[2] = testPosition.GetZ();
	
			arma::Mat<size_t> resultingNeighbors;
			arma::mat resultingDistances;
			int nNeighbor = 3;
			neighborSearch.Search(testPoint, nNeighbor, resultingNeighbors, resultingDistances);
    
            for(size_t j=0; j < resultingNeighbors.n_elem; ++j)
            {
            	size_t neighbor = resultingNeighbors[j];
            	//double hitsDist = resultingDistances[j];

				//std::cout << "--- cluster: " << clusterVector.at(neighbor) << std::endl;
				neighborClusters.push_back( clusterVector.at(neighbor) );
			}
		
			//std::cout << "-neighor clusters: " << neighborClusters.size() << std::endl;
			GetNearbyClustersByDistance(pCaloHit, neighborClusters);
			//std::cout << "--neighor clusters: " << neighborClusters.size() << std::endl;

			for(auto cluster : neighborClusters)
			{
				// ---
				int nhits = cluster->GetNCaloHits();


				// ---
				float clusterEMEnergy = cluster->GetElectromagneticEnergy();
				float clusterHadEnergy = cluster->GetHadronicEnergy();
				float hitEMEnergy = pCaloHit->GetElectromagneticEnergy();
				float hitHadEnergy = pCaloHit->GetHadronicEnergy();

				float hitType = pCaloHit->GetHitType();
				float hitLayer = pCaloHit->GetPseudoLayer();
				float clusterInnerHitType = cluster->GetInnerLayerHitType();
				float clusterOuterHitType = cluster->GetOuterLayerHitType();


				// ---
				float centroidDistance;

				// ---
				float closestDistance;

				const pandora::CartesianVector hitPoint = pCaloHit->GetPositionVector();
				ClusterHelper::GetCentroidDistance(cluster, hitPoint, centroidDistance);
				ClusterHelper::GetClosestDistanceApproach(cluster, hitPoint, closestDistance);

				//------------------
	            const pandora::MCParticle* pClusterMCParticle  = nullptr;

				//int clusterPDG = 0;

				try
				{
					pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(cluster);
					//clusterPDG = pClusterMCParticle->GetParticleId();
				}
                catch (pandora::StatusCodeException &)
				{
				}

				//------------------
	            const pandora::MCParticle* pHitMCParticle  = nullptr;

				try
				{
					pHitMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pCaloHit);
				}
                catch (pandora::StatusCodeException &)
				{
				}

				// ---

				bool isRightHitMerging = false;

				if(pHitMCParticle != nullptr && pClusterMCParticle != nullptr) 
				{
					isRightHitMerging = (pHitMCParticle == pClusterMCParticle);
				}

		
				float surroundingEnergy = 0.;
				float meanDensity = 0.;

				ClusterHelper::GetMeanSurroundingEnergy(cluster, surroundingEnergy);
				ClusterHelper::GetMeanDensity(cluster, meanDensity);
				/////////////////////////////////////////////////////////////////////////////////////////
	            std::vector<float> vars;
	            vars.push_back( clusterEMEnergy );
	            vars.push_back( clusterHadEnergy );
	            vars.push_back( hitEMEnergy );
	            vars.push_back( hitHadEnergy );
	            vars.push_back( hitType );
	            vars.push_back( hitLayer );
	            vars.push_back( clusterInnerHitType );
	            vars.push_back( clusterOuterHitType );
	            vars.push_back( nhits );
	            vars.push_back( centroidDistance );
	            vars.push_back( closestDistance );
				vars.push_back( meanDensity );
				vars.push_back( surroundingEnergy );

				//bool isPhoton = (clusterPDG == 22);
	            //vars.push_back( float(isPhoton) );
	            //vars.push_back( float(isRightHitMerging) );

				if(isRightHitMerging)
				{
					HistogramManager::CreateFill("HitClusterCombinationTrue", "clusterEMEnergy:clusterHadEnergy:hitEMEnergy:hitHadEnergy:hitType:hitLayer:clusterInnerHitType:clusterOuterHitType:nhits:centroidDistance:closestDistance:meanDensity:surroundingEnergy", vars);
				}
				else
				{
					HistogramManager::CreateFill("HitClusterCombinationFalse", "clusterEMEnergy:clusterHadEnergy:hitEMEnergy:hitHadEnergy:hitType:hitLayer:clusterInnerHitType:clusterOuterHitType:nhits:centroidDistance:closestDistance:meanDensity:surroundingEnergy", vars);
				}
			}
		}
	}

	return pandora::STATUS_CODE_SUCCESS;
}

float NearbyHitRecoveryAlgorithm::DeterminMergingByMVA(const pandora::Cluster* cluster, const pandora::CaloHit* caloHit)
{
	// ---
	float clusterEMEnergy = cluster->GetElectromagneticEnergy();
	float clusterHadEnergy = cluster->GetHadronicEnergy();
	float hitEMEnergy = caloHit->GetElectromagneticEnergy();
	float hitHadEnergy = caloHit->GetHadronicEnergy();

	//float hitType = caloHit->GetHitType();
	float hitLayer = caloHit->GetPseudoLayer();
	//float clusterInnerHitType = cluster->GetInnerLayerHitType();
	//float clusterOuterHitType = cluster->GetOuterLayerHitType();

	// ---
	int nhits = cluster->GetNCaloHits();

	// ---
	float centroidDistance;

	// ---
	float closestDistance;

	const pandora::CartesianVector hitPoint = caloHit->GetPositionVector();
	ClusterHelper::GetCentroidDistance(cluster, hitPoint, centroidDistance);
	ClusterHelper::GetClosestDistanceApproach(cluster, hitPoint, closestDistance);

	float surroundingEnergy = 0.;
	float meanDensity = 0.;

	ClusterHelper::GetMeanSurroundingEnergy(cluster, surroundingEnergy);
	ClusterHelper::GetMeanDensity(cluster, meanDensity);
	
	/////////////////////////////////////////////////////////////////////////////////////////
	m_clusterEMEnergy     = clusterEMEnergy;
	m_clusterHadEnergy    = clusterHadEnergy;
	m_hitEMEnergy         = hitEMEnergy;
	m_hitHadEnergy        = hitHadEnergy;
	m_hitLayer            = hitLayer;
	m_nhits               = nhits;
	m_centroidDistance    = centroidDistance;
	m_closestDistance     = closestDistance;
	m_meanDensity         = meanDensity;
	m_surroundingEnergy   = surroundingEnergy;

	float mvaValue = m_reader->EvaluateMVA("HitMergingMVA");

	return mvaValue;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::MVAMergeHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap)
{
    const pandora::ClusterList *pClusterList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterList));

	std::cout << " ====== total cluster list size: " << pClusterList->size() << std::endl;

	///////////////////////////////////////////////////////////////////////////////////////////
	pandora::ClusterVector clusterVector;
	clusterVector.insert(clusterVector.begin(), pClusterList->begin(), pClusterList->end());
	std::cout << " ====== total cluster vector size: " << clusterVector.size() << std::endl;

	std::vector<ClusterParamatersPlus> clusterParaVec;
	std::vector<pandora::CartesianVector> clusterCentroids;
	///////////////////////////////////////////////////////////////////////////////////////////

	for(auto clu : clusterVector)
	{
		float surroundingEnergy, meanDensity;
		pandora::CartesianVector centroid(0., 0., 0);

		ClusterHelper::GetMeanSurroundingEnergy(clu, surroundingEnergy);
		ClusterHelper::GetMeanDensity(clu, meanDensity);
		ClusterHelper::GetCentroid(clu, centroid);
		clusterCentroids.push_back(centroid);

		ClusterParamatersPlus cluPara(clu, meanDensity, surroundingEnergy);
		clusterParaVec.push_back(cluPara);
	}

	arma::mat clusterCentroidsMatrix;
	CaloHitRangeSearchHelper::FillMatixByPoints(clusterCentroids, clusterCentroidsMatrix);
	
	mlpack::neighbor::KNN neighborSearch(clusterCentroidsMatrix);

	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

		pandora::ClusterList neighborClusters;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
			pandora::CartesianVector testPosition = pCaloHit->GetPositionVector();

            arma::mat testPoint(3, 1);
	        testPoint.col(0)[0] = testPosition.GetX();
	        testPoint.col(0)[1] = testPosition.GetY();
	        testPoint.col(0)[2] = testPosition.GetZ();
	
			arma::Mat<size_t> resultingNeighbors;
			arma::mat resultingDistances;
			int nNeighbor = 5;
			neighborSearch.Search(testPoint, nNeighbor, resultingNeighbors, resultingDistances);
    
            for(size_t j=0; j < resultingNeighbors.n_elem; ++j)
            {
            	size_t neighbor = resultingNeighbors[j];
            	//double hitsDist = resultingDistances[j];

				//std::cout << "--- cluster: " << clusterVector.at(neighbor) << std::endl;
				neighborClusters.push_back( clusterVector.at(neighbor) );
			}
		
			//std::cout << "-neighor clusters: " << neighborClusters.size() << std::endl;
			GetNearbyClustersByDistance(pCaloHit, neighborClusters);
			//std::cout << "--neighor clusters: " << neighborClusters.size() << std::endl;

			const pandora::Cluster* bestCluster = nullptr;
			float bestMVAValue = -1.e6;
			bool isRightMerging = false;

			for(auto cluster : neighborClusters)
			{
				// ---
				float clusterEMEnergy = cluster->GetElectromagneticEnergy();
				float clusterHadEnergy = cluster->GetHadronicEnergy();
				float hitEMEnergy = pCaloHit->GetElectromagneticEnergy();
				float hitHadEnergy = pCaloHit->GetHadronicEnergy();

				//float hitType = pCaloHit->GetHitType();
				float hitLayer = pCaloHit->GetPseudoLayer();
				//float clusterInnerHitType = cluster->GetInnerLayerHitType();
				//float clusterOuterHitType = cluster->GetOuterLayerHitType();

				// ---
				int nhits = cluster->GetNCaloHits();

				// ---
				float centroidDistance;

				// ---
				float closestDistance;

				const pandora::CartesianVector hitPoint = pCaloHit->GetPositionVector();
				ClusterHelper::GetCentroidDistance(cluster, hitPoint, centroidDistance);
				ClusterHelper::GetClosestDistanceApproach(cluster, hitPoint, closestDistance);

				//------------------
	            const pandora::MCParticle* pClusterMCParticle  = nullptr;

				//int clusterPDG = 0;

				try
				{
					pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(cluster);
					//clusterPDG = pClusterMCParticle->GetParticleId();
				}
                catch (pandora::StatusCodeException &)
				{
				}

				//------------------
	            const pandora::MCParticle* pHitMCParticle  = nullptr;

				try
				{
					pHitMCParticle = pandora::MCParticleHelper::GetMainMCParticle(pCaloHit);
				}
                catch (pandora::StatusCodeException &)
				{
				}

				// ---
				//bool isPhoton = (clusterPDG == 22);

				bool isRightHitMerging = false;

				if(pHitMCParticle != nullptr && pClusterMCParticle != nullptr) 
				{
					isRightHitMerging = (pHitMCParticle == pClusterMCParticle);
				}

		
				float surroundingEnergy = 0.;
				float meanDensity = 0.;

				ClusterHelper::GetMeanSurroundingEnergy(cluster, surroundingEnergy);
				ClusterHelper::GetMeanDensity(cluster, meanDensity);
	
				/////////////////////////////////////////////////////////////////////////////////////////
	            m_clusterEMEnergy     = clusterEMEnergy;
	            m_clusterHadEnergy    = clusterHadEnergy;
	            m_hitEMEnergy         = hitEMEnergy;
	            m_hitHadEnergy        = hitHadEnergy;
	            m_hitLayer            = hitLayer;
	            m_nhits               = nhits;
	            m_centroidDistance    = centroidDistance;
	            m_closestDistance     = closestDistance;
				m_meanDensity         = meanDensity;
				m_surroundingEnergy   = surroundingEnergy;

				float mvaValue = m_reader->EvaluateMVA("HitMergingMVA");

				if(mvaValue > bestMVAValue) 
				{
					bestMVAValue = mvaValue;
					bestCluster = cluster;
					isRightMerging = isRightHitMerging;
				}
			}

	        std::vector<float> vars;
	        vars.push_back( bestMVAValue );
			vars.push_back( float(isRightMerging) );
			HistogramManager::CreateFill("MVAMergeHitToCluster", "mvaValue:isRight", vars);

			/////////////////////
			if(bestMVAValue > 0.8)
			{
		       if(clusterCaloHitListMap.find(bestCluster) == clusterCaloHitListMap.end())
		       {
		           pandora::CaloHitList hitList;
		           hitList.push_back(pCaloHit);
		           clusterCaloHitListMap[bestCluster] = hitList;
		       }
		       else
		       {
		           clusterCaloHitListMap[bestCluster].push_back(pCaloHit);
		       }
			
			   HistogramManager::CreateFill("MVAMergeHitToClusterPassed", "mvaValue:isRight", vars);
			}
		}
	}

	return pandora::STATUS_CODE_SUCCESS;
}


pandora::StatusCode NearbyHitRecoveryAlgorithm::ClusteringByTool(pandora::AlgorithmTool* /* pAlgorithmTool */)
{
    const pandora::ClusterList *pNewClusterList = nullptr; 
	std::string clusterListName;

    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, 
	PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, clusterListName));

	ConnectorAlgorithmTool *pTool = dynamic_cast<ConnectorAlgorithmTool*>(m_pAlgorithmTool);

	if(pTool != nullptr) pTool->Process(*this);
	
	std::cout << "NearbyHitRecoveryAlgorithm: created new clusters size: " << pNewClusterList->size() << std::endl;

	//////////
	pandora::ClusterList clustersToSave;

    const pandora::ClusterList *pClusterListToSave = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pClusterListToSave));

	std::cout << "temp cluster to save: " << pClusterListToSave->size() << std::endl;

	if (pClusterListToSave->empty()) return pandora::STATUS_CODE_SUCCESS;

	for(auto it = pClusterListToSave->begin(); it != pClusterListToSave->end(); ++it)
	{
		clustersToSave.push_back(*it);
	}
	
	std::string m_mergedClusterListName("MergedClustersForPFOCreation");

	// save clusters to the exsiting cluster list
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_INITIALIZED, !=, 
			PandoraContentApi::SaveList(*this, m_mergedClusterListName, clustersToSave));

	return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::MakeClusterHitsAssociation(ClusterCaloHitListMap& clusterCaloHitListMap)
{
    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

	int nUnclusteredHits = 0;

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
		   ++nUnclusteredHits;

		   pandora::CartesianVector testPosition = pCaloHit->GetPositionVector();
		   
		   const int nNeighbor = m_nNeighborHits;
		   pandora::CaloHitList neighborHits;

		   CaloHitNeighborSearchHelper::SearchNeighbourHits(testPosition, nNeighbor, neighborHits);

		   //std::cout << " calo hit is not clustered: " << pCaloHit << ", X = " << testPosition.GetX() << ", " << testPosition.GetY() 
		   //	<< ", " << testPosition.GetZ() << std::endl;

		   const pandora::Cluster* clusterToAdd = nullptr;
		   float hitsDistance = 0.;

		   for(auto& caloHit : neighborHits)
		   {
			   auto& hitPos = caloHit->GetPositionVector();
               const april_content::CaloHit *const pAPRILCaloHit = dynamic_cast<const april_content::CaloHit *const>(caloHit);
			   //std::cout << "     the nearby hit distance: " << (hitPos - testPosition).GetMagnitude() 
				 //  << ", pos: " << hitPos.GetX() << ", " << hitPos.GetY() << ", " << hitPos.GetZ() 
				   //<< ", cluster: " << pAPRILCaloHit->GetMother() << std::endl;

			   if(pAPRILCaloHit != nullptr && clusterToAdd == nullptr)
			   {
				   clusterToAdd = pAPRILCaloHit->GetMother();
				   hitsDistance = (hitPos - testPosition).GetMagnitude();
			   }
		   }

		   if(clusterToAdd != nullptr && hitsDistance < m_maxHitsDistance)
		   {
		      if(clusterCaloHitListMap.find( clusterToAdd ) == clusterCaloHitListMap.end())
		      {
		          pandora::CaloHitList hitList;
		          hitList.push_back( pCaloHit );
		          clusterCaloHitListMap[clusterToAdd] = hitList;
		      }
		      else
		      {
		          clusterCaloHitListMap[clusterToAdd].push_back( pCaloHit );
		      }

		      const pandora::MCParticle* pClusterMCParticle  = nullptr;
		   
			  //float mvaValue = DeterminMergingByMVA(clusterToAdd, pCaloHit);

              try
              {
              	 pClusterMCParticle = pandora::MCParticleHelper::GetMainMCParticle(clusterToAdd);
           	  
		         const pandora::MCParticle* pMCHitParticle = nullptr;
				 pMCHitParticle = pandora::MCParticleHelper::GetMainMCParticle(pCaloHit);

		         //std::cout << "  ====== hit: " << pCaloHit << ", mcp: " << pMCHitParticle 
		   	     // << ", the cluster which may merge the hit: " << clusterToAdd << ", mcp :" << pClusterMCParticle << endl;

			     int hitMCPCharge = pandora::PdgTable::GetParticleCharge(pMCHitParticle->GetParticleId());
			     int clusterMCPCharge = pandora::PdgTable::GetParticleCharge(pClusterMCParticle->GetParticleId());


	             std::vector<float> vars;
	             vars.push_back( float(EventPreparationAlgorithm::GetEventNumber()) );
	             vars.push_back( hitsDistance );
	             vars.push_back( float(pMCHitParticle == pClusterMCParticle) );
				 vars.push_back( float(pMCHitParticle->GetParticleId()) );
				 vars.push_back( float(pClusterMCParticle->GetParticleId()) );
	             vars.push_back( float(hitMCPCharge) );
	             vars.push_back( float(clusterMCPCharge) );
	             //vars.push_back( mvaValue );

		         HistogramManager::CreateFill("AddNearbyHitToCluster", 
			     "evtNumber:hitsDistance:isRight:hitPDG:clusterPDG:hitMCPCharge:clusterMCPCharge", vars);
			  }
              catch (pandora::StatusCodeException &)
              {
		      }
		   }
		}
		else
		{
			//const april_content::CaloHit *const pAPRILCaloHit = dynamic_cast<const april_content::CaloHit *const>(pCaloHit);
			//std::cout << "hit cluster: " << pAPRILCaloHit->GetMother() << std::endl;
		}
	}

	std::cout << "===unClusteredHits size: " << nUnclusteredHits << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::GetNearbyClustersByDistance(const pandora::CaloHit* pCaloHit, pandora::ClusterList& clusterList)
{
	pandora::CartesianVector testPosition = pCaloHit->GetPositionVector();
	
	const int nNeighbor = m_nNeighborHits;
	pandora::CaloHitList neighborHits;

	CaloHitNeighborSearchHelper::SearchNeighbourHits(testPosition, nNeighbor, neighborHits);

	//std::cout << " calo hit is not clustered: " << pCaloHit << ", X = " << testPosition.GetX() << ", " << testPosition.GetY() 
	//	<< ", " << testPosition.GetZ() << std::endl;

	for(auto& caloHit : neighborHits)
	{
	    //auto& hitPos = caloHit->GetPositionVector();
        const april_content::CaloHit *const pAPRILCaloHit = dynamic_cast<const april_content::CaloHit *const>(caloHit);
	    //std::cout << "     the nearby hit distance: " << (hitPos - testPosition).GetMagnitude() 
	 	 //  << ", pos: " << hitPos.GetX() << ", " << hitPos.GetY() << ", " << hitPos.GetZ() 
	 	   //<< ", cluster: " << pAPRILCaloHit->GetMother() << std::endl;

	    if(pAPRILCaloHit != nullptr)
	    {
	 	   const pandora::Cluster* clusterToAdd = pAPRILCaloHit->GetMother();
        
		   if(clusterToAdd != nullptr)
		   {
		       //std::cout << " === cluster: " << clusterToAdd << std::endl;

			   if(std::find(clusterList.begin(), clusterList.end(), clusterToAdd) == clusterList.end())
		       {
		           //std::cout << " ====== add cluster: " << clusterToAdd << std::endl;
		           clusterList.push_back(clusterToAdd);
		       }
		   }
	    }
	}

    return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode NearbyHitRecoveryAlgorithm::AddHitToCluster(ClusterCaloHitListMap& clusterCaloHitListMap)
{
	// add the unused hit to exsiting clusters (not by MCP)
	for(auto clusterIt = clusterCaloHitListMap.begin(); clusterIt != clusterCaloHitListMap.end(); ++clusterIt)
	{
		const auto& cluster = clusterIt->first;
		const auto& caloHitList = clusterIt->second;

		if(caloHitList.empty()) continue;

		const pandora::CaloHitList& hitsAddToCluster = caloHitList;

        PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, APRILContentApi::AddToCluster(*this, cluster, &hitsAddToCluster));
	}

	// check 
    const pandora::CaloHitList *pCaloHitList = nullptr; 
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));
	//std::cout << "------- # CaloHit : " << pCaloHitList->size() << std::endl;
	
	pandora::CaloHitList unClusteredHits;

    for(pandora::CaloHitList::const_iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const pandora::CaloHit* const pCaloHit = *iter;

        if (PandoraContentApi::IsAvailable(*this, pCaloHit))
		{
			unClusteredHits.push_back(pCaloHit);
		}
	}

	std::cout << "===unClusteredHits size: " << unClusteredHits.size() << std::endl;

    return pandora::STATUS_CODE_SUCCESS;
}
	
//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode NearbyHitRecoveryAlgorithm::ReadSettings(const pandora::TiXmlHandle xmlHandle)
{
    m_nNeighborHits = 10;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "NeighborHitsNumber", m_nNeighborHits));

    m_maxHitsDistance = std::numeric_limits<float>::max();
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "MaxHitsDistance", m_maxHitsDistance));

    m_pAlgorithmTool = nullptr;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, 
        pandora::XmlHelper::ProcessAlgorithmTool(*this, xmlHandle, "CaloHitMergingTool", m_pAlgorithmTool));

    return pandora::STATUS_CODE_SUCCESS;
}

} // namespace april_content
