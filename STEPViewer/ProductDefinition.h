#pragma once

#include "_oglUtils.h"

#include "Generic.h"
#include "ProductInstance.h"
#include "InstanceBase.h"

#include <string>
#include <vector>
#include <map>
using namespace std;

// ************************************************************************************************
class CProductDefinition : public _instance
{
	friend class CSTEPModel;

private: // Members

	SdaiInstance m_iInstance;
	ExpressID m_iExpressID;	
	wstring m_strId;
	wstring m_strName;
	wstring m_strDescription;
	wstring m_strProductId;
	wstring m_strProductName;

	// Instances		
	int_t m_iRelatingProductRefs; // if == 0 then it has geometry, otherwise it is a placeholder
	int_t m_iRelatedProductRefs; // if == 0 then it is a root element
	vector<CProductInstance*> m_vecProductInstances;
	int m_iNextProductInstance;

	bool m_bCalculated;

public: // Methods

	// ctor/dtor
	CProductDefinition(SdaiInstance iInstance);
	virtual ~CProductDefinition();
	
	void Calculate();	
	
	void CalculateMinMaxTransform(
		float & fXmin, float & fXmax, 
		float & fYmin, float & fYmax, 
		float & fZmin, float & fZmax);

	void CalculateMinMaxTransform(
		float fXTranslation, float fYTranslation, float fZTranslation,
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax);

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

	void ScaleAndCenter(
		float fXmin, float fXmax, 
		float fYmin, float fYmax, 
		float fZmin, float fZmax, 
		float fResoltuion);

	void Scale(float fResoltuion);
	
	SdaiInstance GetInstance() const { return m_iInstance; }
	ExpressID GetExpressID() const { return m_iExpressID; }
	const wchar_t* GetId() const { return m_strId.c_str(); }
	const wchar_t* GetName() const { return m_strName.c_str(); }
	const wchar_t* GetDescription() const { return m_strDescription.c_str(); }
	const wchar_t* GetProductId() const { return m_strProductId.c_str(); }
	const wchar_t* GetProductName() const { return m_strProductName.c_str(); }
	int_t GetRelatingProductRefs() const { return m_iRelatingProductRefs; }
	int_t GetRelatedProductRefs() const { return m_iRelatedProductRefs; }
	const vector<CProductInstance*>& GetInstances() const { return m_vecProductInstances; }

	int GetNextProductInstance();
	
	int64_t getVertexLength() const;
};

