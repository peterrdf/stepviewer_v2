#include "pch.h"
#include "ifc2gltf.h"

#include "_log.h"
#include "_ifc2gltf_exporter.h"

// ************************************************************************************************
static _log_callback g_pLogCallback = nullptr;

// ************************************************************************************************
#ifdef __cplusplus
extern "C" {
#endif
	void DECSPEC STDCALL SetIFC2GLTFOptions(void* pLogCallback)
	{
		g_pLogCallback = (_log_callback)pLogCallback;
	}

	void DECSPEC STDCALL SaveAsGLTF(const char* szInputFile, const char* szOutputFile, bool bEmbeddedBuffers)
	{
		_c_log log(g_pLogCallback);

		_ifc2gltf::_exporter exporter(szInputFile, szOutputFile, bEmbeddedBuffers);
		exporter.setLog(&log);
		exporter.execute();
	}
#ifdef __cplusplus
};
#endif