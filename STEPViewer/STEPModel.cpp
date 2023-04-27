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
	: CModel(enumModelType::STEP)
	, m_pEntityProvider(nullptr)
	, m_mapExpressID2Definition()
	, m_mapID2Instance()
	, m_mapExpressIDAssembly()
	, m_iID(1)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPModel::~CSTEPModel()
{
	Clean();
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CEntityProvider* CSTEPModel::GetEntityProvider() const /*override*/
{
	return m_pEntityProvider;
}

// --------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPModel::ZoomToInstance(CInstance* pInstance) /*override*/
{	
	ASSERT(pInstance != nullptr);

	auto pProductInstance = dynamic_cast<CProductInstance*>(pInstance);
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

	pProductInstance->GetProductDefinition()->CalculateMinMaxTransform(
		pProductInstance,
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

	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		auto& vecInstances = itDefinition->second->GetInstances();
		for (auto pInstance : vecInstances)
		{
			if (!pInstance->GetEnable())
			{
				continue;
			}

			itDefinition->second->CalculateMinMaxTransform(
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
const map<int_t, CProductDefinition*>& CSTEPModel::GetDefinitions()
{
	return m_mapExpressID2Definition;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CProductInstance*>& CSTEPModel::GetInstances()
{
	return m_mapID2Instance;
}

// ------------------------------------------------------------------------------------------------
const map<int_t, CAssembly*>& CSTEPModel::GetAssemblies()
{
	return m_mapExpressIDAssembly;
}

// ------------------------------------------------------------------------------------------------
CProductInstance* CSTEPModel::getProductInstanceByID(int_t iID) const
{
	auto itInstance = m_mapID2Instance.find(iID);
	if (itInstance == m_mapID2Instance.end())
	{
		ASSERT(FALSE);

		return nullptr;
	}

	return itInstance->second;
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
	
	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->HasGeometry())
		{
			continue;
		}

		auto itInstance = itDefinition->second->GetInstances();
		for (auto pInstance : itDefinition->second->GetInstances())
		{
			itDefinition->second->CalculateMinMaxTransform(
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

	/*
	* World
	*/
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale and Center I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere I *** =>  %.16f",
		m_fBoundingSphereDiameter);

	/*
	* Scale
	*/
	itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->HasGeometry())
		{
			continue;
		}

		itDefinition->second->Scale(m_fBoundingSphereDiameter);
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

	itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->HasGeometry())
		{
			continue;
		}

		auto itInstance = itDefinition->second->GetInstances();
		for (auto pInstance : itDefinition->second->GetInstances())
		{
			if (!pInstance->GetEnable())
			{
				continue;
			}

			itDefinition->second->CalculateMinMaxTransform(
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

	/*
	* World
	*/
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale and Center II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale and Center, Bounding sphere II *** =>  %.16f",
		m_fBoundingSphereDiameter);
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
void CSTEPModel::Load(const wchar_t* szPath, int64_t iModel)
{
	Clean();

	m_iModel = iModel;
	m_strFilePath = szPath;
	
	m_pEntityProvider = new CEntityProvider(m_iModel);

	LoadProductDefinitions();

	LoadAssemblies();

	LoadGeometry();

	ScaleAndCenter();
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadProductDefinitions()
{
	SdaiAggr pProductDefinitionInstances = sdaiGetEntityExtentBN(m_iModel, "PRODUCT_DEFINITION");

	int_t noProductDefinitionInstances = sdaiGetMemberCount(pProductDefinitionInstances);
	for (int_t i = 0; i < noProductDefinitionInstances; i++) 
	{
		SdaiInstance iProductDefinitionInstance = 0;
		sdaiGetAggrByIndex(pProductDefinitionInstances, i, sdaiINSTANCE, &iProductDefinitionInstance);

		ASSERT(iProductDefinitionInstance != 0);

		auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
		ASSERT(m_mapExpressID2Definition.find(pDefinition->GetExpressID()) == m_mapExpressID2Definition.end());

		m_mapExpressID2Definition[pDefinition->GetExpressID()] = pDefinition;
	}	
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CSTEPModel::LoadProductDefinition(SdaiInstance iProductDefinitionInstance)
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
CProductDefinition* CSTEPModel::GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	int_t iExpressID = internalGetP21Line(iProductDefinitionInstance);

	auto itDefinition = m_mapExpressID2Definition.find(iExpressID);
	if (itDefinition != m_mapExpressID2Definition.end())
	{
		if (bRelatingProduct)
		{
			itDefinition->second->m_iRelatingProductRefs++;
		}

		if (bRelatedProduct)
		{
			itDefinition->second->m_iRelatedProductRefs++;
		}

		return itDefinition->second;
	}

	auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
	ASSERT(m_mapExpressID2Definition.find(pDefinition->GetExpressID()) == m_mapExpressID2Definition.end());

	if (bRelatingProduct)
	{
		pDefinition->m_iRelatingProductRefs++;
	}

	if (bRelatedProduct)
	{
		pDefinition->m_iRelatedProductRefs++;
	}

	m_mapExpressID2Definition[pDefinition->GetExpressID()] = pDefinition;

	return pDefinition;
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadAssemblies()
{
	SdaiAggr pNextAssemblyUsageOccurrenceInstances = sdaiGetEntityExtentBN(m_iModel, "NEXT_ASSEMBLY_USAGE_OCCURRENCE");

	int_t noNextAssemblyUsageOccurrenceInstances = sdaiGetMemberCount(pNextAssemblyUsageOccurrenceInstances);
	for (int_t i = 0; i < noNextAssemblyUsageOccurrenceInstances; i++) 
	{
		SdaiInstance pNextAssemblyUsageOccurrenceInstance = 0;
		sdaiGetAggrByIndex(pNextAssemblyUsageOccurrenceInstances, i, sdaiINSTANCE, &pNextAssemblyUsageOccurrenceInstance);

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

		ASSERT(m_mapExpressIDAssembly.find(pAssembly->m_iExpressID) == m_mapExpressIDAssembly.end());

		m_mapExpressIDAssembly[pAssembly->m_iExpressID] = pAssembly;
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
	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (itDefinition->second->GetRelatedProductRefs() == 0)
		{
			WalkAssemblyTreeRecursively("", "", itDefinition->second, nullptr);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pDefinition, MATRIX* pParentMatrix)
{
	map<int_t, CAssembly*>::iterator itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->m_pRelatingProductDefinition == pDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(m_iModel, internalGetInstanceFromP21Line(m_iModel, pAssembly->GetExpressID()), &owlInstanceMatrix);

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

	int_t myProductDefinitionInstanceHandle = internalGetInstanceFromP21Line(m_iModel, pDefinition->GetExpressID());

	int64_t	owlInstanceProductDefinition = 0;
	owlBuildInstance(m_iModel, myProductDefinitionInstanceHandle, &owlInstanceProductDefinition);

	pDefinition->Calculate();

	cleanMemory(m_iModel, 0);

	auto pInstance = new CProductInstance(m_iID++, pDefinition, pParentMatrix);
	m_mapID2Instance[pInstance->GetID()] = pInstance;

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

	delete m_pEntityProvider;
	m_pEntityProvider = nullptr;

	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		delete itDefinition->second;
	}
	m_mapExpressID2Definition.clear();

	auto itInstance = m_mapID2Instance.begin();
	for (; itInstance != m_mapID2Instance.end(); itInstance++)
	{
		delete itInstance->second;
	}
	m_mapID2Instance.clear();

	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapExpressIDAssembly.clear();

	m_iID = 1;
}
