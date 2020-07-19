/**
 *  @file   APRILContent/include/APRILCheating/CheatingClusterCleaningAlgorithm.h
 * 
 *  @brief  Header file for the cheating cluster cleaning algorithm class.
 * 
 *  $Log: $
 */
#ifndef APRIL_CHEATING_CLUSTER_CLEANING_ALGORITHM_H
#define APRIL_CHEATING_CLUSTER_CLEANING_ALGORITHM_H 1

#include "Pandora/Algorithm.h"

namespace april_content
{

/**
 *  @brief CheatingClusterCleaningAlgorithm class
 */
class CheatingClusterCleaningAlgorithm : public pandora::Algorithm
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

    bool m_isOnlyCleanPhoton;
};

inline pandora::Algorithm *CheatingClusterCleaningAlgorithm::Factory::CreateAlgorithm() const
{
    return new CheatingClusterCleaningAlgorithm();
}

} // namespace APRIL_content

#endif // #ifndef APRIL_CHEATING_CLUSTER_CLEANING_ALGORITHM_H
