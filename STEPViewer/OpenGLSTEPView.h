#ifndef _OPEN_GL_STEP_VIEW_H_
#define _OPEN_GL_STEP_VIEW_H_

#include "_oglUtils.h"
#include "OpenGLView.h"
#include "ProductDefinition.h"

// ------------------------------------------------------------------------------------------------
// Open GL View
class COpenGLSTEPView
	: public COpenGLView
	, public _oglRenderer
{

private: // Members	

	// Mouse
	CPoint m_ptStartMousePosition;
	CPoint m_ptPrevMousePosition;

	// Selection
	_oglSelectionFramebuffer* m_pInstanceSelectionFrameBuffer;
	CProductInstance* m_pPointedInstance;
	CProductInstance* m_pSelectedInstance;

	// Materials
	_material* m_pSelectedInstanceMaterial;
	_material* m_pPointedInstanceMaterial;

public: // Methods

	// ctor/dtor
	COpenGLSTEPView(CWnd* pWnd);
	virtual ~COpenGLSTEPView();	

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
	virtual void Load() override;

	// COpenGLView
	virtual void SetProjection(enumProjection enProjection) override;
	virtual enumProjection GetProjection() const override;
	virtual void SetView(enumView enView) override;
	virtual enumRotationMode GetRotationMode() const override;
	virtual void SetRotationMode(enumRotationMode enRotationMode) override;
	virtual void OnMouseEvent(enumMouseEvent enEvent, UINT nFlags, CPoint point) override;
	virtual void OnMouseWheel(UINT nFlags, short zDelta, CPoint pt) override;
	virtual void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags) override;
	virtual void Draw(CDC* pDC) override;

private: // Methods
	
	// UI
	void DrawFaces(_model* pM, bool bTransparent);
	void DrawConceptualFacesPolygons(_model* pM);
	void DrawLines(_model* pM);
	void DrawPoints(_model* pM);

	// Selection
	void DrawInstancesFrameBuffer();

	// Mouse
	void OnMouseMoveEvent(UINT nFlags, CPoint point);
};

#endif // _OPEN_GL_STEP_VIEW_H_
