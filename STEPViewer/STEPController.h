#pragma once

#include "STEPInstance.h"
#include "ProductInstance.h"

#include <set>
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CSTEPModelBase;
class CSTEPView;

// ------------------------------------------------------------------------------------------------
// Controller - MVC
class CSTEPController
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Model - MVC
	CSTEPModelBase* m_pModel;

	// --------------------------------------------------------------------------------------------
	// Updating model - disable all notifications
	bool m_bUpdatingModel;

	// --------------------------------------------------------------------------------------------
	// Views - MVC
	set<CSTEPView*> m_setViews;

	// --------------------------------------------------------------------------------------------
	// Selected instance
	CSTEPInstance* m_pSelectedInstance;

	// --------------------------------------------------------------------------------------------
	// UI properties
	int m_iVisibleValuesCountLimit;

	// --------------------------------------------------------------------------------------------
	// UI properties
	BOOL m_bScaleAndCenter;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPController();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPController();

	// --------------------------------------------------------------------------------------------
	// Getter
	CSTEPModelBase* GetModel() const;

	// --------------------------------------------------------------------------------------------
	// Setter
	void SetModel(CSTEPModelBase* pModel);

	// --------------------------------------------------------------------------------------------
	// Support for notifications
	void RegisterView(CSTEPView* pView);

	// --------------------------------------------------------------------------------------------
	// Support for notifications
	void UnRegisterView(CSTEPView* pView);

	// --------------------------------------------------------------------------------------------
	// Getter
	const set<CSTEPView*> & GetViews();

	// --------------------------------------------------------------------------------------------
	// Getter
	template <class T>
	T * GetView()
	{
		set<CSTEPView*>::const_iterator itView = m_setViews.begin();
		for (; itView != m_setViews.end(); itView++)
		{
			T * pView = dynamic_cast<T *>(*itView);
			if (pView != nullptr)
			{
				return pView;
			}
		}

		return nullptr;
	}
		
	// Zoom
	void ZoomToInstance();
	void ZoomOut();

	// Save
	void SaveInstance();

	// --------------------------------------------------------------------------------------------
	// [-1, 1]
	void ScaleAndCenter();

	// --------------------------------------------------------------------------------------------
	// Meta information support
	void ShowMetaInformation(CProductInstance* pInstance);

	// --------------------------------------------------------------------------------------------
	// Selection support
	void SelectInstance(CSTEPView* pSender, CSTEPInstance* pInstance);

	// --------------------------------------------------------------------------------------------
	// Selection support
	CSTEPInstance* GetSelectedInstance() const;

	// --------------------------------------------------------------------------------------------
	// UI properties
	int GetVisibleValuesCountLimit() const;

	// --------------------------------------------------------------------------------------------
	// UI properties
	void SetVisibleValuesCountLimit(int iVisibleValuesCountLimit);

	// --------------------------------------------------------------------------------------------
	// UI properties
	BOOL GetScaleAndCenter() const;

	// --------------------------------------------------------------------------------------------
	// UI properties
	void SetScaleAndCenter(BOOL bScaleAndCenter);	

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	void OnEnableAllInstances(CSTEPView* pSender);
};




