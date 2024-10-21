#pragma once

#include "_geometry.h"
#include "Model.h"

#include <map>
using namespace std;

#define EMPTY_INSTANCE L"---<EMPTY>---"

// ************************************************************************************************
class CAP242Assembly;
class CProductDefinition;
class CProductInstance;

// ************************************************************************************************
class CAP242Model : public CModel
{

private: // Members
	
	CEntityProvider* m_pEntityProvider;
	map<ExpressID, CProductDefinition*> m_mapExpressID2Definition; // Express ID : Product Definition	
	map<int64_t, CProductInstance*> m_mapID2Instance; // ID : Product Instance
	map<ExpressID, CAP242Assembly*> m_mapExpressIDAssembly; // Express ID : Assembly

	int64_t m_iID;

	bool m_bUpdteVertexBuffers; // when the first instance with geometry is loaded

public: // Members

	static uint32_t DEFAULT_COLOR;

public: // Methods
	
	CAP242Model();
	virtual ~CAP242Model();

	void PreLoadProductDefinition(SdaiInstance iProductDefinitionInstance);

	// CModel	
	virtual CEntityProvider* GetEntityProvider() const override;
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;
	virtual CInstanceBase* LoadInstance(SdaiInstance /*iSdaiInstance*/) override { ASSERT(FALSE); return nullptr; };

	const map<ExpressID, CProductDefinition*>& GetDefinitions() const { return m_mapExpressID2Definition; }
	const map<int64_t, CProductInstance*>& GetInstances() const { return m_mapID2Instance; }
	const map<ExpressID, CAP242Assembly*>& GetAssemblies() const { return m_mapExpressIDAssembly; }
	CProductInstance* getProductInstanceByID(int64_t iID) const;

	void Scale(); // [-1, 1]

	void Save(const wchar_t * szPath);
	void Load(const wchar_t * szPath);
	void Load(const wchar_t* szPath, SdaiModel iModel);

private: // Methods
	
	void LoadProductDefinitions();
	CProductDefinition* LoadProductDefinition(SdaiInstance iProductDefinitionInstance);
	CProductDefinition* GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(CProductDefinition* pDefinition, _matrix4x3* pParentMatrix);
	void Clean();
};

