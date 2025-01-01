#pragma once

#include <map>
#include <vector>
using namespace std;

// ************************************************************************************************
class _ifc_attribute
{

private: // Members

	SdaiAttr m_sdaiAttr;

public: // Methods

	_ifc_attribute(SdaiAttr sdaiAttr);
	virtual ~_ifc_attribute();

	SdaiAttr getSdaiAttr() const { return m_sdaiAttr; }
	SdaiPrimitiveType getType() const;
};

// ************************************************************************************************
class _ifc_attribute_provider
{

private: // Members

	map<SdaiInstance, vector<_ifc_attribute*>> m_mapInstanceAttributes;

public: // Methods

	_ifc_attribute_provider();
	virtual ~_ifc_attribute_provider();

	const vector<_ifc_attribute*>& getInstanceAttributes(SdaiInstance sdaiInstance);

private: // Methods

	void loadInstanceAttributes(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, vector<_ifc_attribute*>& vecAttributes);
};

