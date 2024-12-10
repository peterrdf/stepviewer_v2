#pragma once

#include "_geometry.h"

class _ap_geometry : public _geometry
{

private: // Members

	SdaiModel m_sdaiInstance;

public: // Methods

	_ap_geometry(SdaiInstance sdaiInstance)
		: _geometry(-1, 0, true)
		, m_sdaiInstance(sdaiInstance)
	{
		assert(m_sdaiInstance != 0);

		owlBuildInstance(getSdaiModel(), m_sdaiInstance, &m_iInstance);
	}

	virtual ~_ap_geometry()
	{}

	// _geometry
	virtual OwlModel getModel() const
	{
		OwlModel owlModel = 0;
		owlGetModel(getSdaiModel(), &owlModel);
		assert(owlModel != 0);

		return owlModel;
	}

public: // Properties

	SdaiInstance getSdaiInstance() const { return m_sdaiInstance; }
	ExpressID getExpressID() const { return internalGetP21Line(m_sdaiInstance); }
	SdaiModel getSdaiModel() const { return sdaiGetInstanceModel(m_sdaiInstance); }
};