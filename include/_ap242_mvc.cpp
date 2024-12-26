#include "stdafx.h"
#include "_ap242_mvc.h"

// ************************************************************************************************
_ap242_model::_ap242_model()
	: _ap_model(enumAP::STEP)
	, m_vecDraughtingModels()
{
}

/*virtual*/ _ap242_model::~_ap242_model()
{
}

/*virtual*/ void _ap242_model::preLoadInstance(OwlInstance owlInstance) /*override*/
{
	//#todo
	//setVertexBufferOffset(owlInstance);
}

/*virtual*/ void _ap242_model::clean() /*override*/
{
	_ap_model::clean();

	for (auto pDraughtingModel : m_vecDraughtingModels)
	{
		delete pDraughtingModel;
	}
	m_vecDraughtingModels.clear();
}

void _ap242_model::loadDraughtingModels()
{
	SdaiAggr sdaiDraughtingModelAggr = xxxxGetEntityAndSubTypesExtentBN(getSdaiModel(), "DRAUGHTING_MODEL");
	assert(sdaiDraughtingModelAggr != nullptr);

	SdaiInteger iDraughtingModelsCount = sdaiGetMemberCount(sdaiDraughtingModelAggr);
	for (SdaiInteger i = 0; i < iDraughtingModelsCount; i++)
	{
		SdaiInstance sdaiDraughtingModelInstance = 0;
		sdaiGetAggrByIndex(sdaiDraughtingModelAggr, i, sdaiINSTANCE, &sdaiDraughtingModelInstance);
		assert(sdaiDraughtingModelInstance != 0);

		m_vecDraughtingModels.push_back(new _ap242_draughting_model(sdaiDraughtingModelInstance));
	} // for (SdaiInteger i = ...
}

// ************************************************************************************************
_ap242_geometry::_ap242_geometry(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
{
	calculate();
}

/*virtual*/ _ap242_geometry::~_ap242_geometry()
{
}

// ************************************************************************************************
_ap242_draughting_model::_ap242_draughting_model(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
	, m_strName(L"")
	, m_vecAnnotationPlanes()
	, m_vecDraughtingCallouts()
{
	assert(m_sdaiInstance != 0);

	SdaiAttr sdaiNameAttr = sdaiGetAttrDefinition(sdaiGetEntity(sdaiGetInstanceModel(m_sdaiInstance), "REPRESENTATION"), "name");

	wchar_t* szName = nullptr;
	sdaiGetAttr(m_sdaiInstance, sdaiNameAttr, sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";

	load();
}

/*virtual*/ _ap242_draughting_model::~_ap242_draughting_model()
{
	for (auto pAnnotationPlane : m_vecAnnotationPlanes)
	{
		delete pAnnotationPlane;
	}

	for (auto pDraughtingCallout : m_vecDraughtingCallouts)
	{
		delete pDraughtingCallout;
	}
}

void _ap242_draughting_model::load()
{
	SdaiModel sdaiModel = sdaiGetInstanceModel(m_sdaiInstance);
	assert(sdaiModel != 0);

	SdaiAttr sdaiItemsAttr = sdaiGetAttrDefinition(sdaiGetEntity(sdaiModel, "REPRESENTATION"), "items");
	assert(sdaiItemsAttr != nullptr);

	SdaiAggr sdaiItemsAggr = nullptr;
	sdaiGetAttr(m_sdaiInstance, sdaiItemsAttr, sdaiAGGR, &sdaiItemsAggr);
	SdaiInteger iItemsCount = sdaiGetMemberCount(sdaiItemsAggr);
	for (SdaiInteger j = 0; j < iItemsCount; j++)
	{
		SdaiInstance sdaiItemInstance = 0;
		sdaiGetAggrByIndex(sdaiItemsAggr, j, sdaiINSTANCE, &sdaiItemInstance);

		if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(sdaiModel, "ANNOTATION_PLANE"))
		{
			m_vecAnnotationPlanes.push_back(new _ap242_annotation_plane(sdaiItemInstance));
		}
		else if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(sdaiModel, "DRAUGHTING_CALLOUT")) 
		{
			m_vecDraughtingCallouts.push_back(new _ap242_draughting_callout(sdaiItemInstance));
		}
	}
}

// ************************************************************************************************
_ap242_annotation_plane::_ap242_annotation_plane(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
	, m_strName(L"")
{
	assert(m_sdaiInstance != 0);

	wchar_t* szName = 0;
	sdaiGetAttrBN(m_sdaiInstance, "name", sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";
}

/*virtual*/ _ap242_annotation_plane::~_ap242_annotation_plane()
{
}

// ************************************************************************************************
_ap242_draughting_callout::_ap242_draughting_callout(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
	, m_strName(L"")
{
	assert(m_sdaiInstance != 0);

	wchar_t* szName = 0;
	sdaiGetAttrBN(m_sdaiInstance, "name", sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";
}

/*virtual*/ _ap242_draughting_callout::~_ap242_draughting_callout()
{
}