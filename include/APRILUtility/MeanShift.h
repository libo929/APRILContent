#ifndef MEANSHIFT_H
#define MEANSHIFT_H

#include <vector>

#include "Objects/CaloHit.h"

//typedef std::vector<double> MSPoint;

class MSPoint : public std::vector<double>
{
public:
	MSPoint(const pandora::CaloHit* pCaloHit = NULL) 
	{
		m_caloHit = pCaloHit;
	}

	const pandora::CaloHit* m_caloHit;
};

struct MSCluster 
{
    MSPoint mode;
    std::vector<MSPoint> original_points;
    std::vector<MSPoint> shifted_points;
};

class MeanShift 
{
public:
    MeanShift(double kernelBandwidth = 0.1, double clusterEpsilon = 0.5, double shiftEpsilon = 0.00001);

    std::vector<MSCluster> cluster(const std::vector<MSPoint>&);

private:
    void shift_point(const MSPoint&, const std::vector<MSPoint>&, double, MSPoint&);
    std::vector<MSPoint> meanshift(const std::vector<MSPoint>& points, double kernel_bandwidth, double shiftEpsilon);
    std::vector<MSCluster> cluster(const std::vector<MSPoint>&, const std::vector<MSPoint>&, double clusterEpsilon);

    double m_kernelBandwidth;
	double m_clusterEpsilon;
	double m_shiftEpsilon;
};

#endif // MEANSHIFT_H
