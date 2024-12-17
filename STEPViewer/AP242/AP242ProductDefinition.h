#pragma once

#include "_ap_geometry.h"

#include "AP242ProductInstance.h"

#include <string>
#include <vector>
#include <map>
using namespace std;

// ************************************************************************************************
class CAP242ProductDefinition : public _ap_geometry
{
	friend class CAP242Model;

private: // Members

	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	wchar_t* m_szProductId;
	wchar_t* m_szProductName;

	SdaiInteger m_iRelatingProducts; // if == 0 then it has geometry, otherwise it is a placeholder
	SdaiInteger m_iRelatedProducts;  // if == 0 then it is a root element

public: // Methods

	CAP242ProductDefinition(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~CAP242ProductDefinition();

protected: // Methods

	// _geometry
	virtual void preCalculate() override;
	
public: // Properties

	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	const wchar_t* GetProductId() const { return m_szProductId; }
	const wchar_t* GetProductName() const { return m_szProductName; }

	SdaiInteger GetRelatingProducts() const { return m_iRelatingProducts; }
	SdaiInteger GetRelatedProducts() const { return m_iRelatedProducts; }
};

