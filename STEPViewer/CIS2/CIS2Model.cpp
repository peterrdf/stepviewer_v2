#include "stdafx.h"
#include "CIS2Model.h"
#include <_3DUtils.h>

// ************************************************************************************************
#define DEFAULT_CIRCLE_SEGMENTS 36

// ************************************************************************************************
/*static*/ int_t CCIS2Model::s_iInstanceID = 1;

// ************************************************************************************************
static uint32_t DEFAULT_COLOR_R = 10;
static uint32_t DEFAULT_COLOR_G = 150;
static uint32_t DEFAULT_COLOR_B = 10;
static uint32_t DEFAULT_COLOR_A = 255;
/*static*/ uint32_t CCIS2Model::DEFAULT_COLOR =
256 * 256 * 256 * DEFAULT_COLOR_R +
256 * 256 * DEFAULT_COLOR_G +
256 * DEFAULT_COLOR_B +
DEFAULT_COLOR_A;

// ************************************************************************************************
CCIS2Model::CCIS2Model()
	: CModel(enumModelType::CIS2)
	, m_vecInstances()
	, m_mapInstances()
	, m_mapID2Instance()
	, m_mapExpressID2Instance()
	, m_pEntityProvider(nullptr)
	, m_bUpdteVertexBuffers(true)
{}

/*virtual*/ CCIS2Model::~CCIS2Model()
{
	Clean();
}

/*virtual*/ CInstanceBase* CCIS2Model::GetInstanceByExpressID(ExpressID iExpressID) const /*override*/
{
	auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
	if (itExpressID2Instance != m_mapExpressID2Instance.end())
	{
		return itExpressID2Instance->second;
	}

	return nullptr;
}

/*virtual*/ void CCIS2Model::ZoomToInstance(CInstanceBase* pInstance) /*override*/
{
	ASSERT(pInstance != nullptr);
	
	auto pCIS2Representation = dynamic_cast<CCIS2Representation*>(pInstance);
	if (pCIS2Representation == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pCIS2Representation->GetInstance()) != m_mapInstances.end());

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

	pCIS2Representation->CalculateMinMax(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);

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

/*virtual*/ void CCIS2Model::ZoomOut() /*override*/
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
		if (!itInstance->second->getEnable())
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

/*virtual*/ CInstanceBase* CCIS2Model::LoadInstance(OwlInstance iInstance) /*override*/
{
	ASSERT(iInstance != 0);
	ASSERT(FALSE); //#todo
	m_bUpdteVertexBuffers = true;

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	m_mapInstances.clear();
	m_mapID2Instance.clear();
	m_mapExpressID2Instance.clear();

	auto pInstance = RetrieveGeometry((SdaiInstance)iInstance, DEFAULT_CIRCLE_SEGMENTS);
	pInstance->setEnable(true);

	m_vecInstances.push_back(pInstance);
	m_mapInstances[(SdaiInstance)iInstance] = pInstance;

	// Helper data structures
	m_mapID2Instance[pInstance->getID()] = pInstance;
	m_mapExpressID2Instance[pInstance->ExpressID()] = pInstance;

	// Scale
	Scale();

	return pInstance;
}

void CCIS2Model::Load(const wchar_t* szCIS2File, SdaiModel iModel)
{
	ASSERT(szCIS2File != nullptr);
	ASSERT(iModel != 0);

	/*
	* Memory
	*/
	Clean();

	/*
	* Model
	*/
	m_iModel = iModel;
	m_strPath = szCIS2File;

	// TEST
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	int_t* cis2AnalysisModel3DInstances = sdaiGetEntityExtentBN(iModel, "ANALYSIS_MODEL_3D"),
		noCis2AnalysisModel3DInstances = sdaiGetMemberCount(cis2AnalysisModel3DInstances);

	//
	//	Physical model (Design Part)
	//
	{
		int_t* cis2DesignPartInstances = sdaiGetEntityExtentBN(iModel, "DESIGN_PART"),
			noCis2DesignPartInstances = sdaiGetMemberCount(cis2DesignPartInstances);

		for (int_t i = 0; i < noCis2DesignPartInstances; i++)
		{
			SdaiInstance iProductDefinitionInstance = 0;
			sdaiGetAggrByIndex(cis2DesignPartInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);

			ASSERT(iProductDefinitionInstance != 0);

			_vector3d vecOriginalBBMin;
			_vector3d vecOriginalBBMax;
			if (GetInstanceGeometryClass(iProductDefinitionInstance) &&
				GetBoundingBox(
					iProductDefinitionInstance,
					(double*)&vecOriginalBBMin,
					(double*)&vecOriginalBBMax))
			{
				TRACE(L"\n*** DESIGN_PART *** => MIN/MAX (x/y/z): %.16f, %.16f, %.16f - %.16f, %.16f, %.16f",
					vecOriginalBBMin.x, vecOriginalBBMin.y, vecOriginalBBMin.z,
					vecOriginalBBMax.x, vecOriginalBBMax.y, vecOriginalBBMax.z);
			}
		}
	}

	//
	//	Physical model (Representation)
	//
	{
		int_t* cis2RepresentationInstances = sdaiGetEntityExtentBN(iModel, "REPRESENTATION"),
			noCis2RepresentationInstances = sdaiGetMemberCount(cis2RepresentationInstances);
		for (int_t i = 0; i < noCis2RepresentationInstances; i++)
		{
			SdaiInstance iProductDefinitionInstance = 0;
			sdaiGetAggrByIndex(cis2RepresentationInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);

			ASSERT(iProductDefinitionInstance != 0);

			_vector3d vecOriginalBBMin;
			_vector3d vecOriginalBBMax;
			if (GetInstanceGeometryClass(iProductDefinitionInstance) &&
				GetBoundingBox(
					iProductDefinitionInstance,
					(double*)&vecOriginalBBMin,
					(double*)&vecOriginalBBMax))
			{
				TRACE(L"\n*** REPRESENTATION *** => MIN/MAX (x/y/z): %.16f, %.16f, %.16f - %.16f, %.16f, %.16f",
					vecOriginalBBMin.x, vecOriginalBBMin.y, vecOriginalBBMin.z,
					vecOriginalBBMax.x, vecOriginalBBMax.y, vecOriginalBBMax.z);
			}
		}
	}


	TRACE(L"");
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Entities
	m_pEntityProvider = new CEntityProvider(GetInstance());
}

void CCIS2Model::PreLoadInstance(SdaiInstance iInstance)
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

void CCIS2Model::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel((SdaiModel)m_iModel);
		m_iModel = 0;
	}

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	m_mapInstances.clear();
	m_mapID2Instance.clear();
	m_mapExpressID2Instance.clear();

	delete m_pEntityProvider;
	m_pEntityProvider = nullptr;
}

void CCIS2Model::Scale()
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

	TRACE(L"\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);

	/* Scale */
	itIinstance = m_mapInstances.begin();
	for (; itIinstance != m_mapInstances.end(); itIinstance++)
	{
		itIinstance->second->Scale(m_fBoundingSphereDiameter / 2.f);
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
		if (itIinstance->second->getEnable())
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

	TRACE(L"\n*** Scale II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);

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

CCIS2Representation* CCIS2Model::GetInstanceByID(int64_t iID)
{
	auto itID2Instance = m_mapID2Instance.find(iID);
	if (itID2Instance != m_mapID2Instance.end())
	{
		return itID2Instance->second;
	}

	return nullptr;
}

CCIS2Representation* CCIS2Model::RetrieveGeometry(SdaiInstance iInstance, int_t iCircleSegments)
{
	PreLoadInstance(iInstance);

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	auto pInstance = new CCIS2Representation(s_iInstanceID++, iInstance);

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	cleanMemory(GetInstance(), 0);

	return pInstance;
}