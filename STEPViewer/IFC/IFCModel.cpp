#include "stdafx.h"

#include "_3DUtils.h"
#include "_oglUtils.h"
#include "IFCModel.h"

#include <cwchar>
#include <cfloat>
#include <math.h>

// ------------------------------------------------------------------------------------------------
/*static*/ int_t CIFCModel::s_iInstanceID = 1;

// ------------------------------------------------------------------------------------------------
CIFCModel::CIFCModel()
	: CModel(enumModelType::IFC)
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
	, m_vecInstances()
	, m_mapInstances()
	, m_mapID2Instance()
	, m_mapExpressID2Instance()
	, m_pUnitProvider(nullptr)
	, m_pPropertyProvider(nullptr)
	, m_pEntityProvider(nullptr)
	, m_bUpdteVertexBuffers(true)
{
}

// ------------------------------------------------------------------------------------------------
CIFCModel::~CIFCModel()
{
	Clean();
}

void CIFCModel::PreLoadInstance(SdaiInstance iInstance)
{
	if (m_bUpdteVertexBuffers)
	{
		_vector3d vecOriginalBBMin;
		_vector3d vecOriginalBBMax;
		if (GetInstanceGeometryClass(iInstance) &&
			GetBoundingBox(
				iInstance,
				(double*)&vecOriginalBBMin,
				(double*)&vecOriginalBBMax))
		{
			TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
			SetVertexBufferOffset(
				m_iModel,
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
			ClearedExternalBuffers(m_iModel);

			m_bUpdteVertexBuffers = false;
		}
	} // if (m_bUpdteVertexBuffers)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CEntityProvider* CIFCModel::GetEntityProvider() const /*override*/
{
	return m_pEntityProvider;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCModel::ZoomToInstance(CInstanceBase* pInstance) /*override*/
{
	ASSERT(pInstance != nullptr);

	auto pIFCInstance = dynamic_cast<CIFCInstance*>(pInstance);
	if (pIFCInstance == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pIFCInstance->GetInstance()) != m_mapInstances.end());

	m_fBoundingSphereDiameter = 0.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	pIFCInstance->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.;
		m_fXmax = 1.;
		m_fYmin = -1.;
		m_fYmax = 1.;
		m_fZmin = -1.;
		m_fZmax = 1.;
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXTranslation -= m_fXmin;
	m_fYTranslation -= m_fYmin;
	m_fZTranslation -= m_fZmin;

	// center
	m_fXTranslation -= ((m_fXmax - m_fXmin) / 2.0f);
	m_fYTranslation -= ((m_fYmax - m_fYmin) / 2.0f);
	m_fZTranslation -= ((m_fZmax - m_fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CIFCModel::ZoomOut() /*override*/
{
	m_fBoundingSphereDiameter = 0.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	auto itInstance = m_mapInstances.begin();
	for (; itInstance != m_mapInstances.end(); itInstance++)
	{
		if (!itInstance->second->GetEnable())
		{
			continue;
		}

		itInstance->second->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.;
		m_fXmax = 1.;
		m_fYmin = -1.;
		m_fYmax = 1.;
		m_fZmin = -1.;
		m_fZmax = 1.;
	}

	/*
	* World
	*/
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXTranslation -= m_fXmin;
	m_fYTranslation -= m_fYmin;
	m_fZTranslation -= m_fZmin;

	// center
	m_fXTranslation -= ((m_fXmax - m_fXmin) / 2.0f);
	m_fYTranslation -= ((m_fYmax - m_fYmin) / 2.0f);
	m_fZTranslation -= ((m_fZmax - m_fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);
}


// ------------------------------------------------------------------------------------------------
void CIFCModel::Load(const wchar_t* szIFCFile, SdaiModel iModel)
{
	ASSERT(szIFCFile != nullptr);
	ASSERT(iModel != 0);

	/*
	* Memory
	*/
	Clean();

	/*
	* Model
	*/
	m_iModel = iModel;
	m_strFilePath = szIFCFile;

	/*
	* Entities
	*/
	SdaiEntity ifcObjectEntity = sdaiGetEntity(m_iModel, "IFCOBJECT");
	m_ifcProjectEntity = sdaiGetEntity(m_iModel, "IFCPROJECT");
	m_ifcSpaceEntity = sdaiGetEntity(m_iModel, "IFCSPACE");
	m_ifcOpeningElementEntity = sdaiGetEntity(m_iModel, "IFCOPENINGELEMENT");
	m_ifcDistributionElementEntity = sdaiGetEntity(m_iModel, "IFCDISTRIBUTIONELEMENT");
	m_ifcElectricalElementEntity = sdaiGetEntity(m_iModel, "IFCELECTRICALELEMENT");
	m_ifcElementAssemblyEntity = sdaiGetEntity(m_iModel, "IFCELEMENTASSEMBLY");
	m_ifcElementComponentEntity = sdaiGetEntity(m_iModel, "IFCELEMENTCOMPONENT");
	m_ifcEquipmentElementEntity = sdaiGetEntity(m_iModel, "IFCEQUIPMENTELEMENT");
	m_ifcFeatureElementEntity = sdaiGetEntity(m_iModel, "IFCFEATUREELEMENT");
	m_ifcFeatureElementSubtractionEntity = sdaiGetEntity(m_iModel, "IFCFEATUREELEMENTSUBTRACTION");
	m_ifcFurnishingElementEntity = sdaiGetEntity(m_iModel, "IFCFURNISHINGELEMENT");
	m_ifcReinforcingElementEntity = sdaiGetEntity(m_iModel, "IFCREINFORCINGELEMENT");
	m_ifcTransportElementEntity = sdaiGetEntity(m_iModel, "IFCTRANSPORTELEMENT");
	m_ifcVirtualElementEntity = sdaiGetEntity(m_iModel, "IFCVIRTUALELEMENT");

	/*
	* Retrieve the objects recursively
	*/
	RetrieveObjects__depth(ifcObjectEntity, DEFAULT_CIRCLE_SEGMENTS, 0);	
	//#test
	RetrieveObjects("IFCPROJECT", L"IFCPROJECT", DEFAULT_CIRCLE_SEGMENTS);
	RetrieveObjects("IFCRELSPACEBOUNDARY", L"IFCRELSPACEBOUNDARY", DEFAULT_CIRCLE_SEGMENTS);

	/*
	* Units
	*/
	m_pUnitProvider = new CIFCUnitProvider(m_iModel);

	/*
	* Properties
	*/
	m_pPropertyProvider = new CIFCPropertyProvider(m_iModel, m_pUnitProvider);

	/*
	* Entities
	*/
	m_pEntityProvider = new CEntityProvider(m_iModel);

	/*
	* Helper data structures
	*/
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->ID()] = pInstance;
		m_mapExpressID2Instance[pInstance->ExpressID()] = pInstance;
	}

	/**
	* Scale and Center
	*/

	ScaleAndCenter();
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel(m_iModel);
		m_iModel = 0;
	}

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	delete m_pUnitProvider;
	m_pUnitProvider = nullptr;

	delete m_pPropertyProvider;
	m_pPropertyProvider = nullptr;

	delete m_pEntityProvider;
	m_pEntityProvider = nullptr;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::ScaleAndCenter()
{
	/* World */
	m_dOriginalBoundingSphereDiameter = 2.;
	m_fBoundingSphereDiameter = 2.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	/* Min/Max */
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	auto itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->CalculateMinMax(
			m_fXmin, m_fXmax, 
			m_fYmin, m_fYmax, 
			m_fZmin, m_fZmax);
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		// TODO: new status bar for geometry
		/*::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);*/

		return;
	}

	/* World */
	m_dOriginalBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fZmax - m_fZmin);

	m_fBoundingSphereDiameter = m_dOriginalBoundingSphereDiameter;

	TRACE(L"\n*** Scale and Center I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);

	/* Scale and Center */
	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->ScaleAndCenter(
			m_fXmin, m_fXmax, 
			m_fYmin, m_fYmax, 
			m_fZmin, m_fZmax, 
			m_fBoundingSphereDiameter, true);
	}

	/* Min/Max */
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		if (itIinstance->second->GetEnable())
		{
			itIinstance->second->CalculateMinMax(
				m_fXmin, m_fXmax, 
				m_fYmin, m_fYmax, 
				m_fZmin, m_fZmax);
		}
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		// TODO: new status bar for geometry
		/*::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);*/

		return;
	}

	/* World */
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale and Center II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXTranslation -= m_fXmin;
	m_fYTranslation -= m_fYmin;
	m_fZTranslation -= m_fZmin;

	// center
	m_fXTranslation -= ((m_fXmax - m_fXmin) / 2.0f);
	m_fYTranslation -= ((m_fYmax - m_fYmin) / 2.0f);
	m_fZTranslation -= ((m_fZmax - m_fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);
}


// ------------------------------------------------------------------------------------------------
const map<SdaiInstance, CIFCInstance *>& CIFCModel::GetInstances() const
{
	return m_mapInstances;
}

// ------------------------------------------------------------------------------------------------
CIFCUnitProvider* CIFCModel::GetUnitProvider() const
{
	return m_pUnitProvider;
}

// ------------------------------------------------------------------------------------------------
CIFCPropertyProvider* CIFCModel::GetPropertyProvider() const
{
	return m_pPropertyProvider;
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::GetInstanceByID(int_t iID)
{
	auto itID2Instance = m_mapID2Instance.find(iID);
	if (itID2Instance != m_mapID2Instance.end())
	{
		return itID2Instance->second;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CInstanceBase* CIFCModel::GetInstanceByExpressID(int64_t iExpressID) const /*override*/
{
	auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
	if (itExpressID2Instance != m_mapExpressID2Instance.end())
	{
		return itExpressID2Instance->second;
	}

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::GetInstancesByType(const wchar_t* szType, vector<CIFCInstance*>& vecInstances)
{
	vecInstances.clear();

	CString strTargetType = szType;
	strTargetType.MakeUpper();

	for (auto pInstance : m_vecInstances)
	{
		CString strType = pInstance->GetEntityName();
		strType.MakeUpper();

		if (strType == strTargetType)
		{
			vecInstances.push_back(pInstance);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CIFCModel::RetrieveObjects(const char * szEntityName, const wchar_t * szEntityNameW, int_t iCircleSegements)
{
	SdaiAggr iIFCInstances = sdaiGetEntityExtentBN(m_iModel, (char *) szEntityName);

	int_t iIFCInstancesCount = sdaiGetMemberCount(iIFCInstances);
	if (iIFCInstancesCount == 0)
    {
        return;
    }	

	for (int_t i = 0; i < iIFCInstancesCount; ++i)
	{
		SdaiInstance iInstance = 0;
		engiGetAggrElement(iIFCInstances, i, sdaiINSTANCE, &iInstance);

		wchar_t* szInstanceGUIDW = nullptr;
		sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

		auto pInstance = RetrieveGeometry(szInstanceGUIDW, iInstance, iCircleSegements);
		pInstance->ID() = s_iInstanceID++;

		CString strEntity = szEntityNameW;
		strEntity.MakeUpper();

		pInstance->SetEnable((strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT"));
		
		m_vecInstances.push_back(pInstance);
		m_mapInstances[iInstance] = pInstance;
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

	int_t* piInstances = sdaiGetEntityExtent(m_iModel, iParentEntity);
	int_t iIntancesCount = sdaiGetMemberCount(piInstances);

	if (iIntancesCount != 0)
	{
		char* szParenEntityName = nullptr;
		engiGetEntityName(iParentEntity, sdaiSTRING, (const char**)&szParenEntityName);

		wchar_t* szParentEntityNameW = CEntity::GetName(iParentEntity);

		RetrieveObjects(szParenEntityName, szParentEntityNameW, iCircleSegments);

		//#test
		///*if (iParentEntity == m_ifcProjectEntity) {
		//	for (int_t i = 0; i < iIntancesCount; i++) {
		//		SdaiInstance iInstance = 0;
		//		engiGetAggrElement(piInstances, i, sdaiINSTANCE, &iInstance);

		//		wchar_t* szInstanceGUIDW = nullptr;
		//		sdaiGetAttrBN(iInstance, "GlobalId", sdaiUNICODE, &szInstanceGUIDW);

		//		CIFCInstance * pInstance = RetrieveGeometry(szInstanceGUIDW, iInstance, iCircleSegments);
		//		pInstance->ID() = s_iInstanceID++;

		//		CString strEntity = szParentEntityNameW;
		//		strEntity.MakeUpper();

		//		pInstance->SetEnable((strEntity != "IFCSPACE") && (strEntity != "IFCRELSPACEBOUNDARY") && (strEntity != "IFCOPENINGELEMENT"));

		//		m_vecInstances.push_back(pInstance);
		//		m_mapInstances[iInstance] = pInstance;
		//	}
		//}*/
	} // if (iIntancesCount != 0)

	iIntancesCount = engiGetEntityCount(m_iModel);
	for (int_t i = 0; i < iIntancesCount; i++)
	{
		SdaiEntity iEntity = engiGetEntityElement(m_iModel, i);
		if (engiGetEntityParent(iEntity) == iParentEntity)
		{
			RetrieveObjects__depth(iEntity, iCircleSegments, depth + 1);
		}
	}
}

// ------------------------------------------------------------------------------------------------
CIFCInstance* CIFCModel::RetrieveGeometry(const wchar_t* szInstanceGUIDW, SdaiInstance iInstance, int_t iCircleSegments)
{
	PreLoadInstance(iInstance);

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
	mask += flagbit13;       // CONCEPTUAL FACE POLYGONS
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
	setting += flagbit13;    // CONCEPTUAL FACE POLYGONS ON
	setting += 0;		     // OPENGL
	setting += flagbit24;	 //	AMBIENT
	setting += flagbit25;	 //	DIFFUSE
	setting += flagbit26;	 //	EMISSIVE
	setting += flagbit27;	 //	SPECULAR
	setFormat(m_iModel, setting, mask);
	setFilter(m_iModel, flagbit1, flagbit1);

	/*
	* Default color
	*/
	int32_t	iR = 10,
		iG = 150,
		iB = 10,
		iA = 255;
	int32_t	iDefaultColor = 256 * 256 * 256 * iR +
		256 * 256 * iG +
		256 * iB +
		iA;
	SetDefaultColor(m_iModel, iDefaultColor, iDefaultColor, iDefaultColor, iDefaultColor);

	setSegmentation(m_iModel, 16, 0.);

	/*
	* Set up circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	auto pInstance = new CIFCInstance(this, iInstance, szInstanceGUIDW);

	ASSERT(pInstance->m_pVertexBuffer == nullptr);
	pInstance->m_pVertexBuffer = new _vertices_f(_VERTEX_LENGTH);

	ASSERT(pInstance->m_pIndexBuffer == nullptr);
	pInstance->m_pIndexBuffer = new _indices_i32();

	CalculateInstance(iInstance, &pInstance->m_pVertexBuffer->size(), &pInstance->m_pIndexBuffer->size(), nullptr);
	if ((pInstance->m_pVertexBuffer->size() == 0) || (pInstance->m_pIndexBuffer->size() == 0))
	{
		return pInstance;
	}

	OwlModel iOWLModel = 0;
	owlGetModel(m_iModel, &iOWLModel);

	int64_t iOWLInstance = 0;
	owlGetInstance(m_iModel, iInstance, &iOWLInstance);

	/**
	* Retrieves the vertices
	*/
	pInstance->m_pVertexBuffer->data() = new float[(uint32_t)pInstance->m_pVertexBuffer->size() * pInstance->m_pVertexBuffer->getVertexLength()];
	memset(pInstance->m_pVertexBuffer->data(), 0, (uint32_t)pInstance->m_pVertexBuffer->size() * pInstance->m_pVertexBuffer->getVertexLength() * sizeof(float));

	UpdateInstanceVertexBuffer(iOWLInstance, pInstance->m_pVertexBuffer->data());

	/**
	* Retrieves the indices
	*/
	pInstance->m_pIndexBuffer->data() = new int32_t[(uint32_t)pInstance->m_pIndexBuffer->size()];
	memset(pInstance->m_pIndexBuffer->data(), 0, (uint32_t)pInstance->m_pIndexBuffer->size() * sizeof(int32_t));

	UpdateInstanceIndexBuffer(iOWLInstance, pInstance->m_pIndexBuffer->data());

	// MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.
	MATERIALS mapMaterial2ConcFaces;
	MATERIALS mapMaterial2ConcFacePoints; // MATERIAL : FACE INDEX, START INDEX, INIDCES COUNT, etc.

	//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceCnt.html
	pInstance->m_iConceptualFacesCount = GetConceptualFaceCnt(iInstance);
	for (int64_t iConceptualFace = 0; iConceptualFace < pInstance->m_iConceptualFacesCount; iConceptualFace++)
	{
		//	http://rdf.bg/gkdoc/CP64/GetConceptualFaceEx.html
		int64_t iStartIndexTriangles = 0;
		int64_t iIndicesCountTriangles = 0;
		int64_t iStartIndexLines = 0;
		int64_t iIndicesCountLines = 0;
		int64_t iStartIndexPoints = 0;
		int64_t iIndicesCountPoints = 0;
		int64_t iStartIndexConceptualFacePolygons = 0;
		int64_t iIndicesCountConceptualFacePolygons = 0;
		GetConceptualFaceEx(iInstance, iConceptualFace,
			&iStartIndexTriangles, &iIndicesCountTriangles,
			&iStartIndexLines, &iIndicesCountLines,
			&iStartIndexPoints, &iIndicesCountPoints,
			0, 0,
			&iStartIndexConceptualFacePolygons, &iIndicesCountConceptualFacePolygons);

		if (iIndicesCountTriangles > 0)
		{
			/*
			* Material
			*/
			int32_t iIndexValue = *(pInstance->m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= _VERTEX_LENGTH;

			float fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConceptualFaces = mapMaterial2ConcFaces.find(material);
			if (itMaterial2ConceptualFaces == mapMaterial2ConcFaces.end())
			{
				mapMaterial2ConcFaces[material] = vector<_face>{ _face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles) };
			}
			else
			{
				itMaterial2ConceptualFaces->second.push_back(_face(iConceptualFace, iStartIndexTriangles, iIndicesCountTriangles));
			}
		}

		if (iIndicesCountTriangles > 0)
		{
			pInstance->m_vecTriangles.push_back(_primitives(iStartIndexTriangles, iIndicesCountTriangles));
		}

		if (iIndicesCountConceptualFacePolygons > 0)
		{
			pInstance->m_vecConcFacePolygons.push_back(_primitives(iStartIndexConceptualFacePolygons, iIndicesCountConceptualFacePolygons));
		}

		if (iIndicesCountLines > 0)
		{
			pInstance->m_vecLines.push_back(_primitives(iStartIndexLines, iIndicesCountLines));
		}

		if (iIndicesCountPoints > 0)
		{
			int32_t iIndexValue = *(pInstance->m_pIndexBuffer->data() + iStartIndexTriangles);
			iIndexValue *= _VERTEX_LENGTH;

			float fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 6);
			unsigned int iAmbientColor = *(reinterpret_cast<unsigned int*>(&fColor));
			float fTransparency = (float)(iAmbientColor & (255)) / (float)255;

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 7);
			unsigned int iDiffuseColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 8);
			unsigned int iEmissiveColor = *(reinterpret_cast<unsigned int*>(&fColor));

			fColor = *(pInstance->m_pVertexBuffer->data() + iIndexValue + 9);
			unsigned int iSpecularColor = *(reinterpret_cast<unsigned int*>(&fColor));

			/*
			* Material
			*/
			_material material(
				iAmbientColor,
				iDiffuseColor,
				iEmissiveColor,
				iSpecularColor,
				fTransparency,
				nullptr);

			auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.find(material);
			if (itMaterial2ConcFacePoints == mapMaterial2ConcFacePoints.end())
			{
				mapMaterial2ConcFacePoints[material] = vector<_face>{ _face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints) };
			}
			else
			{
				itMaterial2ConcFacePoints->second.push_back(_face(iConceptualFace, iStartIndexPoints, iIndicesCountPoints));
			}

			pInstance->m_vecPoints.push_back(_primitives(iStartIndexPoints, iIndicesCountPoints));
		} // if (iIndicesCountPoints > 0)
	} // for (int64_t iConceptualFace = ...	

	/*
	* Group the faces
	*/
	auto itMaterial2ConcFaces = mapMaterial2ConcFaces.begin();
	for (; itMaterial2ConcFaces != mapMaterial2ConcFaces.end(); itMaterial2ConcFaces++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFaces->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFaces->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->ConcFacesCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->ConcFacesCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);

				pInstance->ConcFacesCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFaces->first);

				pInstance->ConcFacesCohorts().push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...

	/*
	* Group the polygons
	*/
	if (!pInstance->m_vecConcFacePolygons.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		_cohort* pCohort = pInstance->ConcFacePolygonsCohorts().empty() ? 
			nullptr : pInstance->ConcFacePolygonsCohorts()[pInstance->ConcFacePolygonsCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
		{
			pCohort = new _cohort();
			pInstance->ConcFacePolygonsCohorts().push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < pInstance->m_vecConcFacePolygons.size(); iFace++)
		{
			int64_t iStartIndex = pInstance->m_vecConcFacePolygons[iFace].startIndex();
			int64_t iIndicesCount = pInstance->m_vecConcFacePolygons[iFace].indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit() / 2)
				{
					pCohort = new _cohort();
					pInstance->ConcFacePolygonsCohorts().push_back(pCohort);

					int64_t iPreviousIndex = -1;
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit() / 2;
						iIndex++)
					{
						if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...

					iIndicesCount -= _oglUtils::getIndicesCountLimit() / 2;
					iStartIndex += _oglUtils::getIndicesCountLimit() / 2;
				} // while (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

				if (iIndicesCount > 0)
				{
					pCohort = new _cohort();
					pInstance->ConcFacePolygonsCohorts().push_back(pCohort);

					int64_t iPreviousIndex = -1;
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
						{
							iPreviousIndex = -1;

							continue;
						}

						if (iPreviousIndex != -1)
						{
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
							pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
						} // if (iPreviousIndex != -1)

						iPreviousIndex = iIndex;
					} // for (int_t iIndex = ...
				}

				continue;
			} // if (iIndicesCount > _oglUtils::GetIndicesCountLimit() / 2)

			/*
			* Check the limit
			*/
			if ((pCohort->indices().size() + (iIndicesCount * 2)) > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				pInstance->ConcFacePolygonsCohorts().push_back(pCohort);
			}

			int64_t iPreviousIndex = -1;
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
				{
					iPreviousIndex = -1;

					continue;
				}

				if (iPreviousIndex != -1)
				{
					pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iPreviousIndex]);
					pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
				} // if (iPreviousIndex != -1)

				iPreviousIndex = iIndex;
			} // for (int_t iIndex = ...
		} // for (size_t iFace = ...
	} // if (!m_vecConcFacePolygons.empty())

	/*
	* Group the lines
	*/
	if (!pInstance->m_vecLines.empty())
	{
		/*
		* Use the last cohort (if any)
		*/
		auto pCohort = pInstance->LinesCohorts().empty() ? 
			nullptr : pInstance->LinesCohorts()[pInstance->LinesCohorts().size() - 1];

		/*
		* Create the cohort
		*/
		if (pCohort == nullptr)
		{
			pCohort = new _cohort();
			pInstance->LinesCohorts().push_back(pCohort);
		}

		for (size_t iFace = 0; iFace < pInstance->m_vecLines.size(); iFace++)
		{
			int64_t iStartIndex = pInstance->m_vecLines[iFace].startIndex();
			int64_t iIndicesCount = pInstance->m_vecLines[iFace].indicesCount();

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohort();
				pInstance->LinesCohorts().push_back(pCohort);
			}

			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				if (pInstance->m_pIndexBuffer->data()[iIndex] < 0)
				{
					continue;
				}

				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			} // for (int64_t iIndex = ...
		} // for (size_t iFace = ...
	} // if (!m_vecLines.empty())		

	/*
	* Group the points
	*/
	auto itMaterial2ConcFacePoints = mapMaterial2ConcFacePoints.begin();
	for (; itMaterial2ConcFacePoints != mapMaterial2ConcFacePoints.end(); itMaterial2ConcFacePoints++)
	{
		_cohortWithMaterial* pCohort = nullptr;

		for (size_t iConcFace = 0; iConcFace < itMaterial2ConcFacePoints->second.size(); iConcFace++)
		{
			_face& concFace = itMaterial2ConcFacePoints->second[iConcFace];

			int64_t iStartIndex = concFace.startIndex();
			int64_t iIndicesCount = concFace.indicesCount();

			/*
			* Split the conceptual face - isolated case
			*/
			if (iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				while (iIndicesCount > _oglUtils::getIndicesCountLimit())
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + _oglUtils::getIndicesCountLimit();
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->PointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);

					iIndicesCount -= _oglUtils::getIndicesCountLimit();
					iStartIndex += _oglUtils::getIndicesCountLimit();
				}

				if (iIndicesCount > 0)
				{
					auto pNewCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);
					for (int64_t iIndex = iStartIndex;
						iIndex < iStartIndex + iIndicesCount;
						iIndex++)
					{
						pNewCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
					}

					pInstance->PointsCohorts().push_back(pNewCohort);

					/*
					* Update Conceptual face start index
					*/
					concFace.startIndex() = 0;

					// Conceptual faces
					pNewCohort->faces().push_back(concFace);
				}

				continue;
			} // if (iIndicesCountTriangles > _oglUtils::GetIndicesCountLimit())	

			/*
			* Create material
			*/
			if (pCohort == nullptr)
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);

				pInstance->PointsCohorts().push_back(pCohort);
			}

			/*
			* Check the limit
			*/
			if (pCohort->indices().size() + iIndicesCount > _oglUtils::getIndicesCountLimit())
			{
				pCohort = new _cohortWithMaterial(itMaterial2ConcFacePoints->first);

				pInstance->PointsCohorts().push_back(pCohort);
			}

			/*
			* Update Conceptual face start index
			*/
			concFace.startIndex() = pCohort->indices().size();

			/*
			* Add the indices
			*/
			for (int64_t iIndex = iStartIndex;
				iIndex < iStartIndex + iIndicesCount;
				iIndex++)
			{
				pCohort->indices().push_back(pInstance->m_pIndexBuffer->data()[iIndex]);
			}

			// Conceptual faces
			pCohort->faces().push_back(concFace);
		} // for (size_t iConcFace = ...
	} // for (; itMaterial2ConceptualFaces != ...

	/*
	* Restore circleSegments()
	*/
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(m_iModel, 0);

	return pInstance;
}
