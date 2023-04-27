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
	int_t m_ifcProjectEntity;
	int_t m_ifcSpaceEntity;
	int_t m_ifcOpeningElementEntity;
	int_t m_ifcDistributionElementEntity;
	int_t m_ifcElectricalElementEntity;
	int_t m_ifcElementAssemblyEntity;
	int_t m_ifcElementComponentEntity;
	int_t m_ifcEquipmentElementEntity;
	int_t m_ifcFeatureElementEntity;
	int_t m_ifcFeatureElementSubtractionEntity;
	int_t m_ifcFurnishingElementEntity;
	int_t m_ifcReinforcingElementEntity;
	int_t m_ifcTransportElementEntity;
	int_t m_ifcVirtualElementEntity;
	
	vector<CIFCInstance*> m_vecInstances;
	map<int_t, CIFCInstance*> m_mapInstances;  // C Instance : C++ Instance
	map<int_t, CIFCInstance*> m_mapID2Instance; // ID : Instance
	map<int64_t, CIFCInstance*> m_mapExpressID2Instance; // Express ID : Instance

	CIFCUnitProvider* m_pUnitProvider;
	CIFCPropertyProvider* m_pPropertyProvider;
	CEntityProvider* m_pEntityProvider;

	static int_t s_iInstanceID;

public: // Methods
	
	CIFCModel();
	virtual ~CIFCModel();

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual void ZoomToInstance(CInstance* pInstance) override;
	virtual void ZoomOut() override;

	void ScaleAndCenter(); // [-1, 1]

	void Load(const wchar_t* szIFCFile, int64_t iModel);
	void Clean();

	const map<int64_t, CIFCInstance*>& GetInstances() const;
	CIFCUnitProvider* GetUnitProvider() const;
	CIFCPropertyProvider* GetPropertyProvider() const;
	CIFCInstance* GetInstanceByID(int_t iID);	
	CIFCInstance* GetInstanceByExpressID(int64_t iExpressID);
	void GetInstancesByType(const wchar_t* szType, vector<CIFCInstance*>& vecInstances);

private: // Methods
	
	void RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth);
	void RetrieveObjects(const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	CIFCInstance* RetrieveGeometry(const wchar_t* szInstanceGUIDW, int_t iInstance, int_t iCircleSegments);
};

#endif // IFCFILEPARSER_H
