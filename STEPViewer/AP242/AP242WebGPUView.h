#ifndef _WEBGPU_STEP_VIEW_H_
#define _WEBGPU_STEP_VIEW_H_

#include "_webgpuUtils.h"

// ************************************************************************************************
class CAP242WebGPUView : public _webgpuView
{

public: // Methods

	CAP242WebGPUView(CWnd* pWnd);
	virtual ~CAP242WebGPUView();

protected: // Methods

	// _webgpuRenderer
	virtual void _reset() override;
};

#endif // _WEBGPU_STEP_VIEW_H_