#pragma once

#include "_mvc.h"
#include "InstanceBase.h"
#include "Entity.h"

#include <set>
#include <vector>
using namespace std;

// ************************************************************************************************
class CModel;
class CViewBase;
enum class enumApplicationProperty;

// ************************************************************************************************
// Controller - MVC
class CController : public _controller
{

private: // Members	
	
	bool m_bUpdatingModel; // Updating model - disable all notifications	
	
	set<CViewBase*> m_setViews; // Views - MVC	
	
	// Target
	CInstanceBase* m_pTargetInstance;

	// Selection
	CInstanceBase* m_pSelectedInstance;
	
	// UI properties
	BOOL m_bScaleAndCenter;

public: // Methods
	
	CController();	
	virtual ~CController();
	
	CModel* GetModel() const;
	void SetModel(CModel* pModel);
	CInstanceBase* LoadInstance(OwlInstance iInstance);

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
	virtual void SaveInstance() PURE;
	void SaveInstance(OwlInstance iInstance);
	
	// [-1, 1]
	void ScaleAndCenter();
	
	// Events
	void ShowMetaInformation(CInstanceBase* pInstance);
	void SetTargetInstance(CViewBase* pSender, CInstanceBase* pInstance);
	CInstanceBase* GetTargetInstance() const;
	void SelectInstance(CViewBase* pSender, CInstanceBase* pInstance);
	CInstanceBase* GetSelectedInstance() const;

	// UI
	BOOL GetScaleAndCenter() const;
	void SetScaleAndCenter(BOOL bScaleAndCenter);
	
	// Events
	void OnInstancesEnabledStateChanged(CViewBase* pSender);
	void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty);
	void OnViewRelations(CViewBase* pSender, SdaiInstance iInstance);
	void OnViewRelations(CViewBase* pSender, CEntity* pEntity);
	void OnInstanceAttributeEdited(CViewBase* pSender, SdaiInstance iInstance, SdaiAttr pAttribute);
};




