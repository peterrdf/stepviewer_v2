#pragma once

#include "Model.h"
#include "Generic.h"

#include <map>

using namespace std;

#define EMPTY_INSTANCE L"---<EMPTY>---"

// ------------------------------------------------------------------------------------------------
class CAssembly;
class CProductDefinition;
class CProductInstance;

// ------------------------------------------------------------------------------------------------
// Model
class CSTEPModel : public CModel
{

private: // Members
	
	int_t m_iModel;
	wstring m_strSTEPFile;

	CEntityProvider* m_pEntityProvider;
	map<int_t, CProductDefinition*> m_mapExpressID2Definition; // Express ID : Product Definition	
	map<int_t, CProductInstance*> m_mapID2Instance; // ID : Product Instance
	map<int_t, CAssembly*> m_mapExpressIDAssembly; // Express ID : Assembly

	int64_t m_iID;

	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;	
	float m_fBoundingSphereDiameter;

	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

public: // Methods
	
	CSTEPModel();
	virtual ~CSTEPModel();

	// CModel
	virtual const wchar_t* GetModelName() const override;
	virtual int64_t GetInstance() const override;
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual void ZoomToInstance(CInstance* pInstance) override;
	virtual void ZoomOut() override;

	void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;	
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;
	float GetBoundingSphereDiameter() const;

	const map<int_t, CProductDefinition*>& GetDefinitions();
	const map<int_t, CProductInstance*>& GetInstances();
	const map<int_t, CAssembly*>& GetAssemblies();
	CProductInstance* getProductInstanceByID(int_t iID) const;

	void ScaleAndCenter(); // [-1, 1]

	void Save(const wchar_t * szPath);
	void Load(const wchar_t * szPath);
	void Load(const wchar_t* szPath, int64_t iModel);

private: // Methods
	
	void LoadProductDefinitions();
	CProductDefinition* LoadProductDefinition(int_t iProductDefinitionInstance);
	CProductDefinition* GetProductDefinition(int_t iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pDefinition, MATRIX* pParentMatrix);
	void Clean();
};

