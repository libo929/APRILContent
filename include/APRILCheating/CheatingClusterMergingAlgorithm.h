/**
 *  @file   APRILContent/include/APRILCheating/CheatingClusterMergingAlgorithm.h
 * 
 *  @brief  Header file for the cheating cluster cleaning algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_CHEATING_CLUSTER_MERGING_ALGORITHM_H
#define APRIL_CHEATING_CLUSTER_MERGING_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief CheatingClusterMergingAlgorithm class
 */
class CheatingClusterMergingAlgorithm : public pandora::Algorithm
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

private:
    pandora::StatusCode Run();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);
};

inline pandora::Algorithm *CheatingClusterMergingAlgorithm::Factory::CreateAlgorithm() const
{
    return new CheatingClusterMergingAlgorithm();
}

} // namespace APRIL_content

#endif
