#pragma once

#include "InstanceBase.h"
#include "Entity.h"

#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
enum class enumModelType : int
{
	Unknown = -1,
	STEP = 0,
	IFC = 1
};

// ------------------------------------------------------------------------------------------------
class CModel
{

protected: // Members

	// --------------------------------------------------------------------------------------------
	wstring		  m_strFilePath; // Input file	
	enumModelType m_enModelType;
	SdaiModel	  m_iModel; // Model

	// Offset
	float m_dOriginalBoundingSphereDiameter;

	// World's dimensions
	float m_fXmin;
	float m_fXmax;
	float m_fYmin;
	float m_fYmax;
	float m_fZmin;
	float m_fZmax;
	float m_fBoundingSphereDiameter;

	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

public: // Methods

	// --------------------------------------------------------------------------------------------
	CModel(enumModelType enModelType);

	// --------------------------------------------------------------------------------------------
	virtual ~CModel();

	// --------------------------------------------------------------------------------------------
	const wchar_t* GetModelName() const;

	// --------------------------------------------------------------------------------------------	
	enumModelType GetType() const;

	// --------------------------------------------------------------------------------------------	
	SdaiModel GetSdaiModel() const;

	// --------------------------------------------------------------------------------------------	
	double GetOriginalBoundingSphereDiameter() const { return m_dOriginalBoundingSphereDiameter; }
	void GetWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const;
	void GetWorldTranslations(float& fXTranslation, float& fYTranslation, float& fZTranslation) const;
	float GetBoundingSphereDiameter() const;

	// --------------------------------------------------------------------------------------------	
	virtual CEntityProvider* GetEntityProvider() const PURE;

	// --------------------------------------------------------------------------------------------	
	virtual CInstanceBase* GetInstanceByExpressID(int64_t iExpressID) const PURE;

	// --------------------------------------------------------------------------------------------
	virtual void ZoomToInstance(CInstanceBase* pInstance) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void ZoomOut() PURE;

	// --------------------------------------------------------------------------------------------	
	template<typename T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}
};

