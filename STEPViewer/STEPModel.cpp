#include "stdafx.h"

#include "STEPModel.h"
#include "ProductDefinition.h"
#include "Assembly.h"
#include "_3DUtils.h"

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
/*static*/ uint32_t CSTEPModel::DEFAULT_COLOR =
	256 * 256 * 256 * DEFAULT_COLOR_R +
	256 * 256 * DEFAULT_COLOR_G +
	256 * DEFAULT_COLOR_B +
	DEFAULT_COLOR_A;

// ************************************************************************************************
CSTEPModel::CSTEPModel()
	: CModel(enumModelType::STEP)
	, m_pEntityProvider(nullptr)
	, m_mapExpressID2Definition()
	, m_mapID2Instance()
	, m_mapExpressIDAssembly()
	, m_iID(1)
	, m_bUpdteVertexBuffers(true)
{
}

// ------------------------------------------------------------------------------------------------
CSTEPModel::~CSTEPModel()
{
	Clean();
}

void CSTEPModel::PreLoadProductDefinition(SdaiInstance iProductDefinitionInstance)
{
	if (m_bUpdteVertexBuffers)
	{
		_vector3d vecOriginalBBMin;
		_vector3d vecOriginalBBMax;
		if (GetInstanceGeometryClass(iProductDefinitionInstance) &&
			GetBoundingBox(
				iProductDefinitionInstance,
				(double*)&vecOriginalBBMin,
				(double*)&vecOriginalBBMax))
		{
			TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
			SetVertexBufferOffset(
				m_iModel,
				-(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.,
				-(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.,
				-(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.);

			// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
			ClearedExternalBuffers(m_iModel);

			m_bUpdteVertexBuffers = false;
		}
	} // if (m_bUpdteVertexBuffers)
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CEntityProvider* CSTEPModel::GetEntityProvider() const /*override*/
{
	return m_pEntityProvider;
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ CInstanceBase* CSTEPModel::GetInstanceByExpressID(int64_t iExpressID) const /*override*/
{
	for (auto& item : m_mapID2Instance) {
		if (auto pDef = item.second->GetProductDefinition()) {
			if (pDef->GetExpressID() == iExpressID) {
				return item.second;
			}
		}
	}
	return nullptr;
}

// --------------------------------------------------------------------------------------------
/*virtual*/ void CSTEPModel::ZoomToInstance(CInstanceBase* pInstance) /*override*/
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
CProductInstance* CSTEPModel::getProductInstanceByID(int64_t iID) const
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
void CSTEPModel::Scale()
{
	/* World */
	m_dOriginalBoundingSphereDiameter = 2.;
	m_fBoundingSphereDiameter = 2.f;

	m_fXTranslation = 0.f;
	m_fYTranslation = 0.f;
	m_fZTranslation = 0.f;

	/* Min/Max */
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;	
	
	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->hasGeometry())
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
		// TODO: new status bar for geometry
		/*::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);*/

		return;
	}

	/* World */
	m_dOriginalBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_dOriginalBoundingSphereDiameter = max(m_dOriginalBoundingSphereDiameter, m_fZmax - m_fZmin);

	m_fBoundingSphereDiameter = m_dOriginalBoundingSphereDiameter;

	TRACE(L"\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);

	/* Scale */
	itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->hasGeometry())
		{
			continue;
		}

		itDefinition->second->Scale(m_fBoundingSphereDiameter / 2.f);
	}

	/* Min/Max */
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (!itDefinition->second->hasGeometry())
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
		::MessageBox(
			::AfxGetMainWnd()->GetSafeHwnd(),
			L"Internal error.", L"Error", MB_ICONERROR | MB_OK);

		return;
	}

	/* World */
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);
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

	SdaiModel iModel = sdaiOpenModelBNUnicode(0, szPath, L"");
	if (iModel == 0)
	{
		MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Failed to open the model.", L"Error", MB_ICONERROR | MB_OK);

		return;
	}

	Load(szPath, iModel);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Load(const wchar_t* szPath, SdaiModel iModel)
{
	Clean();

	m_iModel = iModel;
	m_strPath = szPath;
	
	m_pEntityProvider = new CEntityProvider(GetInstance());

	LoadProductDefinitions();

	LoadAssemblies();

	LoadGeometry();

	Scale();
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadProductDefinitions()
{
	SdaiAggr pProductDefinitionInstances = sdaiGetEntityExtentBN(GetInstance(), "PRODUCT_DEFINITION");

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
	PreLoadProductDefinition(iProductDefinitionInstance);

	return new CProductDefinition(iProductDefinitionInstance);
}

// ------------------------------------------------------------------------------------------------
CProductDefinition* CSTEPModel::GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct)
{
	ExpressID iExpressID = internalGetP21Line(iProductDefinitionInstance);

	auto itDefinition = m_mapExpressID2Definition.find(iExpressID);
	if (itDefinition != m_mapExpressID2Definition.end())
	{
		if (bRelatingProduct)
		{
			itDefinition->second->m_iRelatingProducts++;
		}

		if (bRelatedProduct)
		{
			itDefinition->second->m_iRelatedProducts++;
		}

		return itDefinition->second;
	}

	auto pDefinition = LoadProductDefinition(iProductDefinitionInstance);
	ASSERT(m_mapExpressID2Definition.find(pDefinition->GetExpressID()) == m_mapExpressID2Definition.end());

	if (bRelatingProduct)
	{
		pDefinition->m_iRelatingProducts++;
	}

	if (bRelatedProduct)
	{
		pDefinition->m_iRelatedProducts++;
	}

	m_mapExpressID2Definition[pDefinition->GetExpressID()] = pDefinition;

	return pDefinition;
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadAssemblies()
{
	SdaiAggr pNextAssemblyUsageOccurrenceInstances = sdaiGetEntityExtentBN(GetInstance(), "NEXT_ASSEMBLY_USAGE_OCCURRENCE");

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

		auto pAssembly = new CAssembly(pNextAssemblyUsageOccurrenceInstance, pRelatingProductDefinition, pRelatedProductDefinition);
		ASSERT(m_mapExpressIDAssembly.find(pAssembly->GetExpressID()) == m_mapExpressIDAssembly.end());

		m_mapExpressIDAssembly[pAssembly->GetExpressID()] = pAssembly;
	}	
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::LoadGeometry()
{
	// Load
	auto itDefinition = m_mapExpressID2Definition.begin();
	for (; itDefinition != m_mapExpressID2Definition.end(); itDefinition++)
	{
		if (itDefinition->second->GetRelatedProducts() == 0)
		{
			WalkAssemblyTreeRecursively(itDefinition->second, nullptr);
		}
	}
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::WalkAssemblyTreeRecursively(CProductDefinition* pDefinition, _matrix4x3* pParentMatrix)
{
	auto itAssembly = m_mapExpressIDAssembly.begin();
	for (; itAssembly != m_mapExpressIDAssembly.end(); itAssembly++)
	{
		auto pAssembly = itAssembly->second;

		if (pAssembly->GetRelatingProductDefinition() == pDefinition)
		{
			int64_t	owlInstanceMatrix = 0;
			owlBuildInstance(GetInstance(), internalGetInstanceFromP21Line(GetInstance(), pAssembly->GetExpressID()), &owlInstanceMatrix);

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

			WalkAssemblyTreeRecursively(pAssembly->GetRelatedProductDefinition(), &matrix);
		} // if (pAssembly->m_pRelatingProductDefinition == ...
	} // for (; itAssembly != ...

	auto pInstance = new CProductInstance(m_iID++, pDefinition, pParentMatrix);
	m_mapID2Instance[pInstance->GetID()] = pInstance;

	pDefinition->m_vecInstances.push_back(pInstance);
}

// ------------------------------------------------------------------------------------------------
void CSTEPModel::Clean()
{
	if (m_iModel != 0)
	{
		sdaiCloseModel((SdaiModel)m_iModel);
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
