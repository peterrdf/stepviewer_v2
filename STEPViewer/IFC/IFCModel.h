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
	// --------------------------------------------------------------------------------------------
	friend class CIFCController;

private: // Members

	// --------------------------------------------------------------------------------------------
	// Input file
	wstring m_strIFCFile;

	// --------------------------------------------------------------------------------------------
	// Model
	int_t m_iModel;

	// --------------------------------------------------------------------------------------------
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

	// --------------------------------------------------------------------------------------------
	// World's dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;
	float m_fBoundingSphereDiameter;
	
	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;
	
	vector<CIFCInstance*> m_vecInstances;
	map<int_t, CIFCInstance*> m_mapInstances;  // C Instance : C++ Instance
	map<int_t, CIFCInstance*> m_mapID2Instance; // ID : Instance
	map<int64_t, CIFCInstance*> m_mapExpressID2Instance; // ID : Instance

	CIFCUnitProvider* m_pUnitProvider;
	CIFCPropertyProvider* m_pPropertyProvider;
	CEntityProvider* m_pEntityProvider;

	static int_t s_iInstanceID;

public: // Methods
	
	CIFCModel();
	virtual ~CIFCModel();

	// CModel
	virtual const wchar_t* GetModelName() const override;
	virtual int64_t GetInstance() const override;
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual void ZoomToInstance(CInstance* pInstance) override;
	virtual void ZoomOut() override;

	void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;		
	void ScaleAndCenter(); // [-1, 1]

	void Load(const wchar_t* szIFCFile, int64_t iModel);
	void Clean();
	
	int_t getModel() const;	
	float GetBoundingSphereDiameter() const;	
	const map<int64_t, CIFCInstance*> & GetInstances() const;
	CIFCUnitProvider* GetUnitProvider() const;
	CIFCPropertyProvider* GetPropertyProvider() const;
	CIFCInstance* GetInstanceByID(int_t iID);
	CIFCInstance* GetInstanceByExpressID(int64_t iExpressID);

private: // Methods
	
	void RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth);
	void RetrieveObjects(const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	CIFCInstance* RetrieveGeometry(const wchar_t* szInstanceGUIDW, int_t iInstance, int_t iCircleSegments);
};

#endif // IFCFILEPARSER_H
