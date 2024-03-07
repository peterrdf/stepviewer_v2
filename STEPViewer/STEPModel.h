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
	
	CEntityProvider* m_pEntityProvider;
	map<int_t, CProductDefinition*> m_mapExpressID2Definition; // Express ID : Product Definition	
	map<int_t, CProductInstance*> m_mapID2Instance; // ID : Product Instance
	map<int_t, CAssembly*> m_mapExpressIDAssembly; // Express ID : Assembly

	int64_t m_iID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Methods
	
	CSTEPModel();
	virtual ~CSTEPModel();

	void PreLoadProductDefinition(SdaiInstance iProductDefinitionInstance);

	// CModel
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;

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
	CProductDefinition* LoadProductDefinition(SdaiInstance iProductDefinitionInstance);
	CProductDefinition* GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pDefinition, MATRIX* pParentMatrix);
	void Clean();
};

