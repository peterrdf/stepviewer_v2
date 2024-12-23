#include "stdafx.h"

#include "_ptr.h"
#include "_owl_instance.h"

#include "AP242Model.h"
#include "AP242ProductDefinition.h"
#include "AP242Assembly.h"

#include <bitset>
#include <algorithm>
using namespace std;

// ************************************************************************************************
CAP242Model::CAP242Model()
	: _step_model()
	, m_mapExpressIDAssembly()
{
}

CAP242Model::~CAP242Model()
{
	clean();
}

/*virtual*/ void CAP242Model::attachModelCore() /*override*/
{
	LoadProductDefinitions();
	LoadAssemblies();
	LoadGeometry();

	LoadDraughtingModels();

	scale();
}

/*virtual*/ void CAP242Model::clean() /*override*/
{
	_ap_model::clean();

	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapExpressIDAssembly.clear();	
}

void CAP242Model::Save(const wchar_t* /*szPath*/)
{
	ASSERT(0); // todo
	//m_pModel->saveModelW(szPath);
}

void CAP242Model::LoadProductDefinitions()
{
	SdaiAggr pProductDefinitionInstances = sdaiGetEntityExtentBN(getSdaiModel(), "PRODUCT_DEFINITION");

	int_t noProductDefinitionInstances = sdaiGetMemberCount(pProductDefinitionInstances);
	for (int_t i = 0; i < noProductDefinitionInstances; i++) 
	{
		SdaiInstance iProductDefinitionInstance = 0;
		sdaiGetAggrByIndex(pProductDefinitionInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);
		ASSERT(iProductDefinitionInstance != 0);

		LoadProductDefinition(iProductDefinitionInstance);
	}	
}

CAP242ProductDefinition* CAP242Model::LoadProductDefinition(SdaiInstance iProductDefinitionInstance)
{
	OwlInstance owlInstance = _ap_geometry::buildOwlInstance(iProductDefinitionInstance);
	if (owlInstance != 0)
	{
		preLoadInstance(owlInstance);
	}

	auto pGeometry = new CAP242ProductDefinition(owlInstance, iProductDefinitionInstance);
	addGeometry(pGeometry);	

	return pGeometry;
}

CAP242ProductDefinition* CAP242Model::GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	ExpressID iExpressID = internalGetP21Line(iProductDefinitionInstance);

	auto pGeometry = getGeometryByExpressID(iExpressID);
	if (pGeometry != nullptr)
	{
		_ptr<CAP242ProductDefinition> apProductDefinition(pGeometry);
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

void CAP242Model::LoadAssemblies()
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

		auto pAssembly = new CAP242Assembly(pNextAssemblyUsageOccurrenceInstance, pRelatingProductDefinition, pRelatedProductDefinition);
		ASSERT(m_mapExpressIDAssembly.find(pAssembly->GetExpressID()) == m_mapExpressIDAssembly.end());

		m_mapExpressIDAssembly[pAssembly->GetExpressID()] = pAssembly;
	}	
}

void CAP242Model::LoadGeometry()
{
	// Load
	for (auto pGeometry : getGeometries())
	{
		_ptr<CAP242ProductDefinition> apProductDefinition(pGeometry);
		if (apProductDefinition->GetRelatedProducts() == 0)
		{
			WalkAssemblyTreeRecursively(apProductDefinition, nullptr, nullptr);
		}
	}
}

void CAP242Model::WalkAssemblyTreeRecursively(CAP242ProductDefinition* pProductDefinition, CAP242Assembly* /*pParentAssembly*/, _matrix4x3* pParentMatrix)
{
	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->GetRelatingProductDefinition() == pProductDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(getSdaiModel(), internalGetInstanceFromP21Line(getSdaiModel(), pAssembly->GetExpressID()), &owlInstanceMatrix);

			if (owlInstanceMatrix && GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "Transformation")) 
			{
				owlInstanceMatrix = _owl_instance::getObjectProperty(owlInstanceMatrix, "matrix");
			}

			ASSERT(owlInstanceMatrix == 0 || GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "Matrix") || 
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

			WalkAssemblyTreeRecursively(pAssembly->GetRelatedProductDefinition(), pAssembly, &matrix);
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...

	auto pInstance = new CAP242ProductInstance(
		_model::getNextInstanceID(),
		pProductDefinition,
		pParentMatrix);
	addInstance(pInstance);
}

void CAP242Model::LoadDraughtingModels()
{
	SdaiAggr sdaiDraughtingModelAggr = xxxxGetEntityAndSubTypesExtentBN(getSdaiModel(), "DRAUGHTING_MODEL");
	SdaiInteger iDraughtingModelMembersCount = sdaiGetMemberCount(sdaiDraughtingModelAggr);
	for (SdaiInteger i = 0; i < iDraughtingModelMembersCount; i++)
	{
		SdaiInstance sdaiProductDefinitionInstance = 0;
		sdaiGetAggrByIndex(sdaiDraughtingModelAggr, i, sdaiINSTANCE, &sdaiProductDefinitionInstance);

		ASSERT(sdaiProductDefinitionInstance != 0);
		//LoadProductDefinition(sdaiProductDefinitionInstance);

		{
			wchar_t* szName = 0;
			SdaiAttr	itemsAttr = sdaiGetAttrDefinition(sdaiGetEntity(sdaiGetInstanceModel(sdaiProductDefinitionInstance), "REPRESENTATION"), "name");
			sdaiGetAttr(sdaiProductDefinitionInstance, itemsAttr, sdaiUNICODE, &szName);

			wchar_t* szName2 = 0;
			sdaiGetAttrBN(sdaiProductDefinitionInstance, "name", sdaiUNICODE, &szName2);
			TRACE(L"");
		}


		SdaiAttr sdaiItemsAttr = sdaiGetAttrDefinition(sdaiGetEntity(getSdaiModel(), "REPRESENTATION"), "items");

		SdaiAggr sdaiItemsAggr = nullptr;
		sdaiGetAttr(sdaiProductDefinitionInstance, sdaiItemsAttr, sdaiAGGR, &sdaiItemsAggr);
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
				//auto pDefinition = LoadProductDefinition(sdaiItemInstance);
				//assert(pDefinition);
				//ASSERT(m_mapExpressID2G.find(pDefinition->getExpressID()) == m_mapExpressID2Definition.end());

				//m_mapExpressID2Definition[pDefinition->getExpressID()] = pDefinition;
			}
		}
	} // for (SdaiInteger i = ...
}