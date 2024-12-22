#pragma once

#include "_ap_mvc.h"

#include "IFCUnit.h"
#include "IFCProperty.h"
#include "IFCAttribute.h"

#include <string>
#include <map>
#include <set>
using namespace std;

// ************************************************************************************************
class _ifc_geometry;

// ************************************************************************************************
class _ifc_model : public _ap_model
{

private: // Members

	// Load
	bool m_bLoadInstancesOnDemand;

	// Entities
	SdaiEntity m_ifcSpaceEntity;
	SdaiEntity m_ifcOpeningElementEntity;
	SdaiEntity m_ifcDistributionElementEntity;
	SdaiEntity m_ifcElectricalElementEntity;
	SdaiEntity m_ifcElementAssemblyEntity;
	SdaiEntity m_ifcElementComponentEntity;
	SdaiEntity m_ifcEquipmentElementEntity;
	SdaiEntity m_ifcFeatureElementEntity;
	SdaiEntity m_ifcFeatureElementSubtractionEntity;
	SdaiEntity m_ifcFurnishingElementEntity;
	SdaiEntity m_ifcReinforcingElementEntity;
	SdaiEntity m_ifcTransportElementEntity;
	SdaiEntity m_ifcVirtualElementEntity;

	CIFCUnitProvider* m_pUnitProvider;
	CIFCPropertyProvider* m_pPropertyProvider;
	CIFCAttributeProvider* m_pAttributeProvider;

public: // Methods

	_ifc_model(bool bLoadInstancesOnDemand = false);
	virtual ~_ifc_model();

protected: // Methods

	// _ap_model
	virtual void attachModelCore() override;
	virtual void clean() override;

public: // Methods

	CIFCUnitProvider* GetUnitProvider() const { return m_pUnitProvider; }
	CIFCPropertyProvider* GetPropertyProvider() const { return m_pPropertyProvider; }
	CIFCAttributeProvider* GetAttributeProvider() const { return m_pAttributeProvider; }

protected: // Methods

	virtual _ifc_geometry* createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual _ap_instance* createInstance(int64_t iID, _geometry* pGeometry, _matrix4x3* pTransformationMatrix);

private: // Methods

	void GetObjectsReferencedState();
	void GetObjectsReferencedStateIsDecomposedBy(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateIsNestedBy(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateContainsElements(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateHasAssignments(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateRecursively(SdaiInstance sdaiInstance);

	void RetrieveGeometryRecursively(int_t iParentEntity, int_t iCircleSegments);
	void RetrieveGeometry(const char* szEntityName, int_t iCircleSegements);
	_geometry* LoadGeometry(const char* szEntityName, SdaiInstance sdaiInstance, int_t iCircleSegments);
};

