#pragma once

#include "_ap242_geometry.h"

// ************************************************************************************************
class _ap242_product_definition : public _ap242_geometry
{
	friend class _ap242_model;

private: // Members

	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	wchar_t* m_szProductId;
	wchar_t* m_szProductName;

	SdaiInteger m_iRelatingProducts; // if == 0 then it has geometry, otherwise it is a placeholder
	SdaiInteger m_iRelatedProducts;  // if == 0 then it is a root element

public: // Methods

	_ap242_product_definition(OwlInstance owlInstance, SdaiInstance sdaiInstance);
	virtual ~_ap242_product_definition();

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

// ************************************************************************************************
class _ap242_assembly
{

private: // Members

	SdaiInstance m_iInstance;
	ExpressID m_iExpressID;
	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	_ap242_product_definition* m_pRelatingProductDefinition;
	_ap242_product_definition* m_pRelatedProductDefinition;

public: // Methods

	_ap242_assembly(SdaiInstance iInstance, _ap242_product_definition* pRelatingProductDefinition, _ap242_product_definition* pRelatedProductDefinition);
	virtual ~_ap242_assembly();

	SdaiInstance GetInstance() const { return m_iInstance; }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	_ap242_product_definition* GetRelatingProductDefinition() const { return m_pRelatingProductDefinition; }
	_ap242_product_definition* GetRelatedProductDefinition() const { return m_pRelatedProductDefinition; }
};