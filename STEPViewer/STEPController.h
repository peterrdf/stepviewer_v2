#pragma once

#include "STEPInstance.h"
#include "Entity.h"
#include "ProductInstance.h"

#include <set>
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CSTEPModelBase;
class CSTEPView;
enum class enumApplicationProperty;

// ------------------------------------------------------------------------------------------------
// Controller - MVC
class CSTEPController
{

private: // Members	
	
	CSTEPModelBase* m_pModel; // Model - MVC
	bool m_bUpdatingModel; // Updating model - disable all notifications	
	
	set<CSTEPView*> m_setViews; // Views - MVC	
	
	// Selection
	CSTEPInstance* m_pSelectedInstance;
	
	// UI properties
	int m_iVisibleValuesCountLimit;
	BOOL m_bScaleAndCenter;

public: // Methods
	
	CSTEPController();	
	virtual ~CSTEPController();
	
	CSTEPModelBase* GetModel() const;
	void SetModel(CSTEPModelBase* pModel);

	// Events
	void RegisterView(CSTEPView* pView);
	void UnRegisterView(CSTEPView* pView);
	
	const set<CSTEPView*> & GetViews();
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
	
	// [-1, 1]
	void ScaleAndCenter();
	
	// Events
	void ShowMetaInformation(CProductInstance* pInstance);
	void SelectInstance(CSTEPView* pSender, CSTEPInstance* pInstance);
	CSTEPInstance* GetSelectedInstance() const;

	// UI 
	int GetVisibleValuesCountLimit() const;
	void SetVisibleValuesCountLimit(int iVisibleValuesCountLimit);
	BOOL GetScaleAndCenter() const;
	void SetScaleAndCenter(BOOL bScaleAndCenter);
	
	// Events
	void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	void OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty);
	void OnViewRelations(CSTEPView* pSender, CSTEPInstance* pInstance);
	void OnViewRelations(CSTEPView* pSender, CEntity* pEntity);
};




