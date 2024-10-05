#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "Model.h"
#include "IFCInstance.h"
#include "Texture.h"
#include "IFCUnit.h"
#include "IFCProperty.h"
#include "Entity.h"
#include "IFCAttribute.h"

#include <string>
#include <map>
#include <set>

using namespace std;

class CIFCModel : public CModel
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
	
	vector<CIFCInstance*> m_vecInstances;
	map<SdaiInstance, CIFCInstance*> m_mapInstances;
	map<int64_t, CIFCInstance*> m_mapID2Instance;
	map<ExpressID, CIFCInstance*> m_mapExpressID2Instance;

	CIFCUnitProvider* m_pUnitProvider;
	CIFCPropertyProvider* m_pPropertyProvider;
	CEntityProvider* m_pEntityProvider;
	CIFCAttributeProvider* m_pAttributeProvider;

	static int_t s_iInstanceID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Members

	static uint32_t DEFAULT_COLOR;

public: // Methods
	
	CIFCModel(bool bLoadInstancesOnDemand = false);
	virtual ~CIFCModel();

	void PreLoadInstance(SdaiInstance iInstance);

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override { return m_pEntityProvider; }
	virtual CInstanceBase* GetInstanceByExpressID(ExpressID iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;

	void Scale(); // [-1, 1]

	void Load(const wchar_t* szIFCFile, SdaiModel iModel);
	virtual CInstanceBase* LoadInstance(OwlInstance iInstance) override;
	void Clean();

	const map<SdaiInstance, CIFCInstance*>& GetInstances() const { return m_mapInstances; }
	CIFCUnitProvider* GetUnitProvider() const { return m_pUnitProvider; }
	CIFCPropertyProvider* GetPropertyProvider() const { return m_pPropertyProvider; }
	CIFCAttributeProvider* GetAttributeProvider() const { return m_pAttributeProvider; }
	CIFCInstance* GetInstanceByID(int64_t iID);
	void GetInstancesByType(const wchar_t* szType, vector<CIFCInstance*>& vecInstances);

private: // Methods
	
	void GetObjectsReferencedState();	
	void GetObjectsReferencedStateIsDecomposedBy(SdaiInstance iInstance);
	void GetObjectsReferencedStateIsNestedBy(SdaiInstance iInstance);
	void GetObjectsReferencedStateContainsElements(SdaiInstance iInstance);
	void GetObjectsReferencedStateHasAssignments(SdaiInstance iInstance);
	void GetObjectsReferencedStateRecursively(SdaiInstance iInstance);

	void RetrieveObjectsRecursively(int_t iParentEntity, int_t iCircleSegments);
	void RetrieveObjects(const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	CIFCInstance* RetrieveGeometry(SdaiInstance iInstance, int_t iCircleSegments);
};

#endif // IFCFILEPARSER_H
