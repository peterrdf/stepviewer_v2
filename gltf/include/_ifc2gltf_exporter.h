#pragma once

#ifdef _WINDOWS
#include "ifcengine.h"
#else
#include "../../include/ifcengine.h"
#endif

#include "_bin2gltf_exporter.h"
//#include "AP242.h"#todo

// ************************************************************************************************
namespace _ifc2gltf
{
	class _exporter : public _bin2gltf::_exporter
	{

	private: // Members

		SdaiModel m_sdaiModel;
		//_AP242* m_pAP242;//#todo

	public: // Methods

		_exporter(const char* szInputFile, const char* szOutputFile, bool bEmbeddedBuffers);
		virtual ~_exporter();

	protected: // Methods

		virtual void postExecute() override;

		virtual void GetInstancesDefaultState() override;
	};
};

