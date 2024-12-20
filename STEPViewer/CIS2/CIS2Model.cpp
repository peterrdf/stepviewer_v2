#include "stdafx.h"
#include "CIS2Model.h"
#include "CIS2Instance.h"

// ************************************************************************************************
//#todo???
#define DEFAULT_CIRCLE_SEGMENTS 36

// ************************************************************************************************
CCIS2Model::CCIS2Model(bool bLoadInstancesOnDemand/* = false*/)
	: _ap_model(enumAP::CIS2)
	, m_bLoadInstancesOnDemand(bLoadInstancesOnDemand)
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
		int_t* cis2AnalysisModel3DInstances = sdaiGetEntityExtentBN(getSdaiModel(), "ANALYSIS_MODEL_3D"),
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

	scale();
}

void CCIS2Model::LodDesignParts()
{
	int_t* piInstances = sdaiGetEntityExtentBN(getSdaiModel(), "DESIGN_PART");
	int_t iInstancesCount = sdaiGetMemberCount(piInstances);
	for (int_t i = 0; i < iInstancesCount; i++)
	{
		SdaiInstance iInstance = 0;
		sdaiGetAggrByIndex(piInstances, i, sdaiINSTANCE, &iInstance);
		ASSERT(iInstance != 0);

		LoadGeometry(iInstance, enumCIS2GeometryType::DesignPart, DEFAULT_CIRCLE_SEGMENTS);
	}
}

void CCIS2Model::LoadRepresentations()
{
	int_t* piInstances = sdaiGetEntityExtentBN(getSdaiModel(), "REPRESENTATION");
	int_t iInstancesCount = sdaiGetMemberCount(piInstances);
	for (int_t i = 0; i < iInstancesCount; i++)
	{
		SdaiInstance iInstance = 0;
		sdaiGetAggrByIndex(piInstances, i, sdaiINSTANCE, &iInstance);
		ASSERT(iInstance != 0);

		LoadGeometry(iInstance, enumCIS2GeometryType::Reperesentation, DEFAULT_CIRCLE_SEGMENTS);
	}
}

_geometry* CCIS2Model::LoadGeometry(SdaiInstance sdaiInstance, enumCIS2GeometryType enCIS2GeometryType, int_t iCircleSegments)
{
	_ap_geometry* pGeometry = dynamic_cast<_ap_geometry*>(getGeometryByInstance(sdaiInstance));
	if (pGeometry != nullptr)
	{
		return pGeometry;
	}

	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(sdaiInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	// Set up circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(iCircleSegments, 5);
	}

	switch (enCIS2GeometryType)
	{
		case enumCIS2GeometryType::DesignPart:
		{
			pGeometry = new CCIS2DesignPart(owlInstance, sdaiInstance);			
		}
		break;

		case enumCIS2GeometryType::Reperesentation:
		{
			pGeometry = new CCIS2Representation(owlInstance, sdaiInstance);
		}
		break;

		default:
		{
			ASSERT(FALSE);
		}
		break;
	}

	addGeometry(pGeometry);

	auto pInstance = new CCIS2Instance(_model::getNextInstanceID(), pGeometry, nullptr);
	addInstance(pInstance);

	// Restore circleSegments()
	if (iCircleSegments != DEFAULT_CIRCLE_SEGMENTS)
	{
		circleSegments(DEFAULT_CIRCLE_SEGMENTS, 5);
	}

	return pGeometry;
}