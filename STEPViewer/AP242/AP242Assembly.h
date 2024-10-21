#pragma once

#include <string>
#include <vector>

using namespace std;

// ************************************************************************************************
class CProductDefinition;

// ************************************************************************************************
class CAP242Assembly
{

private: // Members

	SdaiInstance m_iSdaiInstance;
	ExpressID m_iExpressID;
	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	CProductDefinition* m_pRelatingProductDefinition;
	CProductDefinition* m_pRelatedProductDefinition;

public: // Methods

	CAP242Assembly(SdaiInstance iInstance, CProductDefinition* pRelatingProductDefinition, CProductDefinition* pRelatedProductDefinition);
	virtual ~CAP242Assembly();

	SdaiInstance GetSdaiInstance() const { return m_iSdaiInstance; }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	CProductDefinition* GetRelatingProductDefinition() const { return m_pRelatingProductDefinition; }
	CProductDefinition* GetRelatedProductDefinition() const { return m_pRelatedProductDefinition; }
};

