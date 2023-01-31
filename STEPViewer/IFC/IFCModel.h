#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "STEPModelBase.h"
#include "IFCInstance.h"
#include "Texture.h"
#include "IFCUnit.h"
#include "IFCEntity.h"
#include "ObjectIFCProperty.h"
#include "BoolIFCProperty.h"
#include "StringIFCProperty.h"
#include "IntIFCProperty.h"
#include "DoubleIFCProperty.h"
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

	// --------------------------------------------------------------------------------------------
	// GUID : CIFCInstance*
	map<wstring, CIFCInstance*> m_mapGUID2Instance;

	// ------------------------------------------------------------------------
	// IFC Units
	map<wstring, CIFCUnit*> m_mapUnits;

	// --------------------------------------------------------------------------------------------
	// Entity : CIFCEntity *
	map<int_t, CIFCEntity*> m_mapEntities;

	// --------------------------------------------------------------------------------------------
	// Instance : Class
	map<int64_t, CIFCClass*> m_mapClasses;

	// --------------------------------------------------------------------------------------------
	// Instance : Property
	map<int64_t, CIFCProperty*> m_mapProperties;

	// --------------------------------------------------------------------------------------------
	// Unique index for each object
	static int_t s_iInstanceID;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CIFCModel();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CIFCModel();

	// --------------------------------------------------------------------------------------------
	// CSTEPModelBase
	virtual int64_t GetInstance() const;

	// --------------------------------------------------------------------------------------------
	// CSTEPModelBase
	virtual void ZoomToInstance(int64_t iInstanceID);

	// --------------------------------------------------------------------------------------------
	// Getter
	virtual void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;

	// --------------------------------------------------------------------------------------------
	// Getter
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;

	// --------------------------------------------------------------------------------------------
	// [-1, 1]
	void ScaleAndCenter();

	// --------------------------------------------------------------------------------------------
	// Loads and IFC file
	void Load(const wchar_t* szIFCFile, int64_t iModel);

	// --------------------------------------------------------------------------------------------
	// Clean up
	void Clean();

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getModel() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t* getModelName() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	float GetBoundingSphereDiameter() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCInstance*> & GetInstances() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<wstring, CIFCUnit*>& GetUnits() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCUnit* GetUnit(const wchar_t* szUnit) const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int_t, CIFCEntity*>& GetEntities() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCClass*>& GetClasses() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCProperty*>& GetProperties() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCInstance* GetInstanceByID(int_t iID);

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCInstance* GetInstanceByExpressID(int64_t iExpressID);

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCInstance* GetInstanceByGUID(const wstring & GUID);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityLength(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Units
	void LoadIFCQuantityArea(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityVolume(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityCount(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityWeight(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Support for properties
	void LoadIFCQuantityTime(int_t iIFCQuantity, wstring & strQuantity);

	// --------------------------------------------------------------------------------------------
	// Schema
	void SaveSchema();

	// --------------------------------------------------------------------------------------------
	// Schema
	void LoadSchema();

private: // Methods		

	// --------------------------------------------------------------------------------------------
	// Retrieves IFC objects by Entity
	void RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth);

	// --------------------------------------------------------------------------------------------
	// Retrieves IFC objects by Entity
	void RetrieveObjects(int_t iEntity, const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements);

	// --------------------------------------------------------------------------------------------
	// Retrieves the geometry for an IFC object
	CIFCInstance * RetrieveGeometry(const wchar_t * szInstanceGUIDW, int_t iEntity, const wchar_t * szEntityNameW, int_t iInstance, int_t iCircleSegments);

	// --------------------------------------------------------------------------------------------
	// Entities
	void LoadUnits();

	// --------------------------------------------------------------------------------------------
	// Entities
	void LoadEntities();

	// --------------------------------------------------------------------------------------------
	// Classes
	void LoadClasses();

	// --------------------------------------------------------------------------------------------
	// Properties
	void LoadProperties();
};

#endif // IFCFILEPARSER_H
