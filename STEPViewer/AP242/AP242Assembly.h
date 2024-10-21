#pragma once

#include <string>
#include <vector>

using namespace std;

// ************************************************************************************************
class CAP242ProductDefinition;

// ************************************************************************************************
class CAP242Assembly
{

private: // Members

	SdaiInstance m_iSdaiInstance;
	ExpressID m_iExpressID;
	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	CAP242ProductDefinition* m_pRelatingProductDefinition;
	CAP242ProductDefinition* m_pRelatedProductDefinition;

public: // Methods

	CAP242Assembly(SdaiInstance iInstance, CAP242ProductDefinition* pRelatingProductDefinition, CAP242ProductDefinition* pRelatedProductDefinition);
	virtual ~CAP242Assembly();

	SdaiInstance GetSdaiInstance() const { return m_iSdaiInstance; }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	CAP242ProductDefinition* GetRelatingProductDefinition() const { return m_pRelatingProductDefinition; }
	CAP242ProductDefinition* GetRelatedProductDefinition() const { return m_pRelatedProductDefinition; }
};

