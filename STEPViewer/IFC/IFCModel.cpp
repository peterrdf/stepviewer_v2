#include "stdafx.h"

#include "_oglUtils.h"
#include "IFCModel.h"

#include <cwchar>
#include <cfloat>
#include <math.h>

#include "ConceptualFace.h"
#include "colors.h"

// ------------------------------------------------------------------------------------------------
/*static*/ int_t CIFCModel::s_iObjectID = 1;

// ------------------------------------------------------------------------------------------------
extern	ENTITY_COLOR	entityColors[ENTITY_COLOR_CNT];
extern	COLOR			entityDefaultCOLOR;
extern	COLOR			onMouseOverCOLOR;
extern	COLOR			onSelectionCOLOR;
extern	COLOR			boundingBoxCOLOR;

// ------------------------------------------------------------------------------------------------
CIFCModel::CIFCModel()
	: CSTEPModelBase(enumSTEPModelType::IFC)
	, m_strIFCFile(L"")
	, m_iIFCModel(0)
	, m_iIFCModelID(-1)
	, m_ifcProjectEntity(0)
	, m_ifcSpaceEntity(0)
	, m_ifcOpeningElementEntity(0)
	, m_ifcDistributionElementEntity(0)
	, m_ifcElectricalElementEntity(0)
	, m_ifcElementAssemblyEntity(0)
	, m_ifcElementComponentEntity(0)
	, m_ifcEquipmentElementEntity(0)
	, m_ifcFeatureElementEntity(0)
	, m_ifcFeatureElementSubtractionEntity(0)
	, m_ifcFurnishingElementEntity(0)
	, m_ifcReinforcingElementEntity(0)
	, m_ifcTransportElementEntity(0)
	, m_ifcVirtualElementEntity(0)
	, m_geometryStarted(false)
	, m_offsetX(0.f)
	, m_offsetY(0.f)
	, m_offsetZ(0.f)
	, m_prXMinMax(pair<float, float>(-1.f, 1.f))
	, m_prYMinMax(pair<float, float>(-1.f, 1.f))
	, m_prZMinMax(pair<float, float>(-1.f, 1.f))
	, m_fBoundingSphereDiameter(2.f)
	, m_vecIFCObjects()
	, m_mapIFCObjects()
	, m_mapID2IFCObject()
	, m_mapExpressID2IFCObject()
	, m_mapGUID2IFCObject()
	, m_mapUnits()
	, m_mapEntities()
	, m_mapClasses()
	, m_mapProperties()
	, m_setSelectedIFCObjects()
	, m_pSubSelection(NULL)
	, m_pOnMouseOverMaterial(NULL)
	, m_pSelectionMaterial(NULL)
	, m_pBoundingBoxMaterial(NULL)
{
	/*
	* Default
	*/
	m_pOnMouseOverMaterial = new CIFCMaterial();
	m_pOnMouseOverMaterial->Init(
		(BYTE)onMouseOverCOLOR.R, (BYTE)onMouseOverCOLOR.G, (BYTE)onMouseOverCOLOR.B,		// ambient
		(BYTE)onMouseOverCOLOR.R, (BYTE)onMouseOverCOLOR.G, (BYTE)onMouseOverCOLOR.B,		// diffuse
		(BYTE)onMouseOverCOLOR.R, (BYTE)onMouseOverCOLOR.G, (BYTE)onMouseOverCOLOR.B,		// emissive
		(BYTE)onMouseOverCOLOR.R, (BYTE)onMouseOverCOLOR.G, (BYTE)onMouseOverCOLOR.B,		// specular
		(BYTE)onMouseOverCOLOR.A);		

	/*
	* Default
	*/
	m_pSelectionMaterial = new CIFCMaterial();
	m_pSelectionMaterial->Init(
		(BYTE)onSelectionCOLOR.R, (BYTE)onSelectionCOLOR.G, (BYTE)onSelectionCOLOR.B,		// ambient
		(BYTE)onSelectionCOLOR.R, (BYTE)onSelectionCOLOR.G, (BYTE)onSelectionCOLOR.B,		// diffuse
		(BYTE)onSelectionCOLOR.R, (BYTE)onSelectionCOLOR.G, (BYTE)onSelectionCOLOR.B,		// emissive
		(BYTE)onSelectionCOLOR.R, (BYTE)onSelectionCOLOR.G, (BYTE)onSelectionCOLOR.B,		// specular
		(BYTE)onSelectionCOLOR.A);

	/*
	* Default
	*/
	m_pBoundingBoxMaterial = new CIFCMaterial();
	m_pBoundingBoxMaterial->Init(
		(BYTE)boundingBoxCOLOR.R, (BYTE)boundingBoxCOLOR.G, (BYTE)boundingBoxCOLOR.B,		// ambient
		(BYTE)boundingBoxCOLOR.R, (BYTE)boundingBoxCOLOR.G, (BYTE)boundingBoxCOLOR.B,		// diffuse
		(BYTE)boundingBoxCOLOR.R, (BYTE)boundingBoxCOLOR.G, (BYTE)boundingBoxCOLOR.B,		// emissive
		(BYTE)boundingBoxCOLOR.R, (BYTE)boundingBoxCOLOR.G, (BYTE)boundingBoxCOLOR.B,		// specular
		(BYTE)boundingBoxCOLOR.A);	
}

// ------------------------------------------------------------------------------------------------
CIFCModel::~CIFCModel()
{
	if (m_iIFCModel != 0)
	{
		sdaiCloseModel(m_iIFCModel);
		m_iIFCModel = 0;
	}		

	for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	{
		delete m_vecIFCObjects[iIFCObject];
	}
	m_vecIFCObjects.clear();

	map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.begin();
	for (; itUnits != m_mapUnits.end(); itUnits++)
	{
		delete itUnits->second;
	}
	m_mapUnits.clear();

	map<int_t, CIFCEntity *>::iterator itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		delete itEntities->second;
	}
	m_mapEntities.clear();

	map<int64_t, CIFCClass *>::iterator itIFCClasses = m_mapClasses.begin();
	for (; itIFCClasses != m_mapClasses.end(); itIFCClasses++)
	{
		delete itIFCClasses->second;
	}
	m_mapClasses.clear();

	map<int64_t, CIFCProperty *>::iterator itProperty = m_mapProperties.begin();
	for (; itProperty != m_mapProperties.end(); itProperty++)
	{
		delete itProperty->second;
	}
	m_mapProperties.clear();

	delete m_pOnMouseOverMaterial;
	delete m_pSelectionMaterial;
	delete m_pBoundingBoxMaterial;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ int64_t CIFCModel::GetInstance() const
{
	return m_iIFCModel;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCModel::ZoomToInstance(int64_t /*iInstanceID*/)
{
	ASSERT(FALSE); // TODO
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_prXMinMax.first;
	fXmax = m_prXMinMax.second;
	fYmin = m_prYMinMax.first;
	fYmax = m_prYMinMax.second;
	fZmin = m_prZMinMax.first;
	fZmax = m_prZMinMax.second;
}

// ------------------------------------------------------------------------------------------------
bool CIFCModel::Load(const wchar_t * szIFCFile)
{
	/*
	* Load
	*/
	int64_t iModel = sdaiOpenModelBNUnicode(0, (const void*) m_strIFCFile.c_str(), (const void*) L"");
	if (iModel == 0)
    {
		ATLASSERT(FALSE);

        return false;
    }	

	return Load(szIFCFile, iModel);
}

// ------------------------------------------------------------------------------------------------
bool CIFCModel::Load(const wchar_t* szIFCFile, int64_t iInstance)
{
	m_strIFCFile = szIFCFile;
	m_iIFCModel = iInstance;	

	setBRepProperties(
		m_iIFCModel,
		1 + 2 + 4 + 32,	// + 64,
		0.92,
		0.000001,
		600
	);

	/*
	* Entities
	*/
	int_t ifcObjectEntity = sdaiGetEntity(m_iIFCModel, "IFCOBJECT");
	//int_t ifcProductEntity = sdaiGetEntity(m_iIFCModel, "IFCPRODUCT");
	m_ifcProjectEntity = sdaiGetEntity(m_iIFCModel, "IFCPROJECT");
	m_ifcSpaceEntity = sdaiGetEntity(m_iIFCModel, "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(m_iIFCModel, "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(m_iIFCModel, "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(m_iIFCModel, "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(m_iIFCModel, "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(m_iIFCModel, "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(m_iIFCModel, "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(m_iIFCModel, "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(m_iIFCModel, "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(m_iIFCModel, "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(m_iIFCModel, "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(m_iIFCModel, "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(m_iIFCModel, "IFCVIRTUALELEMENT");

	/*
	* Retrieve the objects recursively
	*/
	RetrieveObjects__depth(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS, 0);

	int_t ifcRelSpaceBoundaryEntity = sdaiGetEntity(m_iIFCModel, "IFCRELSPACEBOUNDARY");
	RetrieveObjects(ifcRelSpaceBoundaryEntity, "IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

	/*
	* Units
	*/
	LoadUnits();

	/*
	* Entities
	*/
	LoadEntities();

	/*
	* Classes
	*/
	LoadClasses();

	/*
	* Properties
	*/
	LoadProperties();

	/*
	* Helper data structures
	*/
	for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	{
		CIFCObject* pIFCObject = m_vecIFCObjects[iIFCObject];

		m_mapID2IFCObject[pIFCObject->ID()] = pIFCObject;
		m_mapExpressID2IFCObject[pIFCObject->expressID()] = pIFCObject;
		m_mapGUID2IFCObject[pIFCObject->getGUID()] = pIFCObject;
	}

	/*
	* Min/Max
	*/
	float fXmin = FLT_MAX;
	float fXmax = -FLT_MAX;
	float fYmin = FLT_MAX;
	float fYmax = -FLT_MAX;
	float fZmin = FLT_MAX;
	float fZmax = -FLT_MAX;

	for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	{
		CIFCObject* pIFCObject = m_vecIFCObjects[iIFCObject];

		if (!pIFCObject->hasGeometry())
		{
			// skip the objects without geometry
			continue;
		}

		pIFCObject->CalculateMinMaxValues(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);
	}

	m_prXMinMax = pair<float, float>(fXmin, fXmax);
	m_prYMinMax = pair<float, float>(fYmin, fYmax);
	m_prZMinMax = pair<float, float>(fZmin, fZmax);

	m_fBoundingSphereDiameter = fXmax - fXmin;
	m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, fYmax - fYmin);
	m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, fZmax - fZmin);

	/*
	* Build selection colors - TODO!!!
	*/
	//const float STEP = 1.0f / 255.0f;

	//for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	//{
	//	CIFCObject* pIFCObject = m_vecIFCObjects[iIFCObject];

	//	if (!pIFCObject->hasGeometry())
	//	{
	//		// skip the objects without geometry
	//		continue;
	//	}

	//	float fR = floor((float)pIFCObject->ID() / (255.0f * 255.0f));
	//	if (fR >= 1.0f)
	//	{
	//		fR *= STEP;
	//	}

	//	float fG = floor((float)pIFCObject->ID() / 255.0f);
	//	if (fG >= 1.0f)
	//	{
	//		fG *= STEP;
	//	}

	//	float fB = (float)(pIFCObject->ID() % 255);
	//	fB *= STEP;

	//	ASSERT(pIFCObject->rgbID() != NULL);
	//	pIFCObject->rgbID()->Init(fR, fG, fB);
	//} // for (; itIFCCobject != ...		

	return true;
}

// ------------------------------------------------------------------------------------------------
int_t CIFCModel::getModel() const
{
	return m_iIFCModel;
}

// ------------------------------------------------------------------------------------------------
LONG CIFCModel::getModelID() const
{
	return m_iIFCModelID;
}

// ------------------------------------------------------------------------------------------------
const wchar_t * CIFCModel::getModelName() const
{
	return m_strIFCFile.c_str();
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCModel::getXMinMax() const
{
	return m_prXMinMax;
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCModel::getYMinMax() const
{
	return m_prYMinMax;
}

// ------------------------------------------------------------------------------------------------
const pair<float, float> & CIFCModel::getZMinMax() const
{
	return m_prZMinMax;
}

// ------------------------------------------------------------------------------------------------
const float CIFCModel::getXoffset() const
{
	return m_offsetX;
}

// ------------------------------------------------------------------------------------------------
const float CIFCModel::getYoffset() const
{
	return m_offsetY;
}

// ------------------------------------------------------------------------------------------------
const float CIFCModel::getZoffset() const
{
	return m_offsetZ;
}

// ------------------------------------------------------------------------------------------------
float CIFCModel::getBoundingSphereDiameter() const
{
	return m_fBoundingSphereDiameter;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCObject *> & CIFCModel::GetIFCObjects() const
{
	return m_mapIFCObjects;
}

// ------------------------------------------------------------------------------------------------
const map<wstring, CIFCUnit *> & CIFCModel::getUnits() const
{
	return m_mapUnits;
}

// ------------------------------------------------------------------------------------------------
const CIFCUnit * CIFCModel::getUnit(const wchar_t * szUnit) const
{
	map<wstring, CIFCUnit *>::const_iterator itUnit = m_mapUnits.find(szUnit);
	if (itUnit != m_mapUnits.end())
	{
		return itUnit->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CIFCEntity *> & CIFCModel::getEntities() const
{
	return m_mapEntities;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCClass *> & CIFCModel::getClasses() const
{
	return m_mapClasses;
}

// ------------------------------------------------------------------------------------------------
const map<int64_t, CIFCProperty *> & CIFCModel::getProperties() const
{
	return m_mapProperties;
}

// ------------------------------------------------------------------------------------------------
const vector<CIFCObject *> & CIFCModel::getIFCObjects()
{
	return m_vecIFCObjects;
}

// ------------------------------------------------------------------------------------------------
CIFCObject * CIFCModel::getIFCObject(int_t iObjectID)
{
	map<int_t, CIFCObject *>::iterator itID2IFCObject = m_mapID2IFCObject.find(iObjectID);
	if (itID2IFCObject != m_mapID2IFCObject.end())
	{
		return itID2IFCObject->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
CIFCObject * CIFCModel::getIFCObjectByExpressID(int64_t iExpressID)
{
	map<int64_t, CIFCObject *>::iterator itExpressID2IFCObject = m_mapExpressID2IFCObject.find(iExpressID);
	if (itExpressID2IFCObject != m_mapExpressID2IFCObject.end())
	{
		return itExpressID2IFCObject->second;
	}

	return NULL;
}

// --------------------------------------------------------------------------------------------
CIFCObject * CIFCModel::getIFCObjectbyGUID(const wstring & GUID)
{
	map<wstring, CIFCObject *>::iterator itGUID2IFCObject = m_mapGUID2IFCObject.find(GUID);
	if (itGUID2IFCObject != m_mapGUID2IFCObject.end())
	{
		return itGUID2IFCObject->second;
	}

	return NULL;
}

// ------------------------------------------------------------------------------------------------
const set<int64_t>& CIFCModel::getSelectedIFCObjects() const
{
	return m_setSelectedIFCObjects;
}

// ------------------------------------------------------------------------------------------------
CIFCObject* CIFCModel::getSubSelection() const
{
	return m_pSubSelection;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::SetSelectMaterial(CIFCMaterial * pOnMouseOverMaterial, CIFCMaterial * pOnSelectMaterial, CIFCMaterial * pBoundingBoxMaterial)
{
	m_pOnMouseOverMaterial = pOnMouseOverMaterial;
	m_pSelectionMaterial = pOnSelectMaterial;
	m_pBoundingBoxMaterial = pBoundingBoxMaterial;
}

void CIFCModel::SetFallbackColor(CIFCMaterial * pMaterial)
{	
	for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	{
		CIFCObject * pIFCObject = m_vecIFCObjects[iIFCObject];

		for (size_t iMaterial = 0; iMaterial < pIFCObject->m_vecConceptualFacesMaterials.size(); iMaterial++)
		{
			const CIFCGeometryWithMaterial	* pConceptualFaceGeometryWithMaterial = pIFCObject->m_vecConceptualFacesMaterials[iMaterial];
			const CIFCMaterial				* pConceptualFaceMaterial = pConceptualFaceGeometryWithMaterial->getMaterial();

			const CIFCColor	* ambientColor = &pConceptualFaceMaterial->getAmbientColor();
			const CIFCColor	* diffuseColor = &pConceptualFaceMaterial->getDiffuseColor();
			const CIFCColor	* emissiveColor = &pConceptualFaceMaterial->getEmissiveColor();
			const CIFCColor	* specularColor = &pConceptualFaceMaterial->getSpecularColor();
			if (ambientColor->R() == 0 &&
				ambientColor->G() == 0 &&
				ambientColor->B() == 0 &&
				diffuseColor->R() == 0 &&
				diffuseColor->G() == 0 &&
				diffuseColor->B() == 0 &&
				emissiveColor->R() == 0 &&
				emissiveColor->G() == 0 &&
				emissiveColor->B() == 0 &&
				specularColor->R() == 0 &&
				specularColor->G() == 0 &&
				specularColor->B() == 0) {
				pIFCObject->m_vecConceptualFacesMaterials[iMaterial]->setMaterial(pMaterial);
			}
		}
	} // for (size_t iIFCObject = ...		
}

void CIFCModel::SetFallbackColorPerEntity(CString entityName, CIFCMaterial * pMaterial)
{
	entityName.MakeUpper();

	for (size_t iIFCObject = 0; iIFCObject < m_vecIFCObjects.size(); iIFCObject++)
	{
		CIFCObject * pIFCObject = m_vecIFCObjects[iIFCObject];

		wstring	myStr = pIFCObject->m_strEntity;

		CString myObjectInstanceStr = myStr.c_str();
		myObjectInstanceStr.MakeUpper();

		if (entityName.Compare(myObjectInstanceStr) == 0) {
			for (size_t iMaterial = 0; iMaterial < pIFCObject->m_vecConceptualFacesMaterials.size(); iMaterial++)
			{
				const CIFCMaterial * pConceptualFaceMaterial = pIFCObject->m_vecConceptualFacesMaterials[iMaterial]->getMaterial();

				const CIFCColor	* ambientColor = &pConceptualFaceMaterial->getAmbientColor();
				const CIFCColor	* diffuseColor = &pConceptualFaceMaterial->getDiffuseColor();
				const CIFCColor	* emissiveColor = &pConceptualFaceMaterial->getEmissiveColor();
				const CIFCColor	* specularColor = &pConceptualFaceMaterial->getSpecularColor();
				if (ambientColor->R() == 0 &&
					ambientColor->G() == 0 &&
					ambientColor->B() == 0 &&
					diffuseColor->R() == 0 &&
					diffuseColor->G() == 0 &&
					diffuseColor->B() == 0 &&
					emissiveColor->R() == 0 &&
					emissiveColor->G() == 0 &&
					emissiveColor->B() == 0 &&
					specularColor->R() == 0 &&
					specularColor->G() == 0 &&
					specularColor->B() == 0) {
					pIFCObject->m_vecConceptualFacesMaterials[iMaterial]->setMaterial(pMaterial);
				}
			}
		}
	} // for (size_t iIFCObject = ...		
}

// ------------------------------------------------------------------------------------------------
const CIFCMaterial * CIFCModel::getOnMouseOverMaterial() const
{
	return m_pOnMouseOverMaterial;
}

// ------------------------------------------------------------------------------------------------
const CIFCMaterial * CIFCModel::getSelectionMaterial() const
{
	return m_pSelectionMaterial;
}

// ------------------------------------------------------------------------------------------------
const CIFCMaterial * CIFCModel::getBoundingBoxMaterial() const
{
	return m_pBoundingBoxMaterial;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityLength(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "LengthValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"LENGTHUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityArea(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "AreaValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"AREAUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityVolume(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "VolumeValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"VOLUMEUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityCount(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "CountValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)		

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityWeight(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "WeigthValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"MASSUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadIFCQuantityTime(int_t iIFCQuantity, wstring & strQuantity)
{
	wchar_t	* szQuantityName = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Name", sdaiUNICODE, &szQuantityName);

	wchar_t	* szQuantityDescription = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Description", sdaiUNICODE, &szQuantityDescription);

	wchar_t	* szValue = NULL;
	sdaiGetAttrBN(iIFCQuantity, "TimeValue", sdaiUNICODE, &szValue);

	wchar_t	* szUnit = NULL;
	sdaiGetAttrBN(iIFCQuantity, "Unit", sdaiUNICODE, &szUnit);

	strQuantity = szQuantityName;
	strQuantity += L" = ";
	strQuantity += szValue;

	if (szUnit != NULL)
	{
		strQuantity += L" ";
		strQuantity += szUnit;
	} // if (szUnit != NULL)
	else
	{
		map<wstring, CIFCUnit *>::iterator itUnits = m_mapUnits.find(L"TIMEUNIT");
		if (itUnits != m_mapUnits.end())
		{
			strQuantity += L" ";
			strQuantity += itUnits->second->getName();
		}
	} // else if (szUnit != NULL)	

	if ((szQuantityDescription != NULL) && (wcslen(szQuantityDescription) > 0))
	{
		strQuantity += L" ('";
		strQuantity += szQuantityDescription;
		strQuantity += L"')";
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::SaveSchema()
{
	TCHAR szFilters[] = _T("Schema Files (*.ifcschema)|*.ifcschema|All Files (*.*)|*.*||");

	CFileDialog dlgFile(FALSE, _T("ifcschema"), _T("IFC Schema"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	CString strSchemaFile = dlgFile.GetPathName();

	CFile file(strSchemaFile, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary);
	CArchive archive(&file, CArchive::store);

	const int VERSION = 1;

	// Version
	archive << VERSION;

	// Entities Count
	archive << m_mapEntities.size();

	map<int_t, CIFCEntity*>::const_iterator itEntity = m_mapEntities.begin();
	for (; itEntity != m_mapEntities.end(); itEntity++)
	{
		// Entity Name
		archive << CString(itEntity->second->getName());

		const set<wstring>& setIgnoredAttributes = itEntity->second->getIgnoredAttributes();

		// Ignored Attributes Count
		archive << setIgnoredAttributes.size();

		set<wstring>::const_iterator itAttribute = setIgnoredAttributes.begin();
		for (; itAttribute != setIgnoredAttributes.end(); itAttribute++)
		{
			// Attribute Name
			archive << CString(itAttribute->c_str());
		}
	} // for (; itEntity != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadSchema()
{
	TCHAR szFilters[] = _T("Schema Files (*.ifcschema)|*.ifcschema|All Files (*.*)|*.*||");

	CFileDialog dlgFile(TRUE, _T("ifcschema"), _T("IFC Schema"),
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	CString strSchemaFile = dlgFile.GetPathName();

	CFile file;
	file.Open(strSchemaFile, CFile::modeRead | CFile::typeBinary);
	CArchive archive(&file, CArchive::load);

	int VERSION = 1;

	// Version
	archive >> VERSION;
	ASSERT(VERSION == 1);

	// Entities Count
	size_t iEntitiesCount = 0;
	archive >> iEntitiesCount;

	map<wstring, vector<wstring>> mapIgnoredAttributes;
	for (size_t iEntity = 0; iEntity < iEntitiesCount; iEntity++)
	{
		CString strEntity;
		archive >> strEntity;

		size_t iIgnoredAttributesCount = 0;
		archive >> iIgnoredAttributesCount;

		vector<wstring> vecIgnoredAttributes;
		for (size_t iIgnoredAttribute = 0; iIgnoredAttribute < iIgnoredAttributesCount; iIgnoredAttribute++)
		{
			CString strAttribute;
			archive >> strAttribute;

			vecIgnoredAttributes.push_back((LPCTSTR)strAttribute);
		}

		if (!vecIgnoredAttributes.empty())
		{
			mapIgnoredAttributes[(LPCTSTR)strEntity] = vecIgnoredAttributes;
		}
	} // for (size_t iEntity = ...

	map<int_t, CIFCEntity*>::iterator itEntity = m_mapEntities.begin();
	for (; itEntity != m_mapEntities.end(); itEntity++)
	{
		CIFCEntity* pIFCEntity = itEntity->second;

		map<wstring, vector<wstring>>::iterator itIgnoredAttributes = mapIgnoredAttributes.find(pIFCEntity->getName());
		if (itIgnoredAttributes != mapIgnoredAttributes.end())
		{
			for (size_t iIgnoredAttribute = 0; iIgnoredAttribute < itIgnoredAttributes->second.size(); iIgnoredAttribute++)
			{
				if (!pIFCEntity->isAttributeIgnored(itIgnoredAttributes->second[iIgnoredAttribute]))
				{
					pIFCEntity->ignoreAttribute(itIgnoredAttributes->second[iIgnoredAttribute], true);
				}
			}
		}
	} // for (; itEntity != ...
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects(int_t iEntity, const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	int_t * iIFCObjectInstances = sdaiGetEntityExtentBN(m_iIFCModel, (char *) szEntityName);

	int_t iIFCObjectInstancesCount = sdaiGetMemberCount(iIFCObjectInstances);
	if (iIFCObjectInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCObjectInstancesCount; ++i)
	{
		int_t iInstance = 0;
		engiGetAggrElement(iIFCObjectInstances, i, sdaiINSTANCE, &iInstance);

		wchar_t	* szInstanceGUIDW = nullptr;
		sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

		CIFCObject * pIFCObject = RetrieveGeometry(szInstanceGUIDW, iEntity, szEntityNameW, iInstance, iCircleSegements);
		pIFCObject->ID() = s_iObjectID++;

		CString strEntity = szEntityNameW;
		strEntity.MakeUpper();

		pIFCObject->visible__() = (strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT");
		
		m_vecIFCObjects.push_back(pIFCObject);
		m_mapIFCObjects[iInstance] = pIFCObject;
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects__depth(int_t iParentEntity, int_t iCircleSegments, int_t depth)
{
	if ((iParentEntity == m_ifcDistributionElementEntity) ||
		(iParentEntity == m_ifcElectricalElementEntity) ||
		(iParentEntity == m_ifcElementAssemblyEntity) ||
		(iParentEntity == m_ifcElementComponentEntity) ||
		(iParentEntity == m_ifcEquipmentElementEntity) ||
		(iParentEntity == m_ifcFeatureElementEntity) ||
		(iParentEntity == m_ifcFurnishingElementEntity) ||
		(iParentEntity == m_ifcTransportElementEntity) ||
		(iParentEntity == m_ifcVirtualElementEntity))
	{
		iCircleSegments = 12;
	}

	if (iParentEntity == m_ifcReinforcingElementEntity)
	{
		iCircleSegments = 6;
	}

	int_t* ifcObjectInstances = sdaiGetEntityExtent(m_iIFCModel, iParentEntity);
	int_t noIfcObjectIntances = sdaiGetMemberCount(ifcObjectInstances);

	if (noIfcObjectIntances != 0)
	{
		char * szParenEntityName = NULL;
		engiGetEntityName(iParentEntity, sdaiSTRING, &szParenEntityName);

		wchar_t	* szParentEntityNameW = NULL;
		engiGetEntityName(iParentEntity, sdaiUNICODE, (char **)&szParentEntityNameW);

		RetrieveObjects(iParentEntity, szParenEntityName, szParentEntityNameW, iCircleSegments);

		if (iParentEntity == m_ifcProjectEntity) {
			for (int_t i = 0; i < noIfcObjectIntances; i++) {
				int_t ifcObjectInstance = 0;
				engiGetAggrElement(ifcObjectInstances, i, sdaiINSTANCE, &ifcObjectInstance);

				wchar_t	* szInstanceGUIDW = nullptr;
				sdaiGetAttrBN(ifcObjectInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

				CIFCObject * pIFCObject = RetrieveGeometry(szInstanceGUIDW, iParentEntity, szParentEntityNameW, ifcObjectInstance, iCircleSegments);
				pIFCObject->ID() = s_iObjectID++;

				CString strEntity = szParentEntityNameW;
				strEntity.MakeUpper();

				pIFCObject->visible__() = (strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT");

				m_vecIFCObjects.push_back(pIFCObject);
				m_mapIFCObjects[ifcObjectInstance] = pIFCObject;
			}
		}
	} // if (noIfcObjectIntances != 0)

	noIfcObjectIntances = engiGetEntityCount(m_iIFCModel);
	for (int_t i = 0; i < noIfcObjectIntances; i++)
	{
		int_t ifcEntity = engiGetEntityElement(m_iIFCModel, i);
		if (engiGetEntityParent(ifcEntity) == iParentEntity)
		{
			RetrieveObjects__depth(ifcEntity, iCircleSegments, depth + 1);
		}
	}
}

bool	EQUALSUC(const wchar_t * strI, const wchar_t * strII)
{
	if (strI && strII) {
		int i = 0;
		while (strI[i]) {
			wchar_t	strI_UC = (strI[i] >= 'a' && strI[i] <= 'z') ? strI[i] + 'A' - 'a' : strI[i],
					strII_UC = (strII[i] >= 'a' && strII[i] <= 'z') ? strII[i] + 'A' - 'a' : strII[i];
			if (strI_UC != strII_UC) {
				return	false;
			}
			i++;
		}

		if (strII[i] == 0) {
			return	true;
		}
	}

	return	false;
}

// ------------------------------------------------------------------------------------------------
CIFCObject * CIFCModel::RetrieveGeometry(const wchar_t * szInstanceGUIDW, int_t iEntity, const wchar_t * szEntityNameW, int_t iInstance, int_t iCircleSegments)
{
	CIFCObject * pIFCObject = new CIFCObject(this, iInstance, szInstanceGUIDW, iEntity, szEntityNameW);

	/*
	* Set up format
	*/
	int_t setting = 0, mask = 0;
	mask += flagbit2;        // PRECISION (32/64 bit)
	mask += flagbit3;        //	INDEX ARRAY (32/64 bit)
	mask += flagbit5;        // NORMALS
	mask += flagbit6;        // TEXTURE
	mask += flagbit8;        // TRIANGLES
	mask += flagbit9;        // LINES
	mask += flagbit10;       // POINTS
	mask += flagbit12;       // WIREFRAME
	mask += flagbit17;       // OPENGL
	mask += flagbit24;		 //	AMBIENT
	mask += flagbit25;		 //	DIFFUSE
	mask += flagbit26;		 //	EMISSIVE
	mask += flagbit27;		 //	SPECULAR

	setting += 0;		     // SINGLE PRECISION (float)
	setting += 0;            // 32 BIT INDEX ARRAY (Int32)
	setting += flagbit5;     // NORMALS ON
	setting += 0;			 // TEXTURE OFF
	setting += flagbit8;     // TRIANGLES ON
	setting += flagbit9;     // LINES ON
	setting += flagbit10;    // POINTS ON
	setting += flagbit12;    // WIREFRAME ON
	setting += 0;		     // OPENGL
	setting += flagbit24;	 //	AMBIENT
	setting += flagbit25;	 //	DIFFUSE
	setting += flagbit26;	 //	EMISSIVE
	setting += flagbit27;	 //	SPECULAR
	setFormat(m_iIFCModel, setting, mask);
	setFilter(m_iIFCModel, flagbit1, flagbit1);

	/*
	* Default color
	*/
	// ((R * 255 + G) * 255 + B) * 255 + A
	uint32_t	R = entityDefaultCOLOR.R,
				G = entityDefaultCOLOR.G,
				B = entityDefaultCOLOR.B,
				A = entityDefaultCOLOR.A;

	for (int i = 0; i < ENTITY_COLOR_CNT; i++) {
		if (EQUALSUC(entityColors[i].entityName, szEntityNameW)) {
			R = entityColors[i].R;
			G = entityColors[i].G;
			B = entityColors[i].B;
			A = entityColors[i].A;
		}
	}

	uint32_t iDefaultColor = 256 * 256 * 256 * R +
		256 * 256 * G +
		256 * B +
		A;

	SetDefaultColor(m_iIFCModel, iDefaultColor, iDefaultColor, iDefaultColor, iDefaultColor);

	/*
	* Set up circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	int64_t iVerticesCount = 0;
	int64_t iIndicesCount = 0;
	CalculateInstance(iInstance, &iVerticesCount, &iIndicesCount, 0);
	if ((iVerticesCount > 0) && (iIndicesCount > 0))
	{
		if (!m_geometryStarted) 
		{
			float* pVertices = new float[(int_t) iVerticesCount * VERTEX_LENGTH];
			UpdateInstanceVertexBuffer(iInstance, pVertices);

			SetVertexBufferOffset(m_iIFCModel, -pVertices[0], -pVertices[1], -pVertices[2]);
			m_offsetX = pVertices[0];
			m_offsetY = pVertices[1];
			m_offsetZ = pVertices[2];

			delete[] pVertices;

			ClearedInstanceExternalBuffers(iInstance);
			CalculateInstance(iInstance, &iVerticesCount, &iIndicesCount, 0);

			m_geometryStarted = true;
		}

		int64_t iOWLModel = 0;
		owlGetModel(m_iIFCModel, &iOWLModel);

		int64_t iOWLInstance = 0;
		owlGetInstance(m_iIFCModel, iInstance, &iOWLInstance);		

		pIFCObject->verticesCount() = (int_t)iVerticesCount;

		
		float * pVertices = new float[(int_t)iVerticesCount * VERTEX_LENGTH];
		UpdateInstanceVertexBuffer(iOWLInstance, pVertices);

		int32_t * pIndices = new int32_t[(int_t) iIndicesCount];
		UpdateInstanceIndexBuffer(iOWLInstance, pIndices);
		
		// MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.
		map<CIFCGeometryWithMaterial, vector<CConceptualFace>, CIFCGeometryWithMaterialComparator> mapMaterial2ConceptualFaces;
		vector<pair<int_t, int_t>> vecWireframesIndices;
		vector<pair<int_t, int_t>> vecLinesIndices;
		vector<pair<int_t, int_t>> vecPointsIndices;		

		/*
		* Extract the conceptual face - triangles and polygons
		*/
		int_t iFacesCount = getConceptualFaceCnt(iInstance);

		pIFCObject->conceptualFacesCount() = iFacesCount;

		for (int_t iFace = 0; iFace < iFacesCount; iFace++)
		{
			int_t iStartIndexTriangles = 0;
			int_t iIndicesCountTriangles = 0;

			int_t iStartIndexLines = 0;
			int_t iIndicesCountLines = 0;

			int_t iStartIndexPoints = 0;
			int_t iIndicesCountPoints = 0;

			int_t iStartIndexFacesPolygons = 0;
			int_t iIndicesCountFacesPolygons = 0;

			getConceptualFaceEx(
				iInstance, iFace,
				&iStartIndexTriangles, &iIndicesCountTriangles,
				&iStartIndexLines, &iIndicesCountLines,
				&iStartIndexPoints, &iIndicesCountPoints,
				&iStartIndexFacesPolygons, &iIndicesCountFacesPolygons,
				0, 0);

			if (iIndicesCountTriangles > 0)
			{
				/*
				* Material
				*/
				CIFCGeometryWithMaterial * pMaterial = NULL;

				int32_t iIndexValue = *(pIndices + iStartIndexTriangles);
				iIndexValue *= VERTEX_LENGTH;

				float fColor = *(pVertices + iIndexValue + 6);
				unsigned int iAmbientColor = *(reinterpret_cast<unsigned int *>(&fColor));
				float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

				fColor = *(pVertices + iIndexValue + 7);
				unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int *>(&fColor));

				fColor = *(pVertices + iIndexValue + 8);
				unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int *>(&fColor));

				fColor = *(pVertices + iIndexValue + 9);
				unsigned int iSpecularColor = *(reinterpret_cast<unsigned int *>(&fColor));

				pMaterial = new CIFCGeometryWithMaterial(iAmbientColor, iDiffuseColor, iEmissiveColor, iSpecularColor, fTransparency);

				map<CIFCGeometryWithMaterial, vector<CConceptualFace>, CIFCGeometryWithMaterialComparator>::iterator itMaterial2ConceptualFaces = mapMaterial2ConceptualFaces.find(*pMaterial);
				if (itMaterial2ConceptualFaces == mapMaterial2ConceptualFaces.end())
				{
					vector<CConceptualFace> vecConceptualFaces;

					CConceptualFace conceptualFace;
					conceptualFace.index() = iFace;
					conceptualFace.trianglesStartIndex() = iStartIndexTriangles;
					conceptualFace.trianglesIndicesCount() = iIndicesCountTriangles;

					vecConceptualFaces.push_back(conceptualFace);

					mapMaterial2ConceptualFaces[*pMaterial] = vecConceptualFaces;
				}
				else
				{
					CConceptualFace conceptualFace;
					conceptualFace.index() = iFace;
					conceptualFace.trianglesStartIndex() = iStartIndexTriangles;
					conceptualFace.trianglesIndicesCount() = iIndicesCountTriangles;

					itMaterial2ConceptualFaces->second.push_back(conceptualFace);
				}

				delete pMaterial;
			} // if (iIndicesCountTriangles > 0)

			if (iIndicesCountLines > 0)
			{
				vecLinesIndices.push_back(pair<int_t, int_t>(iStartIndexLines, iIndicesCountLines));
			}

			if (iIndicesCountPoints > 0)
			{
				vecPointsIndices.push_back(pair<int_t, int_t>(iStartIndexPoints, iIndicesCountPoints));
			}

			if (iIndicesCountFacesPolygons > 0)
			{
				vecWireframesIndices.push_back(pair<int_t, int_t>(iStartIndexFacesPolygons, iIndicesCountFacesPolygons));
			}
		} // for (int_t iFace = ...

		/*
		* Group the triangles
		*/
		map<CIFCGeometryWithMaterial, vector<CConceptualFace>, CIFCGeometryWithMaterialComparator>::iterator itMaterial2ConceptualFaces = mapMaterial2ConceptualFaces.begin();
		for (; itMaterial2ConceptualFaces != mapMaterial2ConceptualFaces.end(); itMaterial2ConceptualFaces++)
		{
			CIFCGeometryWithMaterial * pMaterial = NULL;

			for (size_t iConceptualFace = 0; iConceptualFace < itMaterial2ConceptualFaces->second.size(); iConceptualFace++)
			{
				CConceptualFace & conceptualFace = itMaterial2ConceptualFaces->second[iConceptualFace];

				int_t iStartIndexTriangles = conceptualFace.trianglesStartIndex();
				int_t iIndicesCountTriangles = conceptualFace.trianglesIndicesCount();

				/*
				* Split the conceptual face - isolated case
				*/
				if (iIndicesCountTriangles > _oglUtils::getIndicesCountLimit())
				{
					while (iIndicesCountTriangles > _oglUtils::getIndicesCountLimit())
					{
						// INDICES
						CIFCGeometryWithMaterial * pNewMaterial = new CIFCGeometryWithMaterial(itMaterial2ConceptualFaces->first);
						for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + _oglUtils::getIndicesCountLimit(); iIndex++)
						{							
							pNewMaterial->addIndex(pIndices[iIndex]);
						}
						
						pIFCObject->conceptualFacesMaterials().push_back(pNewMaterial);

						/*
						* Update Conceptual face start index
						*/
						conceptualFace.trianglesStartIndex() = 0;

						// Conceptual faces
						pNewMaterial->conceptualFaces().push_back(conceptualFace);

						iIndicesCountTriangles -= _oglUtils::getIndicesCountLimit();
						iStartIndexTriangles += _oglUtils::getIndicesCountLimit();
					}

					if (iIndicesCountTriangles > 0)
					{
						// INDICES
						CIFCGeometryWithMaterial * pNewMaterial = new CIFCGeometryWithMaterial(itMaterial2ConceptualFaces->first);
						for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + iIndicesCountTriangles; iIndex++)
						{
							pNewMaterial->addIndex(pIndices[iIndex]);
						}

						pIFCObject->conceptualFacesMaterials().push_back(pNewMaterial);

						/*
						* Update Conceptual face start index
						*/
						conceptualFace.trianglesStartIndex() = 0;

						// Conceptual faces
						pNewMaterial->conceptualFaces().push_back(conceptualFace);
					}					

					continue;
				} // if (iIndicesCountTriangles > _oglUtils::getIndicesCountLimit())	

				/*
				* Create material
				*/
				if (pMaterial == NULL)
				{
					pMaterial = new CIFCGeometryWithMaterial(itMaterial2ConceptualFaces->first);

					pIFCObject->conceptualFacesMaterials().push_back(pMaterial);
				}
				
				/*
				* Check the limit
				*/
				if (pMaterial->getIndicesCount() + iIndicesCountTriangles > _oglUtils::getIndicesCountLimit())
				{
					pMaterial = new CIFCGeometryWithMaterial(itMaterial2ConceptualFaces->first);

					pIFCObject->conceptualFacesMaterials().push_back(pMaterial);
				}

				/*
				* Update Conceptual face start index
				*/
				conceptualFace.trianglesStartIndex() = pMaterial->getIndicesCount();

				/*
				* Add the indices
				*/
				for (int_t iIndex = iStartIndexTriangles; iIndex < iStartIndexTriangles + iIndicesCountTriangles; iIndex++)
				{
					pMaterial->addIndex(pIndices[iIndex]);
				}

				// Conceptual faces
				pMaterial->conceptualFaces().push_back(conceptualFace);
			} // for (size_t iConceptualFace = ...				
		} // for (; itMaterial2ConceptualFaces != ...

		/*
		* Group the polygons
		*/
		if (!vecWireframesIndices.empty())
		{
			CWireframesCohort * pWireframesCohort = NULL;
			for (size_t iFace = 0; iFace < vecWireframesIndices.size(); iFace++)
			{
				int_t iStartIndexFacesPolygons = vecWireframesIndices[iFace].first;
				int_t iIndicesFacesPolygonsCount = vecWireframesIndices[iFace].second;

				/*
				* Create the cohort
				*/
				if (pWireframesCohort == NULL)
				{
					pWireframesCohort = new CWireframesCohort();
					pIFCObject->wireframesCohorts().push_back(pWireframesCohort);
				}

				/*
				* Check the limit
				*/
				if (pWireframesCohort->getIndicesCount() + iIndicesFacesPolygonsCount > _oglUtils::getIndicesCountLimit())
				{
					pWireframesCohort = new CWireframesCohort();
					pIFCObject->wireframesCohorts().push_back(pWireframesCohort);
				}

				int_t iPreviousIndex = -1;
				for (int_t iIndex = iStartIndexFacesPolygons; iIndex < iStartIndexFacesPolygons + iIndicesFacesPolygonsCount; iIndex++)
				{
					if (pIndices[iIndex] < 0)
					{
						iPreviousIndex = -1;

						continue;
					}

					if (iPreviousIndex != -1)
					{
						pWireframesCohort->addIndex(pIndices[iPreviousIndex]);
						pWireframesCohort->addIndex(pIndices[iIndex]);
					} // if (iPreviousIndex != -1)

					iPreviousIndex = iIndex;
				} // for (int_t iIndex = ...
			} // for (size_t iFace = ...
		} // if (!vecWireframes.empty())

		/*
		* Group the lines
		*/
		if (!vecLinesIndices.empty())
		{
			CLinesCohort * pLinesCohort = NULL;
			for (size_t iFace = 0; iFace < vecLinesIndices.size(); iFace++)
			{
				int_t iStartIndexLines = vecLinesIndices[iFace].first;
				int_t iIndicesLinesCount = vecLinesIndices[iFace].second;

				/*
				* Create the cohort
				*/
				if (pLinesCohort == NULL)
				{
					pLinesCohort = new CLinesCohort();
					pIFCObject->linesCohorts().push_back(pLinesCohort);
				}

				/*
				* Check the limit
				*/
				if (pLinesCohort->getIndicesCount() + iIndicesLinesCount > _oglUtils::getIndicesCountLimit())
				{
					pLinesCohort = new CLinesCohort();
					pIFCObject->linesCohorts().push_back(pLinesCohort);
				}

				int_t iPreviousIndex = -1;
				for (int_t iIndex = iStartIndexLines; iIndex < iStartIndexLines + iIndicesLinesCount; iIndex++)
				{
					if (pIndices[iIndex] < 0)
					{
						iPreviousIndex = -1;

						continue;
					}

					if (iPreviousIndex != -1)
					{
						pLinesCohort->addIndex(pIndices[iPreviousIndex]);
						pLinesCohort->addIndex(pIndices[iIndex]);
					} // if (iPreviousIndex != -1)

					iPreviousIndex = iIndex;
				} // for (int_t iIndex = ...
			} // for (size_t iFace = ...
		} // if (!vecLines.empty())

		/*
		* Group the points
		*/
		// TODO

		/*
		* Copy the vertices - <X, Y, Z, Nx, Ny, Nz>
		*/
		pIFCObject->vertices() = new float[(int_t) iVerticesCount * GEOMETRY_VBO_VERTEX_LENGTH];
		for (int_t iVertex = 0; iVertex < iVerticesCount; iVertex++)
		{
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 0] = pVertices[(iVertex * VERTEX_LENGTH) + 0];
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 1] = pVertices[(iVertex * VERTEX_LENGTH) + 1];
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 2] = pVertices[(iVertex * VERTEX_LENGTH) + 2];
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 3] = pVertices[(iVertex * VERTEX_LENGTH) + 3];
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 4] = pVertices[(iVertex * VERTEX_LENGTH) + 4];
			pIFCObject->vertices()[(iVertex * GEOMETRY_VBO_VERTEX_LENGTH) + 5] = pVertices[(iVertex * VERTEX_LENGTH) + 5];
		} // for (int_t iVertex = ...

		delete[] pVertices;
		delete[] pIndices;
	} // if ((iVerticesCount > 0) && ...	

	/*
	* Restore circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(m_iIFCModel, 0);

	return pIFCObject;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadUnits()
{
	int64_t * iIFCProjectInstances = sdaiGetEntityExtentBN(m_iIFCModel, (char *)"IFCPROJECT");

	int64_t iIFCProjectInstancesCount = sdaiGetMemberCount(iIFCProjectInstances);
	if (iIFCProjectInstancesCount > 0)
	{
		int64_t	iIFCProjectInstance = 0;
		engiGetAggrElement(iIFCProjectInstances, 0, sdaiINSTANCE, &iIFCProjectInstance);

		CIFCUnit::LoadUnits(m_iIFCModel, iIFCProjectInstance, m_mapUnits);
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadEntities()
{
	int_t iEntitiesCount = engiGetEntityCount(m_iIFCModel);

	/*
	* Retrieve the Entities
	*/
	int_t i = 0;
	while (i < iEntitiesCount) 
	{		
		int_t iEntity = engiGetEntityElement(m_iIFCModel, i);
		int_t iAttributesCount = engiGetEntityNoArguments(iEntity);
		int_t iInstancesCount = sdaiGetMemberCount(sdaiGetEntityExtent(m_iIFCModel, iEntity));

		CIFCEntity * pIFCEntity = new CIFCEntity(m_iIFCModel, iEntity, iAttributesCount, iInstancesCount);
		ASSERT(m_mapEntities.find(iEntity) == m_mapEntities.end());
		m_mapEntities[iEntity] = pIFCEntity;

		i++;
	}

	/*
	* Connect the Entities
	*/
	map<int_t, CIFCEntity *>::iterator itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		int_t iParentEntity = engiGetEntityParent(itEntities->first);
		if (iParentEntity == 0)
		{
			continue;
		}		

		map<int_t, CIFCEntity *>::iterator itParentEntity = m_mapEntities.find(iParentEntity);
		ASSERT(itParentEntity != m_mapEntities.end());

		itEntities->second->setParent(itParentEntity->second);

		itParentEntity->second->addSubType(itEntities->second);
	} // for (; itEntities != ...	

	/*
	* Post-processing
	*/
	itEntities = m_mapEntities.begin();
	for (; itEntities != m_mapEntities.end(); itEntities++)
	{
		itEntities->second->postProcessing();
	} // for (; itEntities != ...	
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadClasses()
{
	int64_t	iClassInstance = GetClassesByIterator(m_iIFCModel, 0);
	while (iClassInstance != 0)
	{
		m_mapClasses[iClassInstance] = new CIFCClass(iClassInstance);

		iClassInstance = GetClassesByIterator(m_iIFCModel, iClassInstance);
	} // while (iClassInstance != 0)
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::LoadProperties()
{
	int64_t iPropertyInstance = GetPropertiesByIterator(m_iIFCModel, 0);
	while (iPropertyInstance != 0)
	{
		int64_t iPropertyType = GetPropertyType(iPropertyInstance);
		switch (iPropertyType)
		{
		case TYPE_OBJECTTYPE:
		{
			m_mapProperties[iPropertyInstance] = new CObjectIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_BOOL_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CBoolIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_CHAR_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CStringIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_INT_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CIntIFCProperty(iPropertyInstance);
		}
		break;

		case TYPE_DOUBLE_DATATYPE:
		{
			m_mapProperties[iPropertyInstance] = new CDoubleIFCProperty(iPropertyInstance);
		}
		break;

		default:
			ASSERT(FALSE);
			break;
		} // switch (iPropertyType)

		map<int64_t, CIFCClass *>::iterator itIFCClasses = m_mapClasses.begin();
		for (; itIFCClasses != m_mapClasses.end(); itIFCClasses++)
		{
			int64_t	iMinCard = 0;
			int64_t iMaxCard = 0;
			GetPropertyRestrictions(itIFCClasses->first, iPropertyInstance, &iMinCard, &iMaxCard);

			if ((iMinCard == -1) && (iMaxCard == -1))
			{
				continue;
			}

			itIFCClasses->second->AddPropertyRestriction(new CIFCPropertyRestriction(iPropertyInstance, iMinCard, iMaxCard));
		} // for (; itIFCClasses != ...

		iPropertyInstance = GetPropertiesByIterator(m_iIFCModel, iPropertyInstance);
	} // while (iPropertyInstance != 0)
}

