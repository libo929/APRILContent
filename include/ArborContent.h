  /// \file ArborContent.h
/*
 *
 * ArborContent.h header template automatically generated by a class generator
 * Creation date : ven. mars 20 2015
 *
 * This file is part of ArborContent libraries.
 * 
 * ArborContent is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * based upon these libraries are permitted. Any copy of these libraries
 * must include this copyright notice.
 * 
 * ArborContent is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with ArborContent.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * @author Remi Ete
 * @copyright CNRS , IPNL
 */

#ifndef ARBOR_CONTENT_H
#define ARBOR_CONTENT_H 1

// multi-threading option
#ifdef ARBOR_PARALLEL
#include <omp.h>
#endif

#include "Api/PandoraApi.h"
#include "Pandora/PandoraInternal.h"
#include "ArborApi/ArborContentApi.h"
#include "ArborApi/ObjectFactories.h"

#include "ArborCheating/PerfectParticleFlowAlgorithm.h"

#include "ArborClustering/ArborClusteringAlgorithm.h"
#include "ArborClustering/ClusteringParentAlgorithm.h"

#include "ArborHelpers/CaloHitHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/SortingHelper.h"

#include "ArborMonitoring/VisualMonitoringAlgorithm.h"
#include "ArborMonitoring/PerformanceMonitoringAlgorithm.h"

#include "ArborPfoConstruction/PfoCreationAlgorithm.h"

#include "ArborPlugins/ArborBFieldPlugin.h"
#include "ArborPlugins/ArborPseudoLayerPlugin.h"
#include "ArborPlugins/SdhcalQuadraticEnergyFunction.h"

#include "ArborReclustering/NeutralVicinityReclusteringAlgorithm.h"
#include "ArborReclustering/MissingEnergyReclusteringAlgorithm.h"

#include "ArborTools/ConnectorCleaningTool.h"
#include "ArborTools/ConnectorSeedingTool.h"
#include "ArborTools/EnergyEstimateTools.h"
#include "ArborTools/TrackDrivenSeedingTool.h"

#include "ArborTopologicalAssociation/TopologicalAssociationParentAlgorithm.h"
#include "ArborTopologicalAssociation/PointingClusterAssociationAlgorithm.h"
#include "ArborTopologicalAssociation/ClosebySeedMergingAlgorithm.h"
#include "ArborTopologicalAssociation/ClusterFragmentMergingAlgorithm.h"
#include "ArborTopologicalAssociation/ContactClusterMergingAlgorithm.h"
#include "ArborTopologicalAssociation/SurroundingHitsMergingAlgorithm.h"
#include "ArborTopologicalAssociation/CloudClusterRemovalAlgorithm.h"
#include "ArborTopologicalAssociation/ChargedClusterMergingAlgorithm.h"

#include "ArborTrackClusterAssociation/TrackClusterAssociationAlgorithm.h"
#include "ArborTrackClusterAssociation/TopologicalTrackClusterAssociationAlgorithm.h"

#include "ArborUtility/AlgorithmConfiguration.h"
#include "ArborUtility/EventPreparationAlgorithm.h"
#include "ArborUtility/ClusterPreparationAlgorithm.h"
#include "ArborUtility/TrackPreparationAlgorithm.h"
#include "ArborUtility/ListChangingAlgorithm.h"
#include "ArborUtility/ListMergingAlgorithm.h"
#include "ArborUtility/CompositeAlgorithm.h"

/** 
 * @brief  ArborContent class used to register arbor algorithms and plugins
 */ 
class ArborContent 
{
public:

#define ARBOR_ALGORITHM_LIST(d) \
	d("PerfectParticleFlow",                 arbor_content::PerfectParticleFlowAlgorithm::Factory) \
	d("ArborClustering",                     arbor_content::ArborClusteringAlgorithm::Factory) \
	d("ClusteringParent",                    arbor_content::ClusteringParentAlgorithm::Factory) \
	d("VisualMonitoring",                    arbor_content::VisualMonitoringAlgorithm::Factory) \
	d("PerformanceMonitoring",               arbor_content::PerformanceMonitoringAlgorithm::Factory) \
	d("PfoCreation",                         arbor_content::PfoCreationAlgorithm::Factory) \
	d("NeutralVicinityReclustering",         arbor_content::NeutralVicinityReclusteringAlgorithm::Factory) \
	d("MissingEnergyReclustering",           arbor_content::MissingEnergyReclusteringAlgorithm::Factory) \
	d("TopologicalAssociationParent",        arbor_content::TopologicalAssociationParentAlgorithm::Factory) \
	d("PointingClusterAssociation",          arbor_content::PointingClusterAssociationAlgorithm::Factory) \
	d("ClosebySeedMerging",                  arbor_content::ClosebySeedMergingAlgorithm::Factory) \
	d("ContactClusterMerging",               arbor_content::ContactClusterMergingAlgorithm::Factory) \
	d("ClusterFragmentMerging",              arbor_content::ClusterFragmentMergingAlgorithm::Factory) \
	d("SurroundingHitsMerging",              arbor_content::SurroundingHitsMergingAlgorithm::Factory) \
	d("CloudClusterRemoval",                 arbor_content::CloudClusterRemovalAlgorithm::Factory) \
	d("ChargedClusterMerging",               arbor_content::ChargedClusterMergingAlgorithm::Factory) \
	d("TopologicalTrackClusterAssociation",  arbor_content::TopologicalTrackClusterAssociationAlgorithm::Factory) \
	d("TrackClusterAssociation",             arbor_content::TrackClusterAssociationAlgorithm::Factory) \
	d("EventPreparation",                    arbor_content::EventPreparationAlgorithm::Factory) \
	d("ClusterPreparation",                  arbor_content::ClusterPreparationAlgorithm::Factory) \
	d("TrackPreparation",                    arbor_content::TrackPreparationAlgorithm::Factory) \
	d("CaloHitListMerging",                  arbor_content::InputObjectListMergingAlgorithm<pandora::CaloHitList>::Factory) \
	d("TrackListMerging",                    arbor_content::InputObjectListMergingAlgorithm<pandora::TrackList>::Factory) \
	d("MCParticleListMerging",               arbor_content::InputObjectListMergingAlgorithm<pandora::MCParticleList>::Factory) \
	d("PfoListMerging",                      arbor_content::AlgorithmObjectListMergingAlgorithm<pandora::ParticleFlowObject>::Factory) \
	d("ClusterListMerging",                  arbor_content::AlgorithmObjectListMergingAlgorithm<pandora::Cluster>::Factory) \
	d("VertexListMerging",                   arbor_content::AlgorithmObjectListMergingAlgorithm<pandora::Vertex>::Factory) \
	d("ListChanging",                        arbor_content::ListChangingAlgorithm::Factory) \
	d("CompositeAlgorithm",                  arbor_content::CompositeAlgorithm::Factory) \
	d("AlgorithmConfiguration",              arbor_content::AlgorithmConfiguration::Factory)


#define ARBOR_ALGORITHM_TOOL_LIST(d) \
	d("LinearInputEnergyEstimate",           arbor_content::LinearInputEnergyEstimate::Factory) \
	d("CombinedQuadraticEnergyEstimate",     arbor_content::CombinedQuadraticEnergyEstimate::Factory) \
	d("LinearEnergyEstimate",                arbor_content::LinearEnergyEstimate::Factory) \
	d("ConnectorCleaning",                   arbor_content::ConnectorCleaningTool::Factory) \
	d("ConnectorSeeding",                    arbor_content::ConnectorSeedingTool::Factory) \
	d("TrackDrivenSeeding",                  arbor_content::TrackDrivenSeedingTool::Factory)


 /**
  *  @brief  Register all the arbor algorithms with pandora
  *
  *  @param  pandora the pandora instance with which to register content
  */
 static pandora::StatusCode RegisterAlgorithms(const pandora::Pandora &pandora);

 /**
  *  @brief  Register pseudo layer plugin with pandora
  *
  *  @param  pandora the pandora instance with which to register content
  */
 static pandora::StatusCode RegisterArborPseudoLayerPlugin(const pandora::Pandora &pandora);

 /**
  *  @brief  Register the b field plugin (note user side configuration) with pandora
  *
  *  @param  pandora the pandora instance with which to register content
  *  @param  innerBField the bfield in the main tracker, ecal and hcal, units Tesla
  *  @param  muonBarrelBField the bfield in the muon barrel, units Tesla
  *  @param  muonEndCapBField the bfield in the muon endcap, units Tesla
  */
 static pandora::StatusCode RegisterBFieldPlugin(const pandora::Pandora &pandora, const float innerBField, const float muonBarrelBField,
     const float muonEndCapBField);

 /**
  *  @brief  Register the energy corrections with pandora
  *
  *  @param  pandora the pandora instance with which to register content
  */
 static pandora::StatusCode RegisterEnergyCorrections(const pandora::Pandora &pandora);
};

//------------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::StatusCode ArborContent::RegisterAlgorithms(const pandora::Pandora &pandora)
{
	ARBOR_ALGORITHM_LIST(PANDORA_REGISTER_ALGORITHM);
	ARBOR_ALGORITHM_TOOL_LIST(PANDORA_REGISTER_ALGORITHM_TOOL);

	return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::StatusCode ArborContent::RegisterArborPseudoLayerPlugin(const pandora::Pandora &pandora)
{
	return PandoraApi::SetPseudoLayerPlugin(pandora, new arbor_content::ArborPseudoLayerPlugin());
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::StatusCode ArborContent::RegisterBFieldPlugin(const pandora::Pandora &pandora, const float innerBField,
    const float muonBarrelBField, const float muonEndCapBField)
{
    return PandoraApi::SetBFieldPlugin(pandora, new arbor_content::ArborBFieldPlugin(innerBField, muonBarrelBField, muonEndCapBField));
}

//------------------------------------------------------------------------------------------------------------------------------------------

inline pandora::StatusCode ArborContent::RegisterEnergyCorrections(const pandora::Pandora &pandora)
{
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
			"SdhcalQuadraticEnergyFunction", pandora::HADRONIC, new arbor_content::SdhcalQuadraticEnergyFunction()));
	PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
			"SdhcalQuadraticEnergyFunction", pandora::ELECTROMAGNETIC, new arbor_content::SdhcalQuadraticEnergyFunction()));

	return pandora::STATUS_CODE_SUCCESS;
}


#endif  //  ARBOR_CONTENT_H
