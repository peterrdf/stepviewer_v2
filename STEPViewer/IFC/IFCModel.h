#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "Model.h"
#include "IFCInstance.h"
#include "Texture.h"
#include "IFCUnit.h"
#include "IFCProperty.h"
#include "Entity.h"

#include <string>
#include <map>
#include <set>

using namespace std;

// ------------------------------------------------------------------------------------------------
// Parser for IFC files
class CIFCModel : public CModel
{

private: // Classes

	friend class CIFCController;

private: // Members

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
	map<SdaiInstance, CIFCInstance*> m_mapInstances;  // C Instance : C++ Instance
	map<int_t, CIFCInstance*> m_mapID2Instance; // ID : Instance
	map<int64_t, CIFCInstance*> m_mapExpressID2Instance; // Express ID : Instance

	CIFCUnitProvider* m_pUnitProvider;
	CIFCPropertyProvider* m_pPropertyProvider;
	CEntityProvider* m_pEntityProvider;

	static int_t s_iInstanceID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Methods
	
	CIFCModel();
	virtual ~CIFCModel();

	void PreLoadInstance(SdaiInstance iInstance);

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;

	void ScaleAndCenter(); // [-1, 1]

	void Load(const wchar_t* szIFCFile, int64_t iModel);
	void Clean();

	const map<SdaiInstance, CIFCInstance*>& GetInstances() const;
	CIFCUnitProvider* GetUnitProvider() const;
	CIFCPropertyProvider* GetPropertyProvider() const;
	CIFCInstance* GetInstanceByID(int_t iID);	
	void GetInstancesByType(const wchar_t* szType, vector<CIFCInstance*>& vecInstances);

private: // Methods
	
	void RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth);
	void RetrieveObjects(const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	CIFCInstance* RetrieveGeometry(const wchar_t* szInstanceGUIDW, SdaiInstance iInstance, int_t iCircleSegments);
};

#endif // IFCFILEPARSER_H
