#pragma once

#include "_ap_model.h"
#include "_unit.h"
#include "_ifc_property.h"
#include "_ifc_attribute.h"

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

	CIFCUnitProvider* m_pUnitProvider;
	_ifc_property_provider* m_pPropertyProvider;
	_ifc_attribute_provider* m_pAttributeProvider;

public: // Methods

	_ifc_model(bool bLoadInstancesOnDemand = false);
	virtual ~_ifc_model();

protected: // Methods

	// _ap_model
	virtual void attachModelCore() override;
	virtual void clean() override;

protected: // Methods

	virtual _ifc_geometry* createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual _ifc_instance* createInstance(int64_t iID, _ifc_geometry* pGeometry, _matrix4x3* pTransformationMatrix);

private: // Methods

	void getObjectsReferencedState();
	void getObjectsReferencedStateIsDecomposedBy(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateIsNestedBy(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateContainsElements(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateHasAssignments(SdaiInstance sdaiInstance);
	void getObjectsReferencedStateRecursively(SdaiInstance sdaiInstance);

	void retrieveGeometryRecursively(SdaiEntity sdaiParentEntity, SdaiInteger iCircleSegments);
	void retrieveGeometry(const char* szEntityName, SdaiInteger iCircleSegements);
	_geometry* loadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, SdaiInteger iCircleSegments);

public: // Properties

	CIFCUnitProvider* getUnitProvider();
	_ifc_property_provider* getPropertyProvider();
	_ifc_attribute_provider* getAttributeProvider();
};


