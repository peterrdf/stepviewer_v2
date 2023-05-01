#pragma once

#include "Instance.h"
#include "Entity.h"
#include "ProductInstance.h"

#include <set>
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CModel;
class CViewBase;
enum class enumApplicationProperty;

// ------------------------------------------------------------------------------------------------
// Controller - MVC
class CController
{

protected: // Members

	CModel* m_pModel; // Model - MVC

private: // Members	
	
	bool m_bUpdatingModel; // Updating model - disable all notifications	
	
	set<CViewBase*> m_setViews; // Views - MVC	
	
	// Selection
	CInstance* m_pSelectedInstance;
	
	// UI properties
	BOOL m_bScaleAndCenter;

public: // Methods
	
	CController();	
	virtual ~CController();
	
	CModel* GetModel() const;
	void SetModel(CModel* pModel);

	// Events
	void RegisterView(CViewBase* pView);
	void UnRegisterView(CViewBase* pView);
	
	const set<CViewBase*> & GetViews();
	template <class T>
	T * GetView()
	{
		set<CViewBase*>::const_iterator itView = m_setViews.begin();
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
	void ShowMetaInformation(CInstance* pInstance);
	void SelectInstance(CViewBase* pSender, CInstance* pInstance);
	CInstance* GetSelectedInstance() const;

	// UI
	BOOL GetScaleAndCenter() const;
	void SetScaleAndCenter(BOOL bScaleAndCenter);
	
	// Events
	void OnInstancesEnabledStateChanged(CViewBase* pSender);
	void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty);
	void OnViewRelations(CViewBase* pSender, int64_t iInstance);
	void OnViewRelations(CViewBase* pSender, CEntity* pEntity);
};




