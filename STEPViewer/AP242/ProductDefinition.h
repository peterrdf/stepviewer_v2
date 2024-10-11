#pragma once

#include "_oglUtils.h"

#include "ProductInstance.h"
#include "InstanceBase.h"

#include <string>
#include <vector>
#include <map>
using namespace std;

// ************************************************************************************************
class CProductDefinition
	: public _geometry
{
	friend class CSTEPModel;

private: // Members

	ExpressID m_iExpressID;	
	wchar_t* m_szId;
	wchar_t* m_szName;
	wchar_t* m_szDescription;
	wchar_t* m_szProductId;
	wchar_t* m_szProductName;

	SdaiInteger m_iRelatingProducts; // if == 0 then it has geometry, otherwise it is a placeholder
	SdaiInteger m_iRelatedProducts;  // if == 0 then it is a root element

	vector<CProductInstance*> m_vecInstances;
	int32_t m_iNextInstance;

public: // Methods

	// ctor/dtor
	CProductDefinition(SdaiInstance iSdaiInstance);
	virtual ~CProductDefinition();

	// _geometry
	virtual OwlModel getModel() const override;
	virtual int64_t calculateInstance(int64_t* piVertexBufferSize, int64_t* piIndexBufferSize) override;

	void CalculateMinMaxTransform(
		CProductInstance* pInstance, 
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);
	void CalculateMinMaxTransform(
		CProductInstance* pInstance,
		float fXTranslation, float fYTranslation, float fZTranslation,
		float& fXmin, float& fXmax,
		float& fYmin, float& fYmax,
		float& fZmin, float& fZmax);
	void Scale(float fScaleFactor);
	
	SdaiInstance GetInstance() const { return (SdaiInstance)m_iInstance; }
	SdaiModel GetModel() const { return sdaiGetInstanceModel(GetInstance()); }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_szId; }
	const wchar_t* GetName() const { return m_szName; }
	const wchar_t* GetDescription() const { return m_szDescription; }
	const wchar_t* GetProductId() const { return m_szProductId; }
	const wchar_t* GetProductName() const { return m_szProductName; }

	SdaiInteger GetRelatingProducts() const { return m_iRelatingProducts; }
	SdaiInteger GetRelatedProducts() const { return m_iRelatedProducts; }

	const vector<CProductInstance*>& GetInstances() const { return m_vecInstances; }
	int32_t GetNextInstance();

private: // Methods

	void Calculate();
};

