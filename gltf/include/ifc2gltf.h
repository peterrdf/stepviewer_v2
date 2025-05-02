#pragma once

#include "engine.h"

// ************************************************************************************************
#ifdef _WINDOWS
	#define STDCALL __stdcall

	#ifndef IFC2GLTF_STATIC
		#ifdef IFC2GLTF_EXPORTS
			#define DECSPEC __declspec(dllexport)
		#else
			#define DECSPEC __declspec(dllimport)		
		#endif
	#else
		#define DECSPEC /*nothing*/
	#endif
#else
	#define STDCALL /*nothing*/
#endif // _WINDOWS

// ************************************************************************************************
/* C interface */
#ifdef __cplusplus
extern "C" {
#endif
	void DECSPEC STDCALL SetIFC2GLTFOptions(void* pLogCallback);

	void DECSPEC STDCALL SaveAsGLTF(const char* szInputFile, const char* szOutputFile, bool bEmbeddedBuffers);
#ifdef __cplusplus
};
#endif



