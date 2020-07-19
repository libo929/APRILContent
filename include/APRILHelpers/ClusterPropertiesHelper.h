/* ClusterPropertiesHelper.h
 *
 * Author: Bo Li (IPNL)
 * 
 */

#ifndef CLUSTERPROPERTIESHELPER_H
#define CLUSTERPROPERTIESHELPER_H

#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"
#include "Objects/Cluster.h"
#include "TVector3.h"

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace april_content
{
	class ClusterPropertiesHelper
	{
		public:
            static pandora::StatusCode GetClusterProperties(const pandora::Cluster* pCluster, 
		          float& minHitLayer, float& clusterVol, float& energyRatio, 
		          float& hitOutsideRatio, float& axisLengthRatio, float& shortAxisLengthRatio);

			static pandora::StatusCode GetAxisInformation(std::vector<TVector3>& relativePositionVector, 
				  std::vector<float>& hitWeightVector, float weightSum, 
	              TVector3& clusterMainAxis, float& axisLengthRatio, float& shortAxisLengthRatio);

			static pandora::StatusCode CalcClusterProperties(const pandora::Cluster* pCluster, 
				  float& minHitLayer, float& clusterVol, float& energyRatio, 
				  float& hitOutsideRatio, float& axisLengthRatio, float& shortAxisLengthRatio, TVector3& axis);

			static TVector3 GetMainAxis(const pandora::Cluster* const pCluster);
	};
}

#endif  // CLUSTERPROPERTIESHELPER_H
