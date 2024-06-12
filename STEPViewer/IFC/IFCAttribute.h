#pragma once

#include <map>
#include <vector>
using namespace std;

// ************************************************************************************************
class CIFCAttribute
{

private: // Members

	SdaiAttr m_pInstance;

public: // Methods

	CIFCAttribute(SdaiAttr pInstance);
	virtual ~CIFCAttribute();

	SdaiAttr GetInstance() const { return m_pInstance; }
	SdaiPrimitiveType GetType() const;
};

// ************************************************************************************************
class CIFCAttributeProvider
{

private: // Members

	map<SdaiInstance, vector<CIFCAttribute*>> m_mapInstanceAttributes;

public: // Methods

	CIFCAttributeProvider();
	virtual ~CIFCAttributeProvider();

	const vector<CIFCAttribute*>& GetInstanceAttributes(SdaiInstance iInstance);

private: // Methods

	void LoadInstanceAttributes(SdaiEntity iEntity, SdaiInstance iInstance, vector<CIFCAttribute*>& vecAttributes);
};

