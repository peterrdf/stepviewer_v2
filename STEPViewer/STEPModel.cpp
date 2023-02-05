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
/*virtual*/ void CSTEPModel::ZoomToInstance(CSTEPInstance* /*pSTEPInstance*/)
{
	ASSERT(FALSE);//TODO
//	ASSERT(iInstanceID != 0);
//	ASSERT(m_mapProductInstances.find(iInstanceID) != m_mapProductInstances.end());
//
//	m_fXmin = FLT_MAX;
//	m_fXmax = -FLT_MAX;
//	m_fYmin = FLT_MAX;
//	m_fYmax = -FLT_MAX;
//	m_fZmin = FLT_MAX;
//	m_fZmax = -FLT_MAX;
//
//	auto pProductInstance = m_mapProductInstances[iInstanceID];
//
//	pProductInstance->GetDimensions(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
//
//	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
//	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
//	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
//
//#ifndef _LINUX
//	LOG_DEBUG("X/Y/Z min: " << m_fXmin << ", " << m_fYmin << ", " << m_fZmin);
//	LOG_DEBUG("X/Y/Z max: " << m_fXmax << ", " << m_fYmax << ", " << m_fZmax);
//	LOG_DEBUG("World's bounding sphere diameter: " << m_fBoundingSphereDiameter);
//#endif // _LINUX
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPModel::ZoomOut() /*override*/
{
	ASSERT(FALSE);//TODO
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
	map<int_t, CProductInstance*>::const_iterator itProductInstance = m_mapProductInstances.find(iID);
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

	/*
	* Calculate
	*/
	map<int_t, CProductDefinition*>::iterator itProductDefinitions = m_mapProductDefinitions.begin();
	for (; itProductDefinitions != m_mapProductDefinitions.end(); itProductDefinitions++)
	{
		itProductDefinitions->second->CalculateMinMaxTransform(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax);
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	/*
	* Translations
	*/

	// [0.0 -> X/Y/Zmin + X/Y/Zmax]
	m_fXTranslation -= m_fXmin;
	m_fYTranslation -= m_fYmin;
	m_fZTranslation -= m_fZmin;

	// center
	m_fXTranslation -= ((m_fXmax - m_fXmin) / 2.0f);
	m_fYTranslation -= ((m_fYmax - m_fYmin) / 2.0f);
	m_fZTranslation -= ((m_fZmax - m_fZmin) / 2.0f);

	// [-1.0 -> 1.0]
	m_fXTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fYTranslation /= (m_fBoundingSphereDiameter / 2.0f);
	m_fZTranslation /= (m_fBoundingSphereDiameter / 2.0f);

#ifndef _LINUX
	LOG_DEBUG("X/Y/Z min: " << m_fXmin << ", " << m_fYmin << ", " << m_fZmin);
	LOG_DEBUG("X/Y/Z max: " << m_fXmax << ", " << m_fYmax << ", " << m_fZmax);
	LOG_DEBUG("World's bounding sphere diameter: " << m_fBoundingSphereDiameter);
#endif // _LINUX

	/*
	* Scale and Center
	*/

	itProductDefinitions = m_mapProductDefinitions.begin();
	for (; itProductDefinitions != m_mapProductDefinitions.end(); itProductDefinitions++)
	{
		itProductDefinitions->second->ScaleAndCenter(m_fXmin, m_fXmax, m_fYmin, m_fYmax, m_fZmin, m_fZmax, m_fBoundingSphereDiameter);
	}

	m_fXmin = -1.f;
	m_fXmax = 1.f;
	m_fYmin = -1.f;
	m_fYmax = 1.f;
	m_fZmin = -1.f;
	m_fZmax = 1.f;

	m_fBoundingSphereDiameter = 2.f;
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

		auto pProductDefinition = LoadProductDefinition(iProductDefinitionInstance);
		ASSERT(m_mapProductDefinitions.find(pProductDefinition->getExpressID()) == m_mapProductDefinitions.end());

		m_mapProductDefinitions[pProductDefinition->getExpressID()] = pProductDefinition;
	}	
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CSTEPModel::LoadProductDefinition(int_t iProductDefinitionInstance)
{
	auto pProductDefinition = new CProductDefinition();

	pProductDefinition->m_iExpressID = internalGetP21Line(iProductDefinitionInstance);
	pProductDefinition->m_iInstance = iProductDefinitionInstance;

	char* szId = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "id", sdaiSTRING, &szId);
	pProductDefinition->m_strId = szId != nullptr ? CA2W(szId) : L"";

	char* szName = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "name", sdaiSTRING, &szName);
	pProductDefinition->m_strName = szName != nullptr ? CA2W(szName) : L"";

	char* szDescription = nullptr;
	sdaiGetAttrBN(iProductDefinitionInstance, "description", sdaiSTRING, &szDescription);
	pProductDefinition->m_strDescription = szDescription != nullptr ? CA2W(szDescription) : L"";

	int_t iProductDefinitionFormationInstance = 0;
	sdaiGetAttrBN(iProductDefinitionInstance, "formation", sdaiINSTANCE, &iProductDefinitionFormationInstance);

	int_t iProductInstance = 0;
	sdaiGetAttrBN(iProductDefinitionFormationInstance, "of_product", sdaiINSTANCE, &iProductInstance);

	char* szProductId = nullptr;
	sdaiGetAttrBN(iProductInstance, "id", sdaiSTRING, &szProductId);
	pProductDefinition->m_strProductId = szProductId != nullptr ? CA2W(szProductId) : L"";

	char* szProductName = nullptr;
	sdaiGetAttrBN(iProductInstance, "name", sdaiSTRING, &szProductName);
	pProductDefinition->m_strProductName = szProductName != nullptr ? CA2W(szProductName) : L"";

	return pProductDefinition;
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

	auto pProductDefinition = LoadProductDefinition(iProductDefinitionInstance);
	ASSERT(m_mapProductDefinitions.find(pProductDefinition->getExpressID()) == m_mapProductDefinitions.end());

	if (bRelatingProduct)
	{
		pProductDefinition->m_iRelatingProductRefs++;
	}

	if (bRelatedProduct)
	{
		pProductDefinition->m_iRelatedProductRefs++;
	}

	m_mapProductDefinitions[pProductDefinition->getExpressID()] = pProductDefinition;

	return pProductDefinition;
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
	SetDefaultColor(
			m_iModel,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255,
			50 * 256 * 256 * 256 + 50 * 256 * 256 + 50 * 256 + 255
		);

	setSegmentation(m_iModel, 20, 0.);

	/*
	* Load
	*/
	map<int_t, CProductDefinition*>::iterator itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		if (itProductDefinition->second->getRelatedProductRefs() == 0)
		{
			WalkAssemblyTreeRecursively("", "", itProductDefinition->second, nullptr);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pProductDefinition, MATRIX* pParentMatrix)
{
	map<int_t, CAssembly*>::iterator itAssembly = m_mapAssemblies.begin();
	for (; itAssembly != m_mapAssemblies.end(); itAssembly++)
	{
		CAssembly* pAssembly = itAssembly->second;

		if (pAssembly->m_pRelatingProductDefinition == pProductDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(m_iModel, internalGetInstanceFromP21Line(m_iModel, pAssembly->getExpressID()), &owlInstanceMatrix);

			if (owlInstanceMatrix && GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "Transformation")) 
			{
				owlInstanceMatrix = GetObjectProperty(owlInstanceMatrix, "matrix");
			}

			ASSERT(owlInstanceMatrix == 0 || GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "Matrix") || 
				GetInstanceClass(owlInstanceMatrix) == GetClassByName(GetModel(owlInstanceMatrix), "MatrixMultiplication"));

			MATRIX	matrix;
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

    //if (pProductDefinition->getRelatingProductRefs() == 0)
	{
		int_t myProductDefinitionInstanceHandle = internalGetInstanceFromP21Line(m_iModel, pProductDefinition->getExpressID());

		int64_t	owlInstanceProductDefinition = 0;
		owlBuildInstance(m_iModel, myProductDefinitionInstanceHandle, &owlInstanceProductDefinition);

		pProductDefinition->Calculate();

		cleanMemory(m_iModel, 0);

		auto pProductInstance = new CProductInstance(m_iID++, pProductDefinition, pParentMatrix);
		m_mapProductInstances[pProductInstance->getID()] = pProductInstance;

		pProductDefinition->m_vecProductInstances.push_back(pProductInstance);
	} // if (pProductDefinition->getRelatingProductRefs() == 0)
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel(m_iModel);
		m_iModel = 0;
	}

	map<int_t, CProductDefinition*>::iterator itProductDefinition = m_mapProductDefinitions.begin();
	for (; itProductDefinition != m_mapProductDefinitions.end(); itProductDefinition++)
	{
		delete itProductDefinition->second;
	}
	m_mapProductDefinitions.clear();

	map<int_t, CProductInstance*>::iterator itProductInstance = m_mapProductInstances.begin();
	for (; itProductInstance != m_mapProductInstances.end(); itProductInstance++)
	{
		delete itProductInstance->second;
	}
	m_mapProductInstances.clear();

	map<int_t, CAssembly*>::iterator itAssembly = m_mapAssemblies.begin();
	for (; itAssembly != m_mapAssemblies.end(); itAssembly++)
	{
		delete itAssembly->second;
	}
	m_mapAssemblies.clear();

	m_iID = 1;
}
