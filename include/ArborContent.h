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

#include "ArborCheating/CheatingTrackToClusterMatching.h"
#include "ArborCheating/PerfectParticleFlowAlgorithm.h"
#include "ArborCheating/PerfectClusteringAlgorithm.h"
#include "ArborCheating/PerfectFragmentRemovalAlgorithm.h"
#include "ArborCheating/PerfectIsoHitRemovalAlgorithm.h"
#include "ArborCheating/PerfectNeutralHitRecoveryAlgorithm.h"

#include "ArborClustering/ArborClusteringAlgorithm.h"
#include "ArborClustering/ClusteringParentAlgorithm.h"
#include "ArborClustering/SimpleRegionClusteringAlgorithm.h"

#include "ArborHelpers/CaloHitHelper.h"
#include "ArborHelpers/ClusterHelper.h"
#include "ArborHelpers/ClusterPropertiesHelper.h"
#include "ArborHelpers/GeometryHelper.h"
#include "ArborHelpers/ReclusterHelper.h"
#include "ArborHelpers/SortingHelper.h"
#include "ArborHelpers/BDTBasedClusterIdHelper.h"
#include "ArborHelpers/ClusterPropertiesFillingHelper.h"
#include "ArborHelpers/ClusterTrackEnergyFillingHelper.h"

#include "ArborMonitoring/VisualMonitoringAlgorithm.h"
#include "ArborMonitoring/PerformanceMonitoringAlgorithm.h"
#include "ArborMonitoring/SingleParticleMonitoringAlgorithm.h"

#include "ArborPfoConstruction/PfoCreationAlgorithm.h"

#include "ArborParticleId/FinalParticleIdAlgorithm.h"
#include "ArborParticleId/PhotonReconstructionAlgorithm.h"
#include "ArborParticleId/SingleClusterIdAlgorithm.h"

#include "ArborPlugins/ArborBFieldPlugin.h"
#include "ArborPlugins/ArborPseudoLayerPlugin.h"
#include "ArborPlugins/EnergyCorrectionPlugins.h"
#include "ArborPlugins/ParticleIdPlugins.h"

#include "ArborReclustering/EnergyExcessReclusteringAlgorithm.h"
#include "ArborReclustering/MissingEnergyReclusteringAlgorithm.h"
#include "ArborReclustering/SplitTrackReclusteringAlgorithm.h"

#include "ArborTools/ConnectorCleaningTool.h"
#include "ArborTools/ConnectorSeedingTool.h"
#include "ArborTools/ConnectorAlignmentTool.h"
#include "ArborTools/TrackDrivenSeedingTool.h"
#include "ArborTools/CaloHitMergingTool.h"

#include "ArborTopologicalAssociation/TopologicalAssociationParentAlgorithm.h"
#include "ArborTopologicalAssociation/PointingClusterAssociationAlgorithm.h"
#include "ArborTopologicalAssociation/ClosebySeedMergingAlgorithm.h"
#include "ArborTopologicalAssociation/ClusterFragmentMergingAlgorithm.h"
#include "ArborTopologicalAssociation/SurroundingHitsMergingAlgorithm.h"
#include "ArborTopologicalAssociation/NearbyTrackPhotonRemovalAlg.h"
#include "ArborTopologicalAssociation/MipFragmentMergingAlg.h"
#include "ArborTopologicalAssociation/FragmentRemovalAlgorithm.h"

#include "ArborTrackClusterAssociation/TrackClusterAssociationAlgorithm.h"
#include "ArborTrackClusterAssociation/UnassociatedTrackRecoveryAlg.h"
#include "ArborTrackClusterAssociation/LoopingTrackAssociationAlgorithm.h"

#include "ArborUtility/AlgorithmConfiguration.h"
#include "ArborUtility/EventPreparationAlgorithm.h"
#include "ArborUtility/ClusterPreparationAlgorithm.h"
#include "ArborUtility/TrackPreparationAlgorithm.h"
#include "ArborUtility/ListChangingAlgorithm.h"
#include "ArborUtility/ListMergingAlgorithm.h"
#include "ArborUtility/CaloHitPreparationAlgorithm.h"
#include "ArborUtility/CaloHitTimingAlgorithm.h"
#include "ArborUtility/ClusterCheckAlgorithm.h"
#include "ArborUtility/IsolatedHitClusteringAlgorithm.h"

/** 
 * @brief  ArborContent class used to register arbor algorithms and plugins
 */ 
class ArborContent 
{
public:

#define ARBOR_ALGORITHM_LIST(d) \
    d("PerfectParticleFlow",                 arbor_content::PerfectParticleFlowAlgorithm::Factory) \
    d("PerfectClustering",                   arbor_content::PerfectClusteringAlgorithm::Factory) \
    d("PerfectFragmentRemoval",              arbor_content::PerfectFragmentRemovalAlgorithm::Factory) \
    d("PerfectIsoHitRemoval",                arbor_content::PerfectIsoHitRemovalAlgorithm::Factory) \
    d("PerfectNeutralHitRecovery",           arbor_content::PerfectNeutralHitRecoveryAlgorithm::Factory) \
    d("CheatingTrackToClusterMatching",      arbor_content::CheatingTrackToClusterMatching::Factory) \
    d("ArborClustering",                     arbor_content::ArborClusteringAlgorithm::Factory) \
    d("ClusteringParent",                    arbor_content::ClusteringParentAlgorithm::Factory) \
    d("SimpleRegionClustering",              arbor_content::SimpleRegionClusteringAlgorithm::Factory) \
    d("VisualMonitoring",                    arbor_content::VisualMonitoringAlgorithm::Factory) \
    d("PerformanceMonitoring",               arbor_content::PerformanceMonitoringAlgorithm::Factory) \
    d("SingleParticleMonitoring",            arbor_content::SingleParticleMonitoringAlgorithm::Factory) \
    d("FinalParticleId",                     arbor_content::FinalParticleIdAlgorithm::Factory) \
    d("PhotonReconstruction",                arbor_content::PhotonReconstructionAlgorithm::Factory) \
    d("SingleClusterId",                     arbor_content::SingleClusterIdAlgorithm::Factory) \
    d("PfoCreation",                         arbor_content::PfoCreationAlgorithm::Factory) \
    d("EnergyExcessReclustering",            arbor_content::EnergyExcessReclusteringAlgorithm::Factory) \
    d("MissingEnergyReclustering",           arbor_content::MissingEnergyReclusteringAlgorithm::Factory) \
    d("SplitTrackReclustering",              arbor_content::SplitTrackReclusteringAlgorithm::Factory) \
    d("TopologicalAssociationParent",        arbor_content::TopologicalAssociationParentAlgorithm::Factory) \
    d("PointingClusterAssociation",          arbor_content::PointingClusterAssociationAlgorithm::Factory) \
    d("ClosebySeedMerging",                  arbor_content::ClosebySeedMergingAlgorithm::Factory) \
    d("ClusterFragmentMerging",              arbor_content::ClusterFragmentMergingAlgorithm::Factory) \
    d("SurroundingHitsMerging",              arbor_content::SurroundingHitsMergingAlgorithm::Factory) \
    d("NearbyTrackPhotonRemoval",            arbor_content::NearbyTrackPhotonRemovalAlg::Factory) \
    d("MipFragmentMerging",                  arbor_content::MipFragmentMergingAlg::Factory) \
    d("FragmentRemoval",                     arbor_content::FragmentRemovalAlgorithm::Factory) \
    d("TrackClusterAssociation",             arbor_content::TrackClusterAssociationAlgorithm::Factory) \
    d("UnassociatedTrackRecovery",           arbor_content::UnassociatedTrackRecoveryAlg::Factory) \
    d("LoopingTrackAssociation",             arbor_content::LoopingTrackAssociationAlgorithm::Factory) \
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
    d("AlgorithmConfiguration",              arbor_content::AlgorithmConfiguration::Factory) \
    d("CaloHitPreparation",                  arbor_content::CaloHitPreparationAlgorithm::Factory) \
    d("CaloHitTiming",                       arbor_content::CaloHitTimingAlgorithm::Factory) \
    d("ClusterCheck",                        arbor_content::ClusterCheckAlgorithm::Factory) \
    d("IsolatedHitClustering",               arbor_content::IsolatedHitClusteringAlgorithm::Factory)


#define ARBOR_ALGORITHM_TOOL_LIST(d) \
    d("ConnectorCleaning",                   arbor_content::ConnectorCleaningTool::Factory) \
    d("ConnectorSeeding",                    arbor_content::ConnectorSeedingTool::Factory) \
    d("TrackDrivenSeeding",                  arbor_content::TrackDrivenSeedingTool::Factory) \
    d("ConnectorAlignment",                  arbor_content::ConnectorAlignmentTool::Factory) \
    d("CaloHitMerging",                      arbor_content::CaloHitMergingTool::Factory)

#define ARBOR_PARTICLE_ID_LIST(d) \
    d("ArborEmShowerId",                     arbor_content::ArborEmShowerId) \
    d("ArborPhotonId",                       arbor_content::ArborPhotonId) \
    d("ArborElectronId",                     arbor_content::ArborElectronId) \
    d("ArborMuonId",                         arbor_content::ArborMuonId)


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

  /**
   *  @brief  Register arbor particle id functions
   *
   *  @param  pandora the pandora instance with which to register content
   */
  static pandora::StatusCode RegisterParticleIds(const pandora::Pandora &pandora);
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

  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
      "AnalogicEnergyFunction", pandora::HADRONIC, new arbor_content::AnalogicEnergyFunction()));

  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
      "AnalogicEnergyFunction", pandora::ELECTROMAGNETIC, new arbor_content::AnalogicEnergyFunction()));

  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
      "BarrelGapEnergyFunction", pandora::HADRONIC, new arbor_content::BarrelGapEnergyFunction()));

  PANDORA_RETURN_RESULT_IF(pandora::STATUS_CODE_SUCCESS, !=, PandoraApi::RegisterEnergyCorrectionPlugin(pandora,
      "ThetaEnergyFunction", pandora::HADRONIC, new arbor_content::ThetaEnergyFunction()));

  return pandora::STATUS_CODE_SUCCESS;
}

//------------------------------------------------------------------------------------------------------------------------------------------

pandora::StatusCode ArborContent::RegisterParticleIds(const pandora::Pandora &pandora)
{
  ARBOR_PARTICLE_ID_LIST(PANDORA_REGISTER_PARTICLE_ID);

  return pandora::STATUS_CODE_SUCCESS;
}


#endif  //  ARBOR_CONTENT_H
