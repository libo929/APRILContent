#include <stdio.h>

#include "APRILUtility/ClusterShape.h"

#include "TMath.h"
#include "TH1F.h"

using namespace std;

ClusterShape::ClusterShape(const pandora::Cluster* pCluster)
	      : m_cluster(pCluster)
{
	FillHits();
}

void ClusterShape::FillHits()
{
	m_points.clear();

	const pandora::OrderedCaloHitList& orderedCaloHitList = m_cluster->GetOrderedCaloHitList();
	pandora::CaloHitList caloHitList;

	orderedCaloHitList.FillCaloHitList(caloHitList);

    for(pandora::CaloHitList::iterator iter = caloHitList.begin(); iter != caloHitList.end(); ++iter)
    {
		const pandora::CaloHit* hit = *iter;
		if(hit==NULL) continue;

		const pandora::CartesianVector& hitVec = hit->GetPositionVector();

        TriPoint point;
        point.X0 = TVector3(hitVec.GetX(),hitVec.GetY(), hitVec.GetZ());

        m_points.push_back(point);
    }
}

double ClusterShape::CalcClusterShapeFactor()
{
    int numHits = m_points.size();

    ////// get and set the nearest hit distance
    for(int iHit = 0; iHit < numHits; ++iHit)
    {
        for(int jHit = 0; jHit < numHits; ++jHit)
        {
            if(iHit==jHit) continue;                                                                                         

            TriPoint& PointI = m_points.at(iHit);
            TriPoint& PointJ = m_points.at(jHit);

            TVector3 dist = PointJ.X0 - PointI.X0;
            //dist.Print();

            if(dist.Mag() < PointI.X1.Mag()) PointI.X1 = dist;
            if(dist.Mag() > PointI.X1.Mag() && dist.Mag() < PointI.X2.Mag()) PointI.X2 = dist;
        }
    }

    ////// print hit
    double meanHitDist = 0.;

    for(int iHit = 0; iHit < numHits; ++iHit)
    {
        TriPoint& PointI = m_points.at(iHit);
                                                                                                                             
        //TVector3& point0 = PointI.X0;                                                                                        
        TVector3& point1 = PointI.X1;                                                                                        
        //TVector3& point2 = PointI.X2;                                                                                        
                                                                                                                             
        meanHitDist += point1.Mag();                                                                                         
                                                                                                                             
        //point0.Print();                                                                                                    
        //point1.Print();                                                                                                    
        //point2.Print();                                                                                                    
        //cout << "----------" << endl;                                                                                      
    }                                                                                                                        
                                                                                                                             
    meanHitDist /= numHits;                                                                                                  
                                                                                                                             
    //cout << "mean distance: " << meanHitDist << endl;                                                                      
                                                                                                                             
    TH1F h("hist", "hist", 100, -1000, 1000);                                                                                
                                                                                                                             
    for(int iHit = 0; iHit < numHits; ++iHit)                                                                                
    {                                                                                                                        
        TriPoint& PointI = m_points.at(iHit);                                                                                     
                                                                                                                             
        double d1 = PointI.X1.Mag();                                                                                         
        double d2 = PointI.X2.Mag();                                                                                         
                                                                                                                             
        double angle1 = PointI.X1.Angle(PointI.X2);                                                                          
        double angle2 = TMath::Pi() - PointI.X1.Angle(PointI.X2);                                                            
        double angle = min(angle1, angle2);                                                                                  
        //cout << "--> angle: " << angle << endl;                                                                            
                                                                                                                             
        double coeff = angle * TMath::Exp(-(d1-d2)*(d1-d2)/(meanHitDist*meanHitDist));
                                                                                                                             
        h.Fill(coeff);                                                                                                       
        //cout << "-----coeff: " << coeff << endl;                                                                           
    }
                                                                                                                             
    return h.GetMean();
}
