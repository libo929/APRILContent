  /// \file ArborBFieldPlugin.h
/*
 *
 * ArborBFieldPlugin.h header template automatically generated by a class generator
 * Creation date : sam. mars 21 2015
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
 * @author John Marshall
 */

#ifndef ARBORBFIELDPLUGIN_H
#define ARBORBFIELDPLUGIN_H

#include "Plugins/BFieldPlugin.h"

namespace arbor_content
{

/**
 *  @brief  ArborBFieldPlugin class
 */
class ArborBFieldPlugin : public pandora::BFieldPlugin
{
public:
    /**
     *  @brief  Default constructor
     * 
     *  @param  innerBField the bfield in the main tracker, ecal and hcal, units Tesla
     *  @param  muonBarrelBField the bfield in the muon barrel, units Tesla
     *  @param  muonEndCapBField the bfield in the muon endcap, units Tesla
     */
	ArborBFieldPlugin(const float innerBField, const float muonBarrelBField, const float muonEndCapBField);

    float GetBField(const pandora::CartesianVector &positionVector) const;

private:
    pandora::StatusCode Initialize();
    pandora::StatusCode ReadSettings(const pandora::TiXmlHandle xmlHandle);

    float   m_innerBField;              ///< The bfield in the main tracker, ecal and hcal, units Tesla
    float   m_muonBarrelBField;         ///< The bfield in the muon barrel, units Tesla
    float   m_muonEndCapBField;         ///< The bfield in the muon endcap, units Tesla

    float   m_muonEndCapInnerZ;         ///< The muon endcap inner z coordinate, units mm
    float   m_coilMidPointR;            ///< The r coordinate at the coil midpoint, units mm
};

}

#endif  //  ARBORBFIELDPLUGIN_H
