#pragma once

#include "_step_mvc.h"

#include <map>
using namespace std;

// ************************************************************************************************
class CAP242Assembly;
class CAP242ProductDefinition;
class CAP242ProductInstance;

// ************************************************************************************************
class CAP242Model : public _step_model
{

private: // Members

	map<ExpressID, CAP242Assembly*> m_mapExpressIDAssembly; // Express ID : Assembly

public: // Methods
	
	CAP242Model();
	virtual ~CAP242Model();

protected: // Methods

	// _ap_model
	virtual void attachModelCore() override;
	virtual void clean() override;

public: // Methods

	const map<ExpressID, CAP242Assembly*>& GetAssemblies() const { return m_mapExpressIDAssembly; }

	void Save(const wchar_t* szPath);

private: // Methods
	
	void LoadProductDefinitions();
	CAP242ProductDefinition* LoadProductDefinition(SdaiInstance iProductDefinitionInstance);
	CAP242ProductDefinition* GetProductDefinition(SdaiInstance iProductDefinitionInstance, bool bRelatingProduct, bool bRelatedProduct);
	void LoadAssemblies();
	void LoadGeometry();
	void WalkAssemblyTreeRecursively(CAP242ProductDefinition* pProductDefinition, CAP242Assembly* pAssembly, _matrix4x3* pParentMatrix);	
};

