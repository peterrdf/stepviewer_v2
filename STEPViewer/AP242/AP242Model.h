#pragma once

#include "_geometry.h"
#include "Model.h"

#include <map>
using namespace std;

#define EMPTY_INSTANCE L"---<EMPTY>---"

// ************************************************************************************************
class CAP242Assembly;
class CAP242ProductDefinition;
class CAP242ProductInstance;

// ************************************************************************************************
class CAP242Model : public CModel
{

private: // Members
	
	CEntityProvider* m_pEntityProvider;
	map<ExpressID, CAP242ProductDefinition*> m_mapExpressID2Definition; // Express ID : Product Definition	
	map<int64_t, CAP242ProductInstance*> m_mapID2Instance; // ID : Product Instance
	map<ExpressID, CAP242Assembly*> m_mapExpressIDAssembly; // Express ID : Assembly

	int64_t m_iID;

public: // Members

	static uint32_t DEFAULT_COLOR;

public: // Methods
	
	CAP242Model();
	virtual ~CAP242Model();

	// CModel
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const override;
	virtual void ZoomToInstance(CInstanceBase* pInstance) override;
	virtual void ZoomOut() override;
	virtual CInstanceBase* LoadInstance(OwlInstance /*iInstance*/) override { ASSERT(FALSE); return nullptr; };

	const map<ExpressID, CAP242ProductDefinition*>& GetDefinitions() const { return m_mapExpressID2Definition; }
	const map<int64_t, CAP242ProductInstance*>& GetInstances() const { return m_mapID2Instance; }
	const map<ExpressID, CAP242Assembly*>& GetAssemblies() const { return m_mapExpressIDAssembly; }
	CAP242ProductInstance* getProductInstanceByID(int64_t iID) const;

	void Scale(); // [-1, 1]

	void Save(const wchar_t * szPath);
	void Load(const wchar_t * szPath);
	void Attach(const wchar_t* szPath, SdaiModel iModel);

private: // Methods
	
	void LoadProductDefinitions();
	CAP242ProductDefinition* LoadProductDefinition(SdaiInstance iProductDefinitionInstance);
	CAP242ProductDefinition* GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(CAP242ProductDefinition* pProductDefinition, CAP242Assembly* pParentAssembly, _matrix4x3* pParentMatrix);
	void Clean();
};

