#ifndef Geometry_TrackerNumberingBuilder_CmsTrackerPhase1DiskBuilder_H
# define Geometry_TrackerNumberingBuilder_CmsTrackerPhase1DiskBuilder_H

# include "Geometry/TrackerNumberingBuilder/plugins/CmsTrackerLevelBuilder.h"
# include "FWCore/ParameterSet/interface/types.h"
# include <string>

/**
 * Class which contructs PixelForward/Disk.
 */
class CmsTrackerPhase1DiskBuilder : public CmsTrackerLevelBuilder
{
  
private:
  void sortNS( DDFilteredView& , GeometricDet* ) override;
  void buildComponent( DDFilteredView& , GeometricDet*, std::string ) override;
  
  static bool PhiSort(const GeometricDet* Panel1, const GeometricDet* Panel2);

  void PhiPosNegSplit_innerOuter( std::vector< GeometricDet const *>::iterator begin,
				  std::vector< GeometricDet const *>::iterator end );
};

#endif
