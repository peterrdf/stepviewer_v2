#pragma once

#include "_ap_mvc.h"
#include "_ifc_unit.h"
#include "_ifc_property.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class _ifc_geometry;
class _ifc_instance;

// ************************************************************************************************
class _ifc_model : public _ap_model
{

private: // Members

	// Load
	bool m_bUseWorldCoordinates;
	bool m_bLoadInstancesOnDemand;

	// Entities
	SdaiEntity m_sdaiSpaceEntity;
	SdaiEntity m_sdaiOpeningElementEntity;
	SdaiEntity m_sdaiDistributionElementEntity;
	SdaiEntity m_sdaiElectricalElementEntity;
	SdaiEntity m_sdaiElementAssemblyEntity;
	SdaiEntity m_sdaiElementComponentEntity;
	SdaiEntity m_sdaiEquipmentElementEntity;
	SdaiEntity m_sdaiFeatureElementEntity;
	SdaiEntity m_sdaiFeatureElementSubtractionEntity;
	SdaiEntity m_sdaiFurnishingElementEntity;
	SdaiEntity m_sdaiReinforcingElementEntity;
	SdaiEntity m_sdaiTransportElementEntity;
	SdaiEntity m_sdaiVirtualElementEntity;

	_ifc_unit_provider* m_pUnitProvider;
	_ifc_property_provider* m_pPropertyProvider;	

public: // Methods

	_ifc_model(bool bUseWorldCoordinates = false, bool bLoadInstancesOnDemand = false);
	virtual ~_ifc_model();

protected: // Methods

	// _model
	virtual _instance* loadInstance(int64_t iInstance) override;
	virtual void clean(bool bCloseModel = true) override;

	// _ap_model
	virtual void attachModelCore() override;

protected: // Methods

	virtual _ifc_geometry* createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual _ifc_instance* createInstance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix);

private: // Methods

	void getObjectsReferencedState();
	void getObjectsReferencedStateIsDecomposedBy(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateIsNestedBy(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateContainsElements(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateHasAssignments(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateBoundedBy(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateHasOpenings(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateRecursively(SdaiInstance sdaiInstance);

	void retrieveGeometryRecursively(SdaiEntity sdaiParentEntity, SdaiInteger iCircleSegments);
	void retrieveGeometry(const char* szEntityName, SdaiInteger iCircleSegements);
	_geometry* loadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, SdaiInteger iCircleSegments);

public: // Properties

	_ifc_unit_provider* getUnitProvider();
	_ifc_property_provider* getPropertyProvider();
};


