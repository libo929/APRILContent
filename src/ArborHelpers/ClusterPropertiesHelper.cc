#include "ArborHelpers/ClusterPropertiesHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborObjects/CaloHit.h"

#include "TMatrixDSymEigen.h"

pandora::StatusCode arbor_content::ClusterPropertiesHelper::CalcClusterProperties(const pandora::Cluster* pCluster, 
		 float& minHitLayer, float& clusterVol, float& energyRatio, 
		 float& hitOutsideRatio, float& axisLengthRatio, float& shortAxisLengthRatio, TVector3& axis)
{
    pandora::CartesianVector centroid(0.f, 0.f, 0.f);
    PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, ClusterHelper::GetCentroid(pCluster, centroid));

    clusterVol = 0.;

    float clusterEnergy(0.);
	float hcalEnergy(0.);

    pandora::CaloHitList clusterCaloHitList;
    pCluster->GetOrderedCaloHitList().GetCaloHitList(clusterCaloHitList);

	std::vector<TVector3> relativePositionVector;
	std::vector<float> hitWeightVector;

	minHitLayer = std::numeric_limits<float>::max();

    for(pandora::CaloHitList::const_iterator iter = clusterCaloHitList.begin(), endIter = clusterCaloHitList.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::CaloHit *const pCaloHit(*iter);

	  //float hitDepth = pCaloHit->GetNCellRadiationLengths();
	  float hitDepth = pCaloHit->GetPseudoLayer();

	  pandora::CartesianVector hitCellNormal = pCaloHit->GetCellNormalVector();
	  pandora::CartesianVector hitExpectedDirection = pCaloHit->GetExpectedDirection();
	  float cosOpeningAngle = hitExpectedDirection.GetCosOpeningAngle(hitCellNormal);
	  
	  // correct the hit depth by angle
	  hitDepth = hitDepth/cosOpeningAngle;

	  //////////// -------->
	  if(hitDepth < minHitLayer) minHitLayer = hitDepth;

      float hitEnergy(0.);

      if(pCaloHit->GetHitType() == pandora::ECAL)
      {
        hitEnergy = pCaloHit->GetElectromagneticEnergy();
      }
	  else
      {
        hitEnergy = pCaloHit->GetHadronicEnergy();
		hcalEnergy += hitEnergy;
      }

	  clusterEnergy += hitEnergy;

	  pandora::CartesianVector hitPosition = pCaloHit->GetPositionVector();
	  pandora::CartesianVector relativePosition = hitPosition - centroid;

      float deltaVol = hitEnergy * relativePosition.GetMagnitude();
      clusterVol += deltaVol;

	  //////
	  relativePositionVector.push_back( TVector3(relativePosition.GetX(), relativePosition.GetY(), relativePosition.GetZ()) );
	  hitWeightVector.push_back(hitEnergy);
    }

	//////////// -------->
	energyRatio = hcalEnergy/clusterEnergy;
	
	float weightSum = clusterEnergy;

	//////////// -------->
	// FIXME:: clusterEnergy should not be zero
    clusterVol = clusterVol/clusterEnergy;  

	TVector3 cluCentroid(centroid.GetX(), centroid.GetY(), centroid.GetZ());

	TVector3 clusterMainAxis;

	////////
	GetAxisInformation(relativePositionVector, hitWeightVector, weightSum, 
			           clusterMainAxis, axisLengthRatio, shortAxisLengthRatio);

	///////
	int outsideHit(0);

	// the ratio of hit outside 2 * Moliere radius
    for(pandora::CaloHitList::const_iterator iter = clusterCaloHitList.begin(), endIter = clusterCaloHitList.end() ;
        endIter != iter ; ++iter)
    {
      const pandora::CaloHit *const pCaloHit(*iter); 

	  pandora::CartesianVector pos = pCaloHit->GetPositionVector();
	  TVector3 hitPosition(pos.GetX(), pos.GetY(), pos.GetZ());
	  float hitAxisDistance = ((hitPosition - cluCentroid).Cross(clusterMainAxis)).Mag(); 
	  
	  // the hit outside 2 * Moliere radius
	  if(hitAxisDistance > 38.) ++outsideHit;
	}

	int nHits(pCluster->GetNCaloHits());

	//////////// -------->
	hitOutsideRatio = float(outsideHit)/nHits;
	axis = clusterMainAxis;
	
	return pandora::STATUS_CODE_SUCCESS;
}

pandora::StatusCode arbor_content::ClusterPropertiesHelper::GetClusterProperties(const pandora::Cluster* pCluster, 
		 float& minHitLayer, float& clusterVol, float& energyRatio, 
		 float& hitOutsideRatio, float& axisLengthRatio, float& shortAxisLengthRatio)
{
	TVector3 axis;

    return CalcClusterProperties(pCluster, minHitLayer, clusterVol, energyRatio, hitOutsideRatio, axisLengthRatio, shortAxisLengthRatio, 
			                     axis); // axis is local
}

pandora::StatusCode arbor_content::ClusterPropertiesHelper::GetAxisInformation(
	std::vector<TVector3>& relativePositionVector, std::vector<float>& hitWeightVector, float weightSum, 
	TVector3& clusterMainAxis, float& axisLengthRatio, float& shortAxisLengthRatio)
{
    TMatrixDSym m(3);

	float Cnum(0.);

    for(int i=0; i<relativePositionVector.size(); ++i) 
	{
        m(0,0) += hitWeightVector[i] * (relativePositionVector[i].Mag2() - relativePositionVector[i].X()*relativePositionVector[i].X());
        m(0,1) += hitWeightVector[i] * (                                 - relativePositionVector[i].X()*relativePositionVector[i].Y());
        m(0,2) += hitWeightVector[i] * (                                 - relativePositionVector[i].X()*relativePositionVector[i].Z());

        m(1,1) += hitWeightVector[i] * (relativePositionVector[i].Mag2() - relativePositionVector[i].Y()*relativePositionVector[i].Y());
        m(1,2) += hitWeightVector[i] * (                                 - relativePositionVector[i].Y()*relativePositionVector[i].Z());

        m(2,2) += hitWeightVector[i] * (relativePositionVector[i].Mag2() - relativePositionVector[i].Z()*relativePositionVector[i].Z());

		Cnum += hitWeightVector[i] * relativePositionVector[i].Mag2();
    }

    m(1,0) = m(0,1);
    m(2,0) = m(0,2);
    m(2,1) = m(1,2);

    //m.Print();

    TMatrixDSymEigen matrix(m);

    //matrix.GetEigenValues().Print();
    //matrix.GetEigenVectors().Print();

	//const TVectorD& eigenValues  = matrix.GetEigenValues();
	const TMatrixD& eigenVectors = matrix.GetEigenVectors();

	// FIXME:: seems not used ...
	Cnum /= weightSum;

	////// the cluster Axises
	TVector3 cluAxis[3];

	cluAxis[0] = TVector3(eigenVectors[0][0], eigenVectors[1][0], eigenVectors[2][0]);
	cluAxis[1] = TVector3(eigenVectors[0][1], eigenVectors[1][1], eigenVectors[2][1]);
	cluAxis[2] = TVector3(eigenVectors[0][2], eigenVectors[1][2], eigenVectors[2][2]);

	float axisLength[3] = {0., 0., 0.};
	
	/////
	float maxLength(0.);
	int    maxIndex(-1);

	for(int iAxis=0; iAxis<3; ++iAxis) 
	{
		for(int iHit=0; iHit<relativePositionVector.size(); ++iHit) 
		{ 
			float pv = relativePositionVector[iHit] * cluAxis[iAxis];
			axisLength[iAxis] += hitWeightVector[iHit] * pv * pv;
		}

		axisLength[iAxis] /= weightSum;

		axisLength[iAxis] = sqrt(axisLength[iAxis]);

		if(maxLength<axisLength[iAxis]) {
			maxLength = axisLength[iAxis]; 
			maxIndex = iAxis;
		}
	
		//cout << "*******cluster axis lenght: " << axisLength[iAxis] << endl;
	}

	//cout << "-------------------" << endl;
	// pick up the two short axises
	int shortIndex1 = abs(1 - maxIndex);
	int shortIndex2 = 3 - shortIndex1 - maxIndex;

	//float meanLength = (axisLength[0] + axisLength[1] + axisLength[2] - axisLength[maxLength])/2; 
	float meanLength = (axisLength[shortIndex1] + axisLength[shortIndex2])/2; 
	axisLengthRatio = meanLength/maxLength;

	TVector3 axis(eigenVectors[0][maxIndex], eigenVectors[1][maxIndex], eigenVectors[2][maxIndex]);
	clusterMainAxis = axis;

	if(axisLength[shortIndex1] > axisLength[shortIndex2]) 
		shortAxisLengthRatio = axisLength[shortIndex2]/axisLength[shortIndex1];
	else
		shortAxisLengthRatio = axisLength[shortIndex1]/axisLength[shortIndex2];

	return pandora::STATUS_CODE_SUCCESS;
}
