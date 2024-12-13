#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "OpenGLView.h"
#include "AP242ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
// Open GL View
class CAP242OpenGLView
	: public COpenGLView
{

public: // Methods

	// ctor/dtor
	CAP242OpenGLView(CWnd* pWnd);
	virtual ~CAP242OpenGLView();	

	// _oglRendererSettings
	virtual _controller* getController() const override;
	virtual _model* getModel() const override;
	virtual void saveSetting(const string& strName, const string& strValue) override;
	virtual string loadSetting(const string& strName) override;

	// CViewBase
	virtual void OnWorldDimensionsChanged() override;
	virtual void OnInstanceSelected(CViewBase* pSender) override;
	virtual void OnInstancesEnabledStateChanged(CViewBase* pSender) override;
	virtual void OnApplicationPropertyChanged(CViewBase* pSender, enumApplicationProperty enApplicationProperty) override;

protected: // Methods

	// CViewBase
	virtual void OnControllerChanged() override;

public: // Methods

	// COpenGLView
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) override;
};

#endif // _OPEN_GL_STEP_VIEW_H_
