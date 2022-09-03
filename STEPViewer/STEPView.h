#pragma once

#include "ProductInstance.h"

// ------------------------------------------------------------------------------------------------
class CSTEPController;

// ------------------------------------------------------------------------------------------------
// View - MVC
class CSTEPView
{

private: // Members

	// --------------------------------------------------------------------------------------------
	// Controller - MVC
	CSTEPController* m_pController;

public: // Methods

	// --------------------------------------------------------------------------------------------
	// ctor
	CSTEPView();

	// --------------------------------------------------------------------------------------------
	// dtor
	virtual ~CSTEPView();

	// --------------------------------------------------------------------------------------------
	// Sets up Controller
	void SetController(CSTEPController* pController);

	// --------------------------------------------------------------------------------------------
	// A Model has been loaded
	virtual void OnModelChanged();

	// --------------------------------------------------------------------------------------------
	// The Model has been recalculated
	virtual void OnWorldDimensionsChanged();

	// --------------------------------------------------------------------------------------------
	// Meta information support
	virtual void OnShowMetaInformation();

	// --------------------------------------------------------------------------------------------
	// An Instance has been selected
	virtual void OnInstanceSelected(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// A Property has been selected
	virtual void OnInstancePropertySelected();

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	virtual void OnInstancesEnabledStateChanged(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	virtual void OnInstanceEnabledStateChanged(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	virtual void OnDisableAllButThis(CSTEPView* pSender, CProductInstance* pProductInstance);

	// --------------------------------------------------------------------------------------------
	// Enable/Disable support
	virtual void OnEnableAllInstances(CSTEPView* pSender);

	// --------------------------------------------------------------------------------------------
	// UI properties support
	virtual void OnVisibleValuesCountLimitChanged();

protected: // Methods

	// --------------------------------------------------------------------------------------------
	// Getter
	CSTEPController* GetController() const;

	// --------------------------------------------------------------------------------------------
	// The controller has been changed
	virtual void OnControllerChanged();
};

