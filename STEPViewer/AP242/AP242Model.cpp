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
	: _ap_model(enumAP::STEP)
	, m_iID(1)
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

	scale();
}

/*virtual*/ void CAP242Model::clean() /*override*/
{
	_ap_model::clean();

	m_iID = 1;

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
	SdaiAggr pProductDefinitionInstances = sdaiGetEntityExtentBN(getSdaiInstance(), "PRODUCT_DEFINITION");

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
	SdaiAggr pNextAssemblyUsageOccurrenceInstances = sdaiGetEntityExtentBN(getSdaiInstance(), "NEXT_ASSEMBLY_USAGE_OCCURRENCE");

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

void CAP242Model::WalkAssemblyTreeRecursively(CAP242ProductDefinition* pProductDefinition, CAP242Assembly* pAssembly, _matrix4x3* pParentMatrix)
{
	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->GetRelatingProductDefinition() == pProductDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(getSdaiInstance(), internalGetInstanceFromP21Line(getSdaiInstance(), pAssembly->GetExpressID()), &owlInstanceMatrix);

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
		m_iID++,
		pProductDefinition,
		pAssembly, 
		pParentMatrix);
	addInstance(pInstance);
}