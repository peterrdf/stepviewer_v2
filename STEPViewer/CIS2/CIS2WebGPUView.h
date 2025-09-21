#ifndef _WEBGPU_CIS2_VIEW_H_
#define _WEBGPU_CIS2_VIEW_H_

#include "CIS2Representation.h"
#include "_webgpuUtils.h"

// ************************************************************************************************
// WebGPU View
class CCIS2WebGPUView : public _webgpuView
{

public: // Methods
	
	// ctor/dtor
	CCIS2WebGPUView(CWnd * pWnd);
	virtual ~CCIS2WebGPUView();

protected: // Methods

	// _webgpuRenderer
	virtual void _reset() override;
};

#endif // _WEBGPU_CIS2_VIEW_H_