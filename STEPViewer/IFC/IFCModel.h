#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "STEPModelBase.h"
#include "IFCInstance.h"
#include "Texture.h"
#include "IFCUnit.h"
#include "IFCEntity.h"
#include "IFCProperty.h"
#include "IFCClass.h"

#include <string>
#include <map>
#include <set>

using namespace std;

// ------------------------------------------------------------------------------------------------
// Parser for IFC files
class CIFCModel : public CSTEPModelBase
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

	// --------------------------------------------------------------------------------------------
	// World's bounding sphere diameter
	float m_fBoundingSphereDiameter;

	// --------------------------------------------------------------------------------------------
	// World's translations - center
	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

	// --------------------------------------------------------------------------------------------
	// CIFCInstance-s
	vector<CIFCInstance*> m_vecInstances;

	// --------------------------------------------------------------------------------------------
	// Handle : CIFCInstance*
	map<int_t, CIFCInstance*> m_mapInstances;

	// --------------------------------------------------------------------------------------------
	// ID (1-based) : CIFCInstance*
	map<int_t, CIFCInstance*> m_mapID2Instance;

	// --------------------------------------------------------------------------------------------
	// ExpressID : CIFCInstance*
	map<int64_t, CIFCInstance*> m_mapExpressID2Instance;

	// ------------------------------------------------------------------------
	// IFC Units
	CIFCUnitProvider* m_pUnitProvider;

	// ------------------------------------------------------------------------
	// IFC Properties
	CIFCPropertyProvider* m_pPropertyProvider;

	// --------------------------------------------------------------------------------------------
	// Entity : CIFCEntity *
	map<int_t, CIFCEntity*> m_mapEntities;

	// --------------------------------------------------------------------------------------------
	// Instance : Class
	map<int64_t, CIFCClass*> m_mapClasses;

	// --------------------------------------------------------------------------------------------
	// Unique index for each object
	static int_t s_iInstanceID;

public: // Methods
	
	CIFCModel();
	virtual ~CIFCModel();

	// CSTEPModelBase
	virtual int64_t GetInstance() const;
	virtual void ZoomToInstance(CSTEPInstance* pSTEPInstance);
	virtual void ZoomOut();	
	virtual void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;		
	void ScaleAndCenter(); // [-1, 1]

	void Load(const wchar_t* szIFCFile, int64_t iModel);
	void Clean();
	
	int_t getModel() const;
	const wchar_t* getModelName() const;
	float GetBoundingSphereDiameter() const;	
	const map<int64_t, CIFCInstance*> & GetInstances() const;
	CIFCUnitProvider* GetUnitProvider() const;
	CIFCPropertyProvider* CIFCModel::GetPropertyProvider() const;
	const map<int_t, CIFCEntity*>& GetEntities() const;
	const map<int64_t, CIFCClass*>& GetClasses() const;
	CIFCInstance* GetInstanceByID(int_t iID);
	CIFCInstance* GetInstanceByExpressID(int64_t iExpressID);
	
	// Schema
	void SaveSchema();
	void LoadSchema();

private: // Methods
	
	void RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth);
	void RetrieveObjects(int_t iEntity, const char* szEntityName, const wchar_t* szEntityNameW, int_t iCircleSegements);
	CIFCInstance* RetrieveGeometry(const wchar_t* szInstanceGUIDW, int_t iEntity, const wchar_t* szEntityNameW, int_t iInstance, int_t iCircleSegments);
	
	void LoadEntities();
	void LoadClasses();
};

#endif // IFCFILEPARSER_H
