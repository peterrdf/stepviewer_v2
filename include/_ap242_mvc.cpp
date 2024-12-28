#include "stdafx.h"

#include "_ap242_mvc.h"
#include "_ap242_product_definition.h"
#include "_ap242_draughting_model.h"
#include "_ap242_instance.h"
#include "_owl_instance.h"

// ************************************************************************************************
_ap242_model::_ap242_model()
	: _ap_model(enumAP::STEP)
	, m_mapExpressIDAssembly()
	, m_vecDraughtingModels()
{
}

/*virtual*/ _ap242_model::~_ap242_model()
{
	clean();
}

/*virtual*/ void _ap242_model::attachModelCore() /*override*/
{
	LoadProductDefinitions();
	LoadAssemblies();
	LoadGeometry();

	loadDraughtingModels();

	scale();
}

/*virtual*/ void _ap242_model::preLoadInstance(OwlInstance owlInstance) /*override*/
{
	//#todo
	//setVertexBufferOffset(owlInstance);
}

/*virtual*/ void _ap242_model::clean() /*override*/
{
	_ap_model::clean();

	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapExpressIDAssembly.clear();

	for (auto pDraughtingModel : m_vecDraughtingModels)
	{
		delete pDraughtingModel;
	}
	m_vecDraughtingModels.clear();
}

void _ap242_model::LoadProductDefinitions()
{
	SdaiAggr pProductDefinitionInstances = sdaiGetEntityExtentBN(getSdaiModel(), "PRODUCT_DEFINITION");

	int_t noProductDefinitionInstances = sdaiGetMemberCount(pProductDefinitionInstances);
	for (int_t i = 0; i < noProductDefinitionInstances; i++)
	{
		SdaiInstance iProductDefinitionInstance = 0;
		sdaiGetAggrByIndex(pProductDefinitionInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);
		assert(iProductDefinitionInstance != 0);

		LoadProductDefinition(iProductDefinitionInstance);
	}
}

_ap242_product_definition* _ap242_model::LoadProductDefinition(SdaiInstance iProductDefinitionInstance)
{
	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(iProductDefinitionInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	auto pGeometry = new _ap242_product_definition(owlInstance, iProductDefinitionInstance);
	addGeometry(pGeometry);

	return pGeometry;
}

_ap242_product_definition* _ap242_model::GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	ExpressID iExpressID = internalGetP21Line(iProductDefinitionInstance);

	auto pGeometry = getGeometryByExpressID(iExpressID);
	if (pGeometry != nullptr)
	{
		_ptr<_ap242_product_definition> apProductDefinition(pGeometry);
		if (bRelatingProduct)
		{
			apProductDefinition->m_iRelatingProducts++;
		}

		if (bRelatedProduct)
		{
			apProductDefinition->m_iRelatedProducts++;
		}

		return apProductDefinition;
	} // if (pGeometry != nullptr)

	auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
	if (bRelatingProduct)
	{
		pDefinition->m_iRelatingProducts++;
	}

	if (bRelatedProduct)
	{
		pDefinition->m_iRelatedProducts++;
	}

	return pDefinition;
}

void _ap242_model::LoadAssemblies()
{
	SdaiAggr pNextAssemblyUsageOccurrenceInstances = sdaiGetEntityExtentBN(getSdaiModel(), "NEXT_ASSEMBLY_USAGE_OCCURRENCE");

	int_t noNextAssemblyUsageOccurrenceInstances = sdaiGetMemberCount(pNextAssemblyUsageOccurrenceInstances);
	for (int_t i = 0; i < noNextAssemblyUsageOccurrenceInstances; i++)
	{
		SdaiInstance pNextAssemblyUsageOccurrenceInstance = 0;
		sdaiGetAggrByIndex(pNextAssemblyUsageOccurrenceInstances, i, sdaiINSTANCE, &pNextAssemblyUsageOccurrenceInstance);

		int_t iRelatingProductDefinition = 0;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "relating_product_definition", sdaiINSTANCE, &iRelatingProductDefinition);

		auto pRelatingProductDefinition = GetProductDefinition(iRelatingProductDefinition, true, false);

		int_t iRelatedProductDefinition = 0;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "related_product_definition", sdaiINSTANCE, &iRelatedProductDefinition);

		auto pRelatedProductDefinition = GetProductDefinition(iRelatedProductDefinition, false, true);

		auto pAssembly = new _ap242_assembly(pNextAssemblyUsageOccurrenceInstance, pRelatingProductDefinition, pRelatedProductDefinition);
		assert(m_mapExpressIDAssembly.find(pAssembly->getExpressID()) == m_mapExpressIDAssembly.end());

		m_mapExpressIDAssembly[pAssembly->getExpressID()] = pAssembly;
	}
}

void _ap242_model::LoadGeometry()
{
	// Load
	for (auto pGeometry : getGeometries())
	{
		_ptr<_ap242_product_definition> apProductDefinition(pGeometry);
		if (apProductDefinition->getRelatedProducts() == 0)
		{
			WalkAssemblyTreeRecursively(apProductDefinition, nullptr, nullptr);
		}
	}
}

void _ap242_model::WalkAssemblyTreeRecursively(_ap242_product_definition* pProductDefinition, _ap242_assembly* /*pParentAssembly*/, _matrix4x3* pParentMatrix)
{
	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->getRelatingProductDefinition() == pProductDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(getSdaiModel(), internalGetInstanceFromP21Line(getSdaiModel(), pAssembly->getExpressID()), &owlInstanceMatrix);

			if (owlInstanceMatrix && GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "Transformation"))
			{
				owlInstanceMatrix = _owl_instance::getObjectProperty(owlInstanceMatrix, "matrix");
			}

			assert(owlInstanceMatrix == 0 || GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "Matrix") ||
				GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "MatrixMultiplication"));

			_matrix4x3 matrix;
			_matrix4x3Identity(&matrix);

			if (owlInstanceMatrix)
			{
				InferenceInstance(owlInstanceMatrix);
				matrix._11 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_11");
				matrix._12 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_12");
				matrix._13 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_13");
				matrix._21 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_21");
				matrix._22 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_22");
				matrix._23 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_23");
				matrix._31 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_31");
				matrix._32 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_32");
				matrix._33 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_33");
				matrix._41 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_41");
				matrix._42 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_42");
				matrix._43 = _owl_instance::getDoubleProperty(owlInstanceMatrix, "_43");
			}

			if (pParentMatrix)
			{
				_matrix4x3Multiply(&matrix, &matrix, pParentMatrix);
			}

			WalkAssemblyTreeRecursively(pAssembly->getRelatedProductDefinition(), pAssembly, &matrix);
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...

	auto pInstance = new _ap242_instance(
		_model::getNextInstanceID(),
		pProductDefinition,
		pParentMatrix);
	addInstance(pInstance);
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

void _ap242_model::Save(const wchar_t* /*szPath*/)
{
	assert(0); //#todo
}