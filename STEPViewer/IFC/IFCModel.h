#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "_ap_mvc.h"
#include "Entity.h"

#include "IFCGeometry.h"
#include "IFCInstance.h"
#include "IFCUnit.h"
#include "IFCProperty.h"
#include "IFCAttribute.h"

#include <string>
#include <map>
#include <set>

using namespace std;

class CIFCModel : public _ap_model
{

private: // Classes

	friend class CIFCController;

private: // Members

	// Load
	bool m_bLoadInstancesOnDemand;

	// Entities
	SdaiEntity m_ifcProjectEntity;
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

	static int_t s_iInstanceID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Members

	static uint32_t DEFAULT_COLOR;

public: // Methods
	
	CIFCModel(bool bLoadInstancesOnDemand = false);
	virtual ~CIFCModel();

	// _model
	virtual void ZoomToInstance(_instance* pInstance) override;
	virtual void ZoomOut() override;
	virtual _instance* LoadInstance(OwlInstance iInstance) override;

protected: // Methods

	// _ap_model
	virtual void attachModelCore() override;
	virtual void clean() override;

public: // Methods

	void Scale(); // [-1, 1]

	CIFCUnitProvider* GetUnitProvider() const { return m_pUnitProvider; }
	CIFCPropertyProvider* GetPropertyProvider() const { return m_pPropertyProvider; }
	CIFCAttributeProvider* GetAttributeProvider() const { return m_pAttributeProvider; }
	void GetInstancesByType(const wchar_t* szType, vector<_ap_instance*>& vecInstances);

private: // Methods
	
	void GetObjectsReferencedState();	
	void GetObjectsReferencedStateIsDecomposedBy(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateIsNestedBy(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateContainsElements(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateHasAssignments(SdaiInstance sdaiInstance);
	void GetObjectsReferencedStateRecursively(SdaiInstance sdaiInstance);

	void RetrieveObjectsRecursively(int_t iParentEntity, int_t iCircleSegments);
	void RetrieveObjects(const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	_geometry* LoadGeometry(const wchar_t* szEntityName, SdaiInstance sdaiInstance, int_t iCircleSegments);
};

#endif // IFCFILEPARSER_H
