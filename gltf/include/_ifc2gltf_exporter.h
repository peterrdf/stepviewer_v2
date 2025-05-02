#pragma once

#ifdef _WINDOWS
#include "ifcengine.h"
#else
#include "../../include/ifcengine.h"
#endif

#include "_ap_mvc.h"
#include "_ifc_model.h"

#include "_bin2gltf_exporter.h"
//#include "AP242.h"#todo

// ************************************************************************************************
namespace _ifc2gltf
{
	// ********************************************************************************************
	class _gltf_ifc_model;

	// ********************************************************************************************
	class _gltf_ifc_model : public _ifc_model
	{

	public: // Methods

		_gltf_ifc_model();
		virtual ~_gltf_ifc_model();

	protected: // Methods

		// _ifc_model
		virtual _ifc_geometry* createGeometry(OwlInstance owlInstance, SdaiInstance sdaiInstance) override;
	};

	// ********************************************************************************************
	class _ifc_node
		: public _node
		, public _ifc_geometry
	{

	public: // Methods

		_ifc_node(OwlInstance owlInstance, SdaiInstance sdaiInstance, const vector<_ifc_geometry*>& vecMappedGeometries);
		virtual ~_ifc_node();

	protected: // Methods

		// _ap_geometry
		virtual void cleanCachedGeometry() override {}
	};

	// ********************************************************************************************
	class _exporter : public _bin2gltf::_exporter
	{

	private: // Members

		_ap_model* m_pModel;
		SdaiModel m_sdaiModel;//#todo
		//_AP242* m_pAP242;//#todo

	public: // Methods

		_exporter(const char* szInputFile, const char* szOutputFile, bool bEmbeddedBuffers);
		virtual ~_exporter();

	protected: // Methods

		virtual bool preExecute() override;
		virtual void postExecute() override;

		virtual void GetInstancesDefaultState() override;
	};
};

