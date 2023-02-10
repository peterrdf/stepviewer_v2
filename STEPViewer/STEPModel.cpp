#include "stdafx.h"

#include "STEPModel.h"
#include "ProductDefinition.h"
#include "Assembly.h"

#include <bitset>
#include <algorithm>

using namespace std;

#ifdef _LINUX
#include <cfloat>
#include <wx/wx.h>
#include <wx/stdpaths.h>
#include <cwchar>
#endif // _LINUX

// ------------------------------------------------------------------------------------------------
CSTEPModel::CSTEPModel()
	: CSTEPModelBase(enumSTEPModelType::STEP)
	, m_iModel(0)
	, m_mapProductDefinitions()
	, m_mapProductInstances()
	, m_mapAssemblies()
	, m_iID(1)
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
	, m_fBoundingSphereDiameter(1.f)
	, m_fXTranslation(0.f)
	, m_fYTranslation(0.f)
	, m_fZTranslation(0.f)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPModel::~CSTEPModel()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ int64_t CSTEPModel::GetInstance() const
{
	return m_iModel;
}

// --------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPModel::ZoomToInstance(CSTEPInstance* pSTEPInstance)
{	
	ASSERT(pSTEPInstance != nullptr);

	auto pInstance = dynamic_cast<CProductInstance*>(pSTEPInstance);
	ASSERT(pInstance != nullptr);
	ASSERT(pInstance->getProductDefinition() != nullptr);	

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	pInstance->getProductDefinition()->CalculateMinMaxTransform(
		pInstance,
		m_fXTranslation, m_fYTranslation, m_fZTranslation,
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

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPModel::ZoomOut() /*override*/
{
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	auto itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		auto itInstance = itProductDefinition->second->getProductInstances();
		for (auto pInstance : itProductDefinition->second->getProductInstances())
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			itProductDefinition->second->CalculateMinMaxTransform(
				pInstance,
				m_fXTranslation, m_fYTranslation, m_fZTranslation,
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
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const
{
	fXTranslation = m_fXTranslation;
	fYTranslation = m_fYTranslation;
	fZTranslation = m_fZTranslation;
}

// ------------------------------------------------------------------------------------------------
float CSTEPModel::GetBoundingSphereDiameter() const
{
	return m_fBoundingSphereDiameter;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CProductDefinition*>& CSTEPModel::getProductDefinitions()
{
	return m_mapProductDefinitions;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CProductInstance*>& CSTEPModel::getProductInstances()
{
	return m_mapProductInstances;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CAssembly*>& CSTEPModel::getAssemblies()
{
	return m_mapAssemblies;
}

// ------------------------------------------------------------------------------------------------
CProductInstance* CSTEPModel::getProductInstanceByID(int_t iID) const
{
	auto itProductInstance = m_mapProductInstances.find(iID);
	if (itProductInstance == m_mapProductInstances.end())
	{
		ASSERT(FALSE);

		return nullptr;
	}

	return itProductInstance->second;
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::ScaleAndCenter()
{
	/*
	* Min/Max
	*/

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	m_fBoundingSphereDiameter = 0.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;
	
	auto itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		if (!itProductDefinition->second->hasGeometry())
		{
			continue;
		}

		auto itInstance = itProductDefinition->second->getProductInstances();
		for (auto pInstance : itProductDefinition->second->getProductInstances())
		{
			itProductDefinition->second->CalculateMinMaxTransform(
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
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	/*
	* Scale
	*/

	itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		if (!itProductDefinition->second->hasGeometry())
		{
			continue;
		}

		itProductDefinition->second->Scale(m_fBoundingSphereDiameter);
	}

	/*
	* Min/max
	*/

	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		if (!itProductDefinition->second->hasGeometry())
		{
			continue;
		}

		auto itInstance = itProductDefinition->second->getProductInstances();
		for (auto pInstance : itProductDefinition->second->getProductInstances())
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			itProductDefinition->second->CalculateMinMaxTransform(
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
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Save(const wchar_t * /*szPath*/)
{
	ASSERT(0); // todo
	//m_pModel->saveModelW(szPath);
}

// ------------------------------------------------------------------------------------------------
// Loads a model
// ------------------------------------------------------------------------------------------------
void CSTEPModel::Load(const wchar_t * szPath)
{
	Clean();

	int64_t iModel = sdaiOpenModelBNUnicode(0, szPath, L"");
	if (iModel == 0)
	{
		MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

		return;
	}

	Load(szPath, iModel);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Load(const wchar_t* /*szPath*/, int64_t iModel)
{
	Clean();

	m_iModel = iModel;

	LoadProductDefinitions();

	LoadAssemblies();

	LoadGeometry();

	ScaleAndCenter();
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadProductDefinitions()
{
	int_t* pProductDefinitionInstances = sdaiGetEntityExtentBN(m_iModel, "PRODUCT_DEFINITION");

	int_t noProductDefinitionInstances = sdaiGetMemberCount(pProductDefinitionInstances);
	for (int_t i = 0; i < noProductDefinitionInstances; i++) 
	{
		int_t iProductDefinitionInstance = 0;
		engiGetAggrElement(pProductDefinitionInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);

		auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
		ASSERT(m_mapProductDefinitions.find(pDefinition->getExpressID()) == m_mapProductDefinitions.end());

		m_mapProductDefinitions[pDefinition->getExpressID()] = pDefinition;
	}	
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CSTEPModel::LoadProductDefinition(int_t iProductDefinitionInstance)
{
	auto pDefinition = new CProductDefinition();

	pDefinition->m_iExpressID = internalGetP21Line(iProductDefinitionInstance);
	pDefinition->m_iInstance = iProductDefinitionInstance;

	char* szId = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "id", sdaiSTRING, &szId);
	pDefinition->m_strId = szId != nullptr ? CA2W(szId) : L"";

	char* szName = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "name", sdaiSTRING, &szName);
	pDefinition->m_strName = szName != nullptr ? CA2W(szName) : L"";

	char* szDescription = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "description", sdaiSTRING, &szDescription);
	pDefinition->m_strDescription = szDescription != nullptr ? CA2W(szDescription) : L"";

	int_t iProductDefinitionFormationInstance = 0;
	sdaiGetAttrBN(iProductDefinitionInstance, "formation", sdaiINSTANCE, &iProductDefinitionFormationInstance);

	int_t iProductInstance = 0;
	sdaiGetAttrBN(iProductDefinitionFormationInstance, "of_product", sdaiINSTANCE, &iProductInstance);

	char* szProductId = nullptr;
	sdaiGetAttrBN(iProductInstance, "id", sdaiSTRING, &szProductId);
	pDefinition->m_strProductId = szProductId != nullptr ? CA2W(szProductId) : L"";

	char* szProductName = nullptr;
	sdaiGetAttrBN(iProductInstance, "name", sdaiSTRING, &szProductName);
	pDefinition->m_strProductName = szProductName != nullptr ? CA2W(szProductName) : L"";

	return pDefinition;
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CSTEPModel::GetProductDefinition(int_t iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	int_t expressID = internalGetP21Line(iProductDefinitionInstance);

	map<int_t, CProductDefinition*>::iterator itProductDefinition = m_mapProductDefinitions.find(expressID);
	if (itProductDefinition != m_mapProductDefinitions.end())
	{
		if (bRelatingProduct)
		{
			itProductDefinition->second->m_iRelatingProductRefs++;
		}

		if (bRelatedProduct)
		{
			itProductDefinition->second->m_iRelatedProductRefs++;
		}

		return itProductDefinition->second;
	}

	auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
	ASSERT(m_mapProductDefinitions.find(pDefinition->getExpressID()) == m_mapProductDefinitions.end());

	if (bRelatingProduct)
	{
		pDefinition->m_iRelatingProductRefs++;
	}

	if (bRelatedProduct)
	{
		pDefinition->m_iRelatedProductRefs++;
	}

	m_mapProductDefinitions[pDefinition->getExpressID()] = pDefinition;

	return pDefinition;
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadAssemblies()
{
	int_t* pNextAssemblyUsageOccurrenceInstances = sdaiGetEntityExtentBN(m_iModel, "NEXT_ASSEMBLY_USAGE_OCCURRENCE");

	int_t noNextAssemblyUsageOccurrenceInstances = sdaiGetMemberCount(pNextAssemblyUsageOccurrenceInstances);
	for (int_t i = 0; i < noNextAssemblyUsageOccurrenceInstances; i++) 
	{
		int_t pNextAssemblyUsageOccurrenceInstance = 0;
		engiGetAggrElement(pNextAssemblyUsageOccurrenceInstances, i, sdaiINSTANCE, &pNextAssemblyUsageOccurrenceInstance);

		CAssembly* pAssembly = new CAssembly();

		pAssembly->m_iExpressID = internalGetP21Line(pNextAssemblyUsageOccurrenceInstance);

		char* szId = nullptr;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "id", sdaiSTRING, &szId);
		pAssembly->m_strId = szId != nullptr ? CA2W(szId) : L"";

		char* szName = nullptr;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "name", sdaiSTRING, &szName);
		pAssembly->m_strName = szName != nullptr ? CA2W(szName) : L"";

		char* szDescription = nullptr;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "description", sdaiSTRING, &szDescription);
		pAssembly->m_strDescription = szDescription != nullptr ? CA2W(szDescription) : L"";

		int_t iRelatingProductDefinition = 0;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "relating_product_definition", sdaiINSTANCE, &iRelatingProductDefinition);

		pAssembly->m_pRelatingProductDefinition = GetProductDefinition(iRelatingProductDefinition, true, false);

		int_t iRelatedProductDefinition = 0;
		sdaiGetAttrBN(pNextAssemblyUsageOccurrenceInstance, "related_product_definition", sdaiINSTANCE, &iRelatedProductDefinition);

		pAssembly->m_pRelatedProductDefinition = GetProductDefinition(iRelatedProductDefinition, false, true);

		ASSERT(m_mapAssemblies.find(pAssembly->m_iExpressID) == m_mapAssemblies.end());

		m_mapAssemblies[pAssembly->m_iExpressID] = pAssembly;
	}	
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadGeometry()
{
	// Defaults
	SetDefaultColor(
			m_iModel,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255
		);

	setSegmentation(m_iModel, 20, 0.);

	// Load
	auto itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		if (itProductDefinition->second->getRelatedProductRefs() == 0)
		{
			WalkAssemblyTreeRecursively("", "", itProductDefinition->second, nullptr);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pDefinition, MATRIX* pParentMatrix)
{
	map<int_t, CAssembly*>::iterator itAssembly = m_mapAssemblies.begin();
	for (; itAssembly != m_mapAssemblies.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->m_pRelatingProductDefinition == pDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(m_iModel, internalGetInstanceFromP21Line(m_iModel, pAssembly->getExpressID()), &owlInstanceMatrix);

			if (owlInstanceMatrix && GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "Transformation")) 
			{
				owlInstanceMatrix = GetObjectProperty(owlInstanceMatrix, "matrix");
			}

			ASSERT(owlInstanceMatrix == 0 || GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "Matrix") || 
				GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "MatrixMultiplication"));

			MATRIX matrix;
			MatrixIdentity(&matrix);
			if (owlInstanceMatrix) 
			{
				InferenceInstance(owlInstanceMatrix);
				matrix._11 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_11");
				matrix._12 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_12");
				matrix._13 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_13");
				matrix._21 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_21");
				matrix._22 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_22");
				matrix._23 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_23");
				matrix._31 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_31");
				matrix._32 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_32");
				matrix._33 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_33");
				matrix._41 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_41");
				matrix._42 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_42");
				matrix._43 = GetDatatypeProperty__DOUBLE(owlInstanceMatrix, "_43");
			}

			if (pParentMatrix) 
			{
				MatrixMultiply(&matrix, &matrix, pParentMatrix);
			}

			WalkAssemblyTreeRecursively(szStepName, szGroupName, pAssembly->m_pRelatedProductDefinition, &matrix);
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...

	int_t myProductDefinitionInstanceHandle = internalGetInstanceFromP21Line(m_iModel, pDefinition->getExpressID());

	int64_t	owlInstanceProductDefinition = 0;
	owlBuildInstance(m_iModel, myProductDefinitionInstanceHandle, &owlInstanceProductDefinition);

	pDefinition->Calculate();

	cleanMemory(m_iModel, 0);

	auto pInstance = new CProductInstance(m_iID++, pDefinition, pParentMatrix);
	m_mapProductInstances[pInstance->getID()] = pInstance;

	pDefinition->m_vecProductInstances.push_back(pInstance);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel(m_iModel);
		m_iModel = 0;
	}

	auto itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		delete itProductDefinition->second;
	}
	m_mapProductDefinitions.clear();

	auto itProductInstance = m_mapProductInstances.begin();
	for (; itProductInstance != m_mapProductInstances.end(); itProductInstance++)
	{
		delete itProductInstance->second;
	}
	m_mapProductInstances.clear();

	auto itAssembly = m_mapAssemblies.begin();
	for (; itAssembly != m_mapAssemblies.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapAssemblies.clear();

	m_iID = 1;
}
