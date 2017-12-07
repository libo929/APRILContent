#ifndef CLUSTERSHAPE_H
#define CLUSTERSHAPE_H 

#include <vector>
#include "Objects/CaloHit.h"
#include "Objects/Cluster.h"

#include "TVector3.h"

class TriPoint
{
public:
    TriPoint()
    {   
        X1.SetXYZ(10000., 10000., 10000.);
        X2.SetXYZ(10000., 10000., 10000.);
    }   

    TVector3 X0; 
    TVector3 X1; 
    TVector3 X2; 
};

class ClusterShape 
{
public:
	ClusterShape(const pandora::Cluster* pCluster);
	double CalcClusterShapeFactor();

private:
    const pandora::Cluster* m_cluster;

	std::vector<TriPoint> m_points;

	void FillHits();
};
	
#endif // CLUSTERSHAPE_H 
