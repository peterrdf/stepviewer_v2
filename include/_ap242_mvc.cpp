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
	clean();
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

		auto pDraughtingModel = new _ap242_draughting_model(sdaiDraughtingModelInstance);
		m_vecDraughtingModels.push_back(pDraughtingModel);

		SdaiAttr sdaiItemsAttr = sdaiGetAttrDefinition(sdaiGetEntity(getSdaiModel(), "REPRESENTATION"), "items");
		assert(sdaiItemsAttr != nullptr);

		SdaiAggr sdaiItemsAggr = nullptr;
		sdaiGetAttr(sdaiDraughtingModelInstance, sdaiItemsAttr, sdaiAGGR, &sdaiItemsAggr);
		SdaiInteger iItemsCount = sdaiGetMemberCount(sdaiItemsAggr);
		for (SdaiInteger j = 0; j < iItemsCount; j++)
		{
			SdaiInstance sdaiItemInstance = 0;
			sdaiGetAggrByIndex(sdaiItemsAggr, j, sdaiINSTANCE, &sdaiItemInstance);

			if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(getSdaiModel(), "ANNOTATION_PLANE"))
			{
				auto pGeometry = getGeometryByInstance(sdaiItemInstance);
				if (pGeometry == nullptr)
				{
					pDraughtingModel->m_vecAnnotationPlanes.push_back(loadAnnotationPlane(sdaiItemInstance));
				}				
			}
			else if (sdaiGetInstanceType(sdaiItemInstance) == sdaiGetEntity(getSdaiModel(), "DRAUGHTING_CALLOUT"))
			{
				auto pGeometry = getGeometryByInstance(sdaiItemInstance);
				if (pGeometry == nullptr)
				{
					pDraughtingModel->m_vecDraughtingCallouts.push_back(loadDraughtingCallout(sdaiItemInstance));
				}				
			}
		}
	} // for (SdaiInteger i = ...
}

_ap242_annotation_plane* _ap242_model::loadAnnotationPlane(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(sdaiInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	auto pGeometry = new _ap242_annotation_plane(owlInstance, sdaiInstance);
	addGeometry(pGeometry);

	auto pInstance = new _ap_instance(
		_model::getNextInstanceID(),
		pGeometry,
		nullptr);
	addInstance(pInstance);

	return pGeometry;
}

_ap242_draughting_callout* _ap242_model::loadDraughtingCallout(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(sdaiInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	auto pGeometry = new _ap242_draughting_callout(owlInstance, sdaiInstance);
	addGeometry(pGeometry);

	auto pInstance = new _ap_instance(
		_model::getNextInstanceID(),
		pGeometry,
		nullptr);
	addInstance(pInstance);

	return pGeometry;
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
	assert(sdaiNameAttr != nullptr);

	wchar_t* szName = nullptr;
	sdaiGetAttr(m_sdaiInstance, sdaiNameAttr, sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";
}

/*virtual*/ _ap242_draughting_model::~_ap242_draughting_model()
{
}

void _ap242_draughting_model::enableInstances(bool bEnable)
{
	for (auto pAnnotationPlane : m_vecAnnotationPlanes)
	{
		assert(pAnnotationPlane->getInstances().size() == 1);
		pAnnotationPlane->getInstances()[0]->setEnable(bEnable);
	}

	for (auto pDraughtingCallout : m_vecDraughtingCallouts)
	{
		assert(pDraughtingCallout->getInstances().size() == 1);
		pDraughtingCallout->getInstances()[0]->setEnable(bEnable);
	}
}

// ************************************************************************************************
_ap242_annotation_plane::_ap242_annotation_plane(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
	, m_strName(L"")
{
	wchar_t* szName = 0;
	sdaiGetAttrBN(getSdaiInstance(), "name", sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";

	calculate();
}

/*virtual*/ _ap242_annotation_plane::~_ap242_annotation_plane()
{
}

// ************************************************************************************************
_ap242_draughting_callout::_ap242_draughting_callout(OwlInstance owlInstance, SdaiInstance sdaiInstance)
	: _ap_geometry(owlInstance, sdaiInstance)
	, m_strName(L"")
{
	wchar_t* szName = 0;
	sdaiGetAttrBN(getSdaiInstance(), "name", sdaiUNICODE, &szName);
	m_strName = szName != nullptr ? szName : L"NA";

	calculate();
}

/*virtual*/ _ap242_draughting_callout::~_ap242_draughting_callout()
{
}