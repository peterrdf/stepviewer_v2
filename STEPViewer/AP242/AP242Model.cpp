#include "stdafx.h"

#include "_ptr.h"

#include "AP242Model.h"
#include "AP242ProductDefinition.h"
#include "AP242Assembly.h"

#include <bitset>
#include <algorithm>
using namespace std;

// ************************************************************************************************
static double GetDoubleProperty(OwlInstance iInstance, char* szPropertyName)
{
	double* pdValues = nullptr;
	int64_t	iCard = 0;
	GetDatatypeProperty(
		iInstance,
		GetPropertyByName(GetModel(iInstance), szPropertyName),
		(void**)&pdValues,
		&iCard);

	return (iCard == 1) ? pdValues[0] : 0.;
}

static int64_t GetObjectProperty(OwlInstance iInstance, char* szPropertyName)
{
	int64_t* piValues = nullptr;
	int64_t	iCard = 0;
	GetObjectProperty(
		iInstance,
		GetPropertyByName(GetModel(iInstance), szPropertyName),
		&piValues,
		&iCard);

	return (iCard == 1) ? piValues[0] : 0;
}

// ************************************************************************************************
static uint32_t DEFAULT_COLOR_R = 175;
static uint32_t DEFAULT_COLOR_G = 175;
static uint32_t DEFAULT_COLOR_B = 175;
static uint32_t DEFAULT_COLOR_A = 255;
/*static*/ uint32_t CAP242Model::DEFAULT_COLOR =
	256 * 256 * 256 * DEFAULT_COLOR_R +
	256 * 256 * DEFAULT_COLOR_G +
	256 * DEFAULT_COLOR_B +
	DEFAULT_COLOR_A;

// ************************************************************************************************
CAP242Model::CAP242Model()
	: _ap_model(enumAP::STEP)
	, m_mapID2Instance()
	, m_mapExpressIDAssembly()
	, m_iID(1)
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

	Scale();
}

/*virtual*/ void CAP242Model::clean() /*override*/
{
	_ap_model::clean();

	m_mapID2Instance.clear();

	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapExpressIDAssembly.clear();

	m_iID = 1;
}

/*virtual*/ void CAP242Model::ZoomToInstance(_instance* pInstance) /*override*/
{	
	ASSERT(pInstance != nullptr);

	auto pProductInstance = dynamic_cast<CAP242ProductInstance*>(pInstance);
	if ((pProductInstance == nullptr) || (pProductInstance->GetProductDefinition() == nullptr))
	{
		ASSERT(FALSE);

		return;
	}

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	pProductInstance->GetProductDefinition()->calculateMinMaxTransform(
		pProductInstance,
		m_fXmin, m_fXmax,
		m_fYmin, m_fYmax,
		m_fZmin, m_fZmax);

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

/*virtual*/ void CAP242Model::ZoomOut() /*override*/
{
	/*m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		auto& vecInstances = itDefinition->second->getInstances();
		for (auto pInstance : vecInstances)
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			itDefinition->second->calculateMinMaxTransform(
				pInstance,
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
		m_fXmin = -1.;
		m_fXmax = 1.;
		m_fYmin = -1.;
		m_fYmax = 1.;
		m_fZmin = -1.;
		m_fZmax = 1.;
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);*/
}

CAP242ProductInstance* CAP242Model::getProductInstanceByID(int64_t iID) const
{
	auto itInstance = m_mapID2Instance.find(iID);
	if (itInstance == m_mapID2Instance.end())
	{
		ASSERT(FALSE);

		return nullptr;
	}

	return itInstance->second;
}

void CAP242Model::Scale()
{
	scale();
}

void CAP242Model::Save(const wchar_t * /*szPath*/)
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
	preLoadInstance(iProductDefinitionInstance);

	auto pGeometry = new CAP242ProductDefinition(iProductDefinitionInstance);
	m_vecGeometries.push_back(pGeometry);

	ASSERT(m_mapExpressID2Geometry.find(pGeometry->getExpressID()) == m_mapExpressID2Geometry.end());
	m_mapExpressID2Geometry[pGeometry->getExpressID()] = pGeometry;

	ASSERT(m_mapExpressID2Geometry.find(pGeometry->getExpressID()) == m_mapExpressID2Geometry.end());
	m_mapExpressID2Geometry[pGeometry->getExpressID()] = pGeometry;

	return pGeometry;
}

CAP242ProductDefinition* CAP242Model::GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	ExpressID iExpressID = internalGetP21Line(iProductDefinitionInstance);

	auto itExpressID2Geometry = m_mapExpressID2Geometry.find(iExpressID);
	if (itExpressID2Geometry != m_mapExpressID2Geometry.end())
	{
		_ptr<CAP242ProductDefinition> pProductDefinition(itExpressID2Geometry->second);
		if (bRelatingProduct)
		{
			pProductDefinition->m_iRelatingProducts++;
		}

		if (bRelatedProduct)
		{
			pProductDefinition->m_iRelatedProducts++;
		}

		return pProductDefinition.p();
	}

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
	auto itDefinition = m_mapExpressID2Geometry.begin();
	for (auto pGeometry : m_vecGeometries)
	{
		_ptr<CAP242ProductDefinition> pProductDefinition(pGeometry);

		if (pProductDefinition->GetRelatedProducts() == 0)
		{
			WalkAssemblyTreeRecursively(pProductDefinition, nullptr, nullptr);
		}
	}
}

void CAP242Model::WalkAssemblyTreeRecursively(CAP242ProductDefinition* pProductDefinition, CAP242Assembly* pParentAssembly, _matrix4x3* pParentMatrix)
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
				owlInstanceMatrix = GetObjectProperty(owlInstanceMatrix, "matrix");
			}

			ASSERT(owlInstanceMatrix == 0 || GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "Matrix") || 
				GetInstanceClass(owlInstanceMatrix) == GetClassByName(::GetModel(owlInstanceMatrix), "MatrixMultiplication"));

			_matrix4x3 matrix;
			_matrix4x3Identity(&matrix);

			if (owlInstanceMatrix) 
			{
				InferenceInstance(owlInstanceMatrix);
				matrix._11 = GetDoubleProperty(owlInstanceMatrix, "_11");
				matrix._12 = GetDoubleProperty(owlInstanceMatrix, "_12");
				matrix._13 = GetDoubleProperty(owlInstanceMatrix, "_13");
				matrix._21 = GetDoubleProperty(owlInstanceMatrix, "_21");
				matrix._22 = GetDoubleProperty(owlInstanceMatrix, "_22");
				matrix._23 = GetDoubleProperty(owlInstanceMatrix, "_23");
				matrix._31 = GetDoubleProperty(owlInstanceMatrix, "_31");
				matrix._32 = GetDoubleProperty(owlInstanceMatrix, "_32");
				matrix._33 = GetDoubleProperty(owlInstanceMatrix, "_33");
				matrix._41 = GetDoubleProperty(owlInstanceMatrix, "_41");
				matrix._42 = GetDoubleProperty(owlInstanceMatrix, "_42");
				matrix._43 = GetDoubleProperty(owlInstanceMatrix, "_43");
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
		pParentMatrix);
	m_vecInstances.push_back(pInstance);
	m_mapID2Instance[pInstance->getID()] = pInstance;
	pProductDefinition->addInstance(pInstance);
}