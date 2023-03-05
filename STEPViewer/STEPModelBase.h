#pragma once

#include "STEPInstance.h"
#include "Entity.h"

#include <map>

using namespace std;

// ------------------------------------------------------------------------------------------------
enum class enumSTEPModelType
{
	Unknown = -1,
	STEP = 0,
	IFC = 1
};

// ------------------------------------------------------------------------------------------------
class CSTEPModelBase
{

protected: // Members

	// --------------------------------------------------------------------------------------------
	enumSTEPModelType m_enSTEPModelType;

public: // Methods

	// --------------------------------------------------------------------------------------------
	CSTEPModelBase(enumSTEPModelType enSTEPModelType);

	// --------------------------------------------------------------------------------------------
	virtual ~CSTEPModelBase();

	// --------------------------------------------------------------------------------------------
	virtual const wchar_t* GetModelName() const PURE;

	// --------------------------------------------------------------------------------------------	
	enumSTEPModelType GetType() const;

	// --------------------------------------------------------------------------------------------	
	virtual int64_t GetInstance() const PURE;	

	// --------------------------------------------------------------------------------------------	
	virtual CEntityProvider* GetEntityProvider() const PURE;

	// --------------------------------------------------------------------------------------------
	virtual void ZoomToInstance(CSTEPInstance* pSTEPInstance) PURE;

	// --------------------------------------------------------------------------------------------
	virtual void ZoomOut() PURE;

	// --------------------------------------------------------------------------------------------	
	template<typename T>
	T* As()
	{
		return dynamic_cast<T*>(this);
	}
};

