/**
 *
 * This algorithm is to remove isolated hits by creating clusters
 * 
 */

#include "Pandora/AlgorithmHeaders.h"

#include "ArborClustering/IsoHitClusteringAlgorithm.h"
#include "ArborUtility/MeanShift.h"

using namespace pandora;

namespace arbor_content
{

  IsoHitClusteringAlgorithm::IsoHitClusteringAlgorithm()
  {
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitClusteringAlgorithm::Run()
  {
	CaloHitList isoHitList;
	GetIsoHits(isoHitList);
	std::cout << "isolated hit size: " << isoHitList.size() << std::endl;

    const ClusterList *pNewClusterList = NULL; std::string newClusterListName;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::CreateTemporaryListAndSetCurrent(*this, pNewClusterList, newClusterListName));

	std::vector<CaloHitList> clusterHitsCollection;
	MeanShiftClustering(isoHitList, clusterHitsCollection);

	for(int iList = 0; iList < clusterHitsCollection.size(); ++iList)
	{
		CreateCluster(&clusterHitsCollection.at(iList));
	}

	std::string m_outputClusterListName("ClustersFromIsoHits");
	std::cout << "the list " << m_outputClusterListName << " size: " << pNewClusterList->size() << std::endl;

    if (!pNewClusterList->empty())
    {
      PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::SaveList<Cluster>(*this, m_outputClusterListName));
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode IsoHitClusteringAlgorithm::MeanShiftClustering(CaloHitList& isoHitList, std::vector<CaloHitList>& clusterHitsCollection)
  {
	double shift_epsilon = 0.000001;

	MeanShift meanShift(m_kernelBandwidth, m_clusterEpsilon, shift_epsilon);

	vector<MSPoint> msCaloHitPoints;

	for(CaloHitList::iterator caloHitIter = isoHitList.begin(); caloHitIter != isoHitList.end(); ++caloHitIter)
	{
		CartesianVector hitPosition((*caloHitIter)->GetPositionVector());
		
		MSPoint msp(*caloHitIter);
		msp.push_back(hitPosition.GetX());
		msp.push_back(hitPosition.GetY());
		msp.push_back(hitPosition.GetZ());

		msCaloHitPoints.push_back(msp);
	}

	std::vector<MSCluster> msClusters = meanShift.cluster(msCaloHitPoints);

    std::cout << "  ======>>> meanshift cluster size: " << msClusters.size() << std::endl;	

	for(int iMSCluster = 0; iMSCluster < msClusters.size(); ++iMSCluster)
	{
		MSCluster& msCluster = msClusters.at(iMSCluster);
		std::vector<MSPoint>& caloHitPoints = msCluster.original_points;

#if 0
		std::cout << "|---> cluster: " << iMSCluster << std::endl;
#endif

		CaloHitList caloHitList;
		for(int iPoint = 0; iPoint < caloHitPoints.size(); ++iPoint)
		{
			MSPoint& hit = caloHitPoints.at(iPoint);
			const CaloHit* caloHit = hit.m_caloHit;
			CartesianVector hitPos(caloHit->GetPositionVector());

#if 0
			std::cout << "  ----> hit point: " << hit.at(0) << ", " << hit.at(1) << ", " << hit.at(2) 
				      << " --|-- " << hitPos.GetX() << ", " << hitPos.GetY() << ", " << hitPos.GetZ() << std::endl;
#endif

			caloHitList.insert(caloHit);
		}
	
		clusterHitsCollection.push_back(caloHitList);
	}

	return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  StatusCode IsoHitClusteringAlgorithm::GetIsoHits(CaloHitList& isoHitList)
  {
    const CaloHitList *pCaloHitList = NULL;
    PANDORA_RETURN_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::GetCurrentList(*this, pCaloHitList));

    CaloHitList localCaloHitList(pCaloHitList->begin(), pCaloHitList->end());

    for (CaloHitList::const_iterator caloHitIter = localCaloHitList.begin(), caloHitIterEnd = localCaloHitList.end(); 
			caloHitIter != caloHitIterEnd; ++caloHitIter)
    {
      try
      {
        const CaloHit *const pCaloHit = *caloHitIter;

        if (!PandoraContentApi::IsAvailable(*this, pCaloHit))
          continue;

		//energyMissingHits += pCaloHit->GetHadronicEnergy();
		isoHitList.insert(pCaloHit);
      }
      catch (StatusCodeException &)
      {
      }
    }

    return STATUS_CODE_SUCCESS;
  }

  //------------------------------------------------------------------------------------------------------------------------------------------

  void IsoHitClusteringAlgorithm::CreateCluster(const CaloHitList *const caloHitList) const
  {
	const Cluster *pCluster = NULL;
	const CaloHitList* pCaloHitList = caloHitList;

	const int MINCLUSTERSIZE = 3;

	if(pCaloHitList->size() < MINCLUSTERSIZE) return;

    PandoraContentApi::Cluster::Parameters parameters;
    parameters.m_caloHitList = *pCaloHitList;

	//std::cout << " need to CreateCluster" << std::endl;
	//
	
    PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Cluster::Create(*this, parameters, pCluster));

	if(pCluster==NULL)
	{
		std::cout << " --- cluster not created... " << std::endl;
		return;
	}

	float clusterEnergy = pCluster->GetHadronicEnergy();
	std::cout << "cluster energy: " << clusterEnergy << std::endl;

	// FIXME
	if(clusterEnergy<0.0001)
	{
		std::cout << " |=====>  a cluster with energy: " << pCluster->GetHadronicEnergy() 
			      << ", hit size: " << pCaloHitList->size() << ", not created ..." << std::endl;

		PANDORA_THROW_RESULT_IF(STATUS_CODE_SUCCESS, !=, PandoraContentApi::Delete(*this, pCluster));
	}
	else 
	{
		int pid = 0;

	    try
	    {
           const MCParticle *const pMCParticle(MCParticleHelper::GetMainMCParticle(pCluster));
		   pid = pMCParticle->GetParticleId();
	    }
        catch (StatusCodeException &)
        {
        }

#if 1
		std::cout << " |------>  created a cluster with PID: " << pid << ", energy: " << pCluster->GetHadronicEnergy() 
			      << ", hit size: " << pCaloHitList->size() << std::endl;
#endif
	}

#if 0
	pCluster->SetClusterForCaloHit();

	for(CaloHitList::iterator iter = pCaloHitList->begin(); iter != pCaloHitList->end(); ++iter)
	{
		const CaloHit* pCaloHit = *iter;

		std::cout << "-----====----- hit cluster: " << pCaloHit->GetCluster() << std::endl;
	}
#endif

  }

  //------------------------------------------------------------------------------------------------------------------------------------------
  
  StatusCode IsoHitClusteringAlgorithm::ReadSettings(const TiXmlHandle xmlHandle)
  {
	m_kernelBandwidth = 100.;

    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "KernelBandwidth", m_kernelBandwidth));

	m_clusterEpsilon = 30.;
    PANDORA_RETURN_RESULT_IF_AND_IF(pandora::STATUS_CODE_SUCCESS, pandora::STATUS_CODE_NOT_FOUND, !=, pandora::XmlHelper::ReadValue(xmlHandle,
        "ClusterEpsilon", m_clusterEpsilon));

    return STATUS_CODE_SUCCESS;
  }

}
