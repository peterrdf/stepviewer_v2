#ifndef IFCFILEPARSER_H
#define IFCFILEPARSER_H

#include "STEPModelBase.h"
#include "IFCObject.h"
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
	int_t m_iIFCModel;

	// --------------------------------------------------------------------------------------------
	// Model
	LONG m_iIFCModelID;

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
	bool m_geometryStarted;
	float m_offsetX;
	float m_offsetY;
	float m_offsetZ;

	// --------------------------------------------------------------------------------------------
	// Min/Max
	pair<float, float> m_prXMinMax;
	pair<float, float> m_prYMinMax;
	pair<float, float> m_prZMinMax;

	// --------------------------------------------------------------------------------------------
	// Bounding sphere diameter
	float m_fBoundingSphereDiameter;

	// --------------------------------------------------------------------------------------------
	// CIFCObject-s
	vector<CIFCObject *> m_vecIFCObjects;

	// --------------------------------------------------------------------------------------------
	// Instance : IFCObject *
	map<int_t, CIFCObject *> m_mapIFCObjects;

	// --------------------------------------------------------------------------------------------
	// ID (1-based) : IFCObject *
	map<int_t, CIFCObject *> m_mapID2IFCObject;

	// --------------------------------------------------------------------------------------------
	// ExpressID : IFCObject *
	map<int64_t, CIFCObject *> m_mapExpressID2IFCObject;

	// --------------------------------------------------------------------------------------------
	// GUID : IFCObject *
	map<wstring, CIFCObject *> m_mapGUID2IFCObject;

	// ------------------------------------------------------------------------
	// IFC Units
	map<wstring, CIFCUnit *> m_mapUnits;

	// --------------------------------------------------------------------------------------------
	// Entity : CIFCEntity *
	map<int_t, CIFCEntity *> m_mapEntities;

	// --------------------------------------------------------------------------------------------
	// Instance : Class
	map<int64_t, CIFCClass *> m_mapClasses;

	// --------------------------------------------------------------------------------------------
	// Instance : Property
	map<int64_t, CIFCProperty *> m_mapProperties;

	// --------------------------------------------------------------------------------------------
	// Selection
	set<int64_t> m_setSelectedIFCObjects;

	// --------------------------------------------------------------------------------------------
	// Selection
	CIFCObject* m_pSubSelection;

	// --------------------------------------------------------------------------------------------
	// Picked instances
	CIFCMaterial * m_pOnMouseOverMaterial;

	// --------------------------------------------------------------------------------------------
	// Selected instances
	CIFCMaterial * m_pSelectionMaterial;

	// --------------------------------------------------------------------------------------------
	// Bounding box
	CIFCMaterial * m_pBoundingBoxMaterial;

	// --------------------------------------------------------------------------------------------
	// Unique index for each object
	static int_t s_iObjectID;

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
	// Loads and IFC file
    bool Load(const wchar_t * szIFCFile);

	// --------------------------------------------------------------------------------------------
	// Loads and IFC file
	bool Load(const wchar_t* szIFCFile, int64_t iInstance);

	// --------------------------------------------------------------------------------------------
	// Getter
	int_t getModel() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	LONG getModelID() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const wchar_t * getModelName() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getXMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getYMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const pair<float, float> & getZMinMax() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const float getXoffset() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const float getYoffset() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const float getZoffset() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	float getBoundingSphereDiameter() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCObject *> & GetIFCObjects() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<wstring, CIFCUnit *> & getUnits() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCUnit * getUnit(const wchar_t * szUnit) const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int_t, CIFCEntity *> & getEntities() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCClass *> & getClasses() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const map<int64_t, CIFCProperty *> & getProperties() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const vector<CIFCObject *> & getIFCObjects();

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCObject * getIFCObject(int_t iObjectID);

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCObject * getIFCObjectByExpressID(int64_t iExpressID);

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCObject * getIFCObjectbyGUID(const wstring & GUID);

	// --------------------------------------------------------------------------------------------
	// Getter
	const set<int64_t>& getSelectedIFCObjects() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	CIFCObject* getSubSelection() const;

	// --------------------------------------------------------------------------------------------
	// User-defined materials
	void SetSelectMaterial(CIFCMaterial * pOnMouseOverMaterial, CIFCMaterial * pOnSelectMaterial, CIFCMaterial * pBoundingBoxMaterial);

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCMaterial * getOnMouseOverMaterial() const;

	// --------------------------------------------------------------------------------------------
	// Getter
	const CIFCMaterial * getSelectionMaterial() const;

	// --------------------------------------------------------------------------------------------
	// Bounding box
	const CIFCMaterial * getBoundingBoxMaterial() const;

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
	CIFCObject * RetrieveGeometry(const wchar_t * szInstanceGUIDW, int_t iEntity, const wchar_t * szEntityNameW, int_t iInstance, int_t iCircleSegments);

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
