#pragma once

#include "_ap_mvc.h"
#include "InstanceBase.h"
#include "Entity.h"

#include <map>

using namespace std;

// ************************************************************************************************
class CModel : public _ap_model
{

protected: // Members

	// World's dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;
	float m_dOriginalBoundingSphereDiameter;
	float m_fBoundingSphereDiameter;

	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

public: // Methods

	CModel(enumAPModelType enType);
	virtual ~CModel()
	{}

	virtual CEntityProvider* GetEntityProvider() const PURE;
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const PURE;
	virtual void ZoomToInstance(CInstanceBase* pInstance) PURE;
	virtual void ZoomOut() PURE;
	virtual CInstanceBase* LoadInstance(OwlInstance iInstance) PURE;

	// Model
	SdaiModel GetInstance() const { return (SdaiModel)m_iModel; }

	// World's dimensions
	double GetOriginalBoundingSphereDiameter() const { return m_dOriginalBoundingSphereDiameter; }
	float GetBoundingSphereDiameter() const { return m_fBoundingSphereDiameter; }
	void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;
	
	template<typename T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}
};

