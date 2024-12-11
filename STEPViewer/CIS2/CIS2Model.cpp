#include "stdafx.h"
#include "CIS2Model.h"
#include <_3DUtils.h>

// ************************************************************************************************
//#todo???
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
CCIS2Model::CCIS2Model(bool bLoadInstancesOnDemand/* = false*/)
	: _ap_model(enumAP::CIS2)
	, m_bLoadInstancesOnDemand(bLoadInstancesOnDemand)
	, m_vecInstances()	
	, m_mapInstances()
	, m_mapID2Instance()
{}

/*virtual*/ CCIS2Model::~CCIS2Model()
{
	clean();
}

/*virtual*/ void CCIS2Model::attachModelCore() /*override*/
{
	// Objects & Unreferenced
	if (!m_bLoadInstancesOnDemand)
	{
		int_t* cis2AnalysisModel3DInstances = sdaiGetEntityExtentBN(getSdaiInstance(), "ANALYSIS_MODEL_3D"),
			noCis2AnalysisModel3DInstances = sdaiGetMemberCount(cis2AnalysisModel3DInstances);
		if (noCis2AnalysisModel3DInstances > 0)
		{
			ASSERT(FALSE); //#todo
		}

		//
		//	Physical model (Design Part)
		//
		LodDesignParts();

		//
		// Physical model (Representation)
		//
		LoadRepresentations();

		//GetObjectsReferencedState(); #todo?
	} // if (!m_bLoadInstancesOnDemand)



	// Helper data structures
	for (auto pInstance : m_vecInstances)
	{
		m_mapID2Instance[pInstance->getID()] = pInstance;
	}

	// Scale
	Scale();
}

/*virtual*/ void CCIS2Model::clean() /*override*/
{
	_ap_model::clean();

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();

	m_mapInstances.clear();
	m_mapID2Instance.clear();
}

/*virtual*/ void CCIS2Model::ZoomToInstance(_instance* pInstance) /*override*/
{
	ASSERT(pInstance != nullptr);
	
	auto pCIS2Representation = dynamic_cast<CCIS2Representation*>(pInstance);
	if (pCIS2Representation == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	ASSERT(m_mapInstances.find(pCIS2Representation->_ap_geometry::getSdaiInstance()) != m_mapInstances.end());

	m_fBoundingSphereDiameter = 0.f;

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
}

/*virtual*/ void CCIS2Model::ZoomOut() /*override*/
{
	m_fBoundingSphereDiameter = 0.f;

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	auto itInstance = m_mapInstances.begin();
	for (; itInstance != m_mapInstances.end(); itInstance++)
	{
		if (!itInstance->second->_instance::getEnable())
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
}

/*virtual*/ _instance* CCIS2Model::LoadInstance(OwlInstance /*iInstance*/) /*override*/
{
	ASSERT(FALSE); //#todo

	return nullptr;
}

void CCIS2Model::Scale()
{
	/* World */
	m_dOriginalBoundingSphereDiameter = 2.;
	m_fBoundingSphereDiameter = 2.f;

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
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	m_dOriginalBoundingSphereDiameter = m_fBoundingSphereDiameter;

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
		if (itIinstance->second->_instance::getEnable())
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
}

CCIS2Instance* CCIS2Model::GetInstanceByID(int64_t iID)
{
	auto itID2Instance = m_mapID2Instance.find(iID);
	if (itID2Instance != m_mapID2Instance.end())
	{
		return itID2Instance->second;
	}

	return nullptr;
}

void CCIS2Model::LodDesignParts()
{
	int_t* piInstances = sdaiGetEntityExtentBN(getSdaiInstance(), "DESIGN_PART");
	int_t iInstancesCount = sdaiGetMemberCount(piInstances);
	for (int_t i = 0; i < iInstancesCount; i++)
	{
		SdaiInstance iInstance = 0;
		sdaiGetAggrByIndex(piInstances, i, sdaiINSTANCE, &iInstance);
		ASSERT(iInstance != 0);

		auto pInstance = RetrieveGeometry(iInstance, enumCIS2InstanceType::DesignPart, DEFAULT_CIRCLE_SEGMENTS);
		pInstance->_instance::setEnable(true);

		m_vecInstances.push_back(pInstance);
		m_mapInstances[iInstance] = pInstance;
	}
}

void CCIS2Model::LoadRepresentations()
{
	int_t* piInstances = sdaiGetEntityExtentBN(getSdaiInstance(), "REPRESENTATION");
	int_t iInstancesCount = sdaiGetMemberCount(piInstances);
	for (int_t i = 0; i < iInstancesCount; i++)
	{
		SdaiInstance iInstance = 0;
		sdaiGetAggrByIndex(piInstances, i, sdaiINSTANCE, &iInstance);
		ASSERT(iInstance != 0);

		auto pInstance = RetrieveGeometry(iInstance, enumCIS2InstanceType::Reperesentation, DEFAULT_CIRCLE_SEGMENTS);
		pInstance->_instance::setEnable(true);

		m_vecInstances.push_back(pInstance);
		m_mapInstances[iInstance] = pInstance;
	}
}

CCIS2Instance* CCIS2Model::RetrieveGeometry(SdaiInstance iInstance, enumCIS2InstanceType enCIS2InstanceType, int_t iCircleSegments)
{
	preLoadInstance(iInstance);

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	CCIS2Instance* pInstance = nullptr;
	switch (enCIS2InstanceType)
	{
		case enumCIS2InstanceType::DesignPart:
		{
			pInstance = new CCIS2DesignPart(s_iInstanceID++, iInstance);
		}
		break;

		case enumCIS2InstanceType::Reperesentation:
		{
			pInstance = new CCIS2Representation(s_iInstanceID++, iInstance);
		}
		break;

		default:
		{
			ASSERT(FALSE);
		}
		break;
	}

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	return pInstance;
}