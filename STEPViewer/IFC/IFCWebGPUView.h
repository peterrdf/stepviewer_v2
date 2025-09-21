#ifndef _WEBGPU_IFC_VIEW_H_
#define _WEBGPU_IFC_VIEW_H_

#include "_webgpuUtils.h"

// ************************************************************************************************
// WebGPU View
class CIFCWebGPUView : public _webgpuView
{

public: // Methods
	
	// ctor/dtor
	CIFCWebGPUView(CWnd * pWnd);
	virtual ~CIFCWebGPUView();

protected: // Methods

	// _webgpuRenderer
	virtual void _reset() override;
};

#endif // _WEBGPU_IFC_VIEW_H_