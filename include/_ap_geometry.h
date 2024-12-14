#pragma once

#include "_geometry.h"

// ************************************************************************************************
class _ap_geometry : public _geometry
{

private: // Members

	SdaiInstance m_sdaiInstance;

public: // Methods

	_ap_geometry(SdaiInstance sdaiInstance)
		: _geometry(0)
		, m_sdaiInstance(sdaiInstance)
	{
		assert(m_sdaiInstance != 0);

		owlBuildInstance(getSdaiModel(), m_sdaiInstance, &m_owlInstance);
	}

	virtual ~_ap_geometry()
	{
	}

	// _geometry
	virtual OwlModel getOwlModel() override
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