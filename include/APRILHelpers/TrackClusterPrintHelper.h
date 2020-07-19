/* TrackClusterPrintHelper.h
 *
 * Author: Bo Li (IPNL)
 * 
 */

#ifndef TRACKCLUSTERPRINTHELPER_H 
#define TRACKCLUSTERPRINTHELPER_H


#include "Pandora/Algorithm.h"
#include "Pandora/PandoraInternal.h"
#include "Pandora/StatusCodes.h"

#include "Objects/Cluster.h"

namespace pandora { class Algorithm; class CaloHit; class Track; }

namespace april_content
{
	class TrackClusterPrintHelper
	{   
		public: 
			static pandora::StatusCode PrintCluster(const pandora::ClusterList *pClusterList); 
			//static pandora::StatusCode PrintTrackCluster(const pandora::Pandora *const pPandora); 
	}; 
}

#endif  // TRACKCLUSTERPRINTHELPER_H
