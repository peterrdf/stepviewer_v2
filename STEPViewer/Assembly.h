#pragma once

#include <string>
#include <vector>

using namespace std;

// ************************************************************************************************
class CProductDefinition;

// ************************************************************************************************
class CAssembly
{

private: // Members

	SdaiInstance m_iInstance;
	ExpressID m_iExpressID;
	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	CProductDefinition* m_pRelatingProductDefinition;
	CProductDefinition* m_pRelatedProductDefinition;

public: // Methods

	CAssembly(SdaiInstance iInstance, CProductDefinition* pRelatingProductDefinition, CProductDefinition* pRelatedProductDefinition);
	virtual ~CAssembly();

	SdaiInstance GetInstance() const { return m_iInstance; }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	CProductDefinition* GetRelatingProductDefinition() const { return m_pRelatingProductDefinition; }
	CProductDefinition* GetRelatedProductDefinition() const { return m_pRelatedProductDefinition; }
};

