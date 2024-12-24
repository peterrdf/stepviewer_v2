#include "stdafx.h"
#include "_ap242_mvc.h"

// ************************************************************************************************
_ap242_model::_ap242_model()
	: _ap_model(enumAP::STEP)
	, m_vecDraghtingModels()
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

	for (auto pDraghtingModel : m_vecDraghtingModels)
	{
		delete pDraghtingModel;
	}
	m_vecDraghtingModels.clear();
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

		m_vecDraghtingModels.push_back(new _ap242_draghting_model(sdaiDraughtingModelInstance));
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
_ap242_draghting_model::_ap242_draghting_model(SdaiInstance sdaiInstance)
	: m_sdaiInstance(sdaiInstance)
	, m_strName(L"")
{
	assert(m_sdaiInstance != 0);

	SdaiAttr sdaiNameAttr = sdaiGetAttrDefinition(sdaiGetEntity(sdaiGetInstanceModel(m_sdaiInstance), "REPRESENTATION"), "name");

	wchar_t* szName = nullptr;
	sdaiGetAttr(m_sdaiInstance, sdaiNameAttr, sdaiUNICODE, &szName);

	m_strName = szName != nullptr ? szName : L"NA";
}

/*virtual*/ _ap242_draghting_model::~_ap242_draghting_model()
{
}

void _ap242_draghting_model::load()
{
	/*SdaiAttr sdaiItemsAttr = sdaiGetAttrDefinition(sdaiGetEntity(getSdaiModel(), "REPRESENTATION"), "items");

	SdaiAggr sdaiItemsAggr = nullptr;
	sdaiGetAttr(sdaiDraughtingModelInstance, sdaiItemsAttr, sdaiAGGR, &sdaiItemsAggr);
	SdaiInteger iItemsCount = sdaiGetMemberCount(sdaiItemsAggr);
	for (SdaiInteger j = 0; j < iItemsCount; j++)
	{
		SdaiInstance sdaiItemInstance = 0;
		sdaiGetAggrByIndex(sdaiItemsAggr, j, sdaiINSTANCE, &sdaiItemInstance);

		if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(getSdaiModel(), "ANNOTATION_PLANE") ||
			sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(getSdaiModel(), "DRAUGHTING_CALLOUT")) {
			wchar_t* szName2 = 0;
			sdaiGetAttrBN(sdaiItemInstance, "name", sdaiUNICODE, &szName2);
			TRACE(L"");
		}
	}*/
}