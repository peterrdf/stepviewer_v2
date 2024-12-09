#pragma once

#include "_mvc.h"
#include "_settings_storage.h"

#include "Entity.h"

#include <string>
using namespace std;

// ************************************************************************************************
enum class enumAP : int
{
	Unknown = -1,
	STEP = 0,
	IFC = 1,
	CIS2 = 2,
};

// ************************************************************************************************
class _ap_model : public _model
{

protected: // Members

	// Model
	SdaiModel m_sdaiModel;
	enumAP m_enAP;

	// Cache
	map<ExpressID, _geometry*> m_mapExpressID2Geometry;

	// Helpers
	CEntityProvider* m_pEntityProvider;

public: // Methods

	_ap_model(enumAP enAP)
		: _model()
		, m_sdaiModel(0)
		, m_enAP(enAP)
		, m_mapExpressID2Geometry()
		, m_pEntityProvider(nullptr)
	{}

	virtual ~_ap_model()
	{
		clean();
	}

protected: // Methods

	void preLoadInstance(SdaiInstance sdaiInstance)
	{
		if (m_bUpdteVertexBuffers)
		{
			m_dVertexBuffersOffsetX = 0.;
			m_dVertexBuffersOffsetY = 0.;
			m_dVertexBuffersOffsetZ = 0.;

			_vector3d vecOriginalBBMin;
			_vector3d vecOriginalBBMax;
			if (GetInstanceGeometryClass(sdaiInstance) &&
				GetBoundingBox(
					sdaiInstance,
					(double*)&vecOriginalBBMin,
					(double*)&vecOriginalBBMax))
			{
				m_dVertexBuffersOffsetX = -(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.;
				m_dVertexBuffersOffsetY = -(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.;
				m_dVertexBuffersOffsetZ = -(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.;

				TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
					m_dVertexBuffersOffsetX,
					m_dVertexBuffersOffsetY,
					m_dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
				SetVertexBufferOffset(
					m_iModel,
					m_dVertexBuffersOffsetX,
					m_dVertexBuffersOffsetY,
					m_dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
				ClearedExternalBuffers(m_iModel);

				m_bUpdteVertexBuffers = false;
			}
		} // if (m_bUpdteVertexBuffers)
	}

public: // Methods

	CEntityProvider* getEntityProvider()
	{
		if ((m_pEntityProvider == nullptr) && (m_sdaiModel != 0))
		{
			m_pEntityProvider = new CEntityProvider(m_sdaiModel);
		}

		return m_pEntityProvider;
	}

	virtual void clean() override
	{
		_model::clean();

		if (m_sdaiModel != 0)
		{
			sdaiCloseModel(m_sdaiModel);
			m_sdaiModel = 0;
		}

		m_enAP = enumAP::Unknown;

		m_mapExpressID2Geometry.clear();

		delete m_pEntityProvider;
		m_pEntityProvider = nullptr;
	}

public: // Properties

	enumAP getAP() const { return m_enAP; }
	const map<ExpressID, _geometry*>& getExpressID2Geometry() const { return m_mapExpressID2Geometry; }
};

// ************************************************************************************************
class _ap_controller
{

protected: // Members

	_ap_model* m_pModel;

public: // Methods

	_ap_controller()
		: m_pModel(nullptr)
	{}

	virtual ~_ap_controller()
	{
	}

	_model* getModel() const { return m_pModel; }
};

