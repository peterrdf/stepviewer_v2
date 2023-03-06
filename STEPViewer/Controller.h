#pragma once

#include "Instance.h"
#include "Entity.h"
#include "ProductInstance.h"

#include <set>
#include <vector>

using namespace std;

// ------------------------------------------------------------------------------------------------
class CModel;
class CSTEPView;
enum class enumApplicationProperty;

// ------------------------------------------------------------------------------------------------
// Controller - MVC
class CController
{

private: // Members	
	
	CModel* m_pModel; // Model - MVC
	bool m_bUpdatingModel; // Updating model - disable all notifications	
	
	set<CSTEPView*> m_setViews; // Views - MVC	
	
	// Selection
	CInstance* m_pSelectedInstance;
	
	// UI properties
	int m_iVisibleValuesCountLimit;
	BOOL m_bScaleAndCenter;

public: // Methods
	
	CController();	
	virtual ~CController();
	
	CModel* GetModel() const;
	void SetModel(CModel* pModel);

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
	void ShowMetaInformation(CInstance* pInstance);
	void SelectInstance(CSTEPView* pSender, CInstance* pInstance);
	CInstance* GetSelectedInstance() const;

	// UI 
	int GetVisibleValuesCountLimit() const;
	void SetVisibleValuesCountLimit(int iVisibleValuesCountLimit);
	BOOL GetScaleAndCenter() const;
	void SetScaleAndCenter(BOOL bScaleAndCenter);
	
	// Events
	void OnInstancesEnabledStateChanged(CSTEPView* pSender);
	void OnApplicationPropertyChanged(CSTEPView* pSender, enumApplicationProperty enApplicationProperty);
	void OnViewRelations(CSTEPView* pSender, CInstance* pInstance);
	void OnViewRelations(CSTEPView* pSender, CEntity* pEntity);
};




