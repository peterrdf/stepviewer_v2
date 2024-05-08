#pragma once

#include "_geometry.h"
#include "Model.h"

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
	map<ExpressID, CProductDefinition*> m_mapExpressID2Definition; // Express ID : Product Definition	
	map<int64_t, CProductInstance*> m_mapID2Instance; // ID : Product Instance
	map<ExpressID, CAssembly*> m_mapExpressIDAssembly; // Express ID : Assembly

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

	const map<ExpressID, CProductDefinition*>& GetDefinitions() const { return m_mapExpressID2Definition; }
	const map<int64_t, CProductInstance*>& GetInstances() const { return m_mapID2Instance; }
	const map<ExpressID, CAssembly*>& GetAssemblies() const { return m_mapExpressIDAssembly; }
	CProductInstance* getProductInstanceByID(int64_t iID) const;

	void ScaleAndCenter(); // [-1, 1]

	void Save(const wchar_t * szPath);
	void Load(const wchar_t * szPath);
	void Load(const wchar_t* szPath, SdaiModel iModel);

private: // Methods
	
	void LoadProductDefinitions();
	CProductDefinition* LoadProductDefinition(SdaiInstance iProductDefinitionInstance);
	CProductDefinition* GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(const char* szStepName, const char* szGroupName, CProductDefinition* pDefinition, _matrix3x4* pParentMatrix);
	void Clean();
};

