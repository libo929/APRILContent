/**
 *  @file   ArborContent/include/ArborCheating/IsoHitClusteringAlgorithm.h
 *
 *  This algorithm is to remove part of isolated hits by clustering them
 * 
 * 
 */
#ifndef ISOHITCLUSTERINGALGORITHM_H
#define ISOHITCLUSTERINGALGORITHM_H 1

#include "Pandora/Algorithm.h"
#include "ArborUtility/KDTreeLinkerAlgoT.h"

namespace arbor_content
{

/**
 *  @brief IsoHitClusteringAlgorithm class
 */
class IsoHitClusteringAlgorithm : public pandora::Algorithm
{
public:
    /**
     *  @brief  Factory class for instantiating algorithm
     */
    class Factory : public pandora::AlgorithmFactory
    {
    public:
        pandora::Algorithm *CreateAlgorithm() const;
    };

    /**
     *  @brief Default constructor
     */
    IsoHitClusteringAlgorithm();

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

	void CreateCluster(const pandora::CaloHitList *const caloHitList) const;

	pandora::StatusCode GetIsoHits(pandora::CaloHitList& caloHitList);
  
    pandora::StatusCode MeanShiftClustering(pandora::CaloHitList& isoHitList, std::vector<pandora::CaloHitList>& clusterHitsCollection);

private:
	float                 m_kernelBandwidth;
	float                 m_clusterEpsilon;
};

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::Algorithm *IsoHitClusteringAlgorithm::Factory::CreateAlgorithm() const
{
    return new IsoHitClusteringAlgorithm();
}

}

#endif  // ISOHITCLUSTERINGALGORITHM_H
