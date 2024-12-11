#pragma once

#include "_mvc.h"
#include "_ap_instance.h"

#include "Entity.h"

#include <string>
using namespace std;

// ************************************************************************************************
// Application Protocol
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

private: // Members

	// Model
	SdaiModel m_sdaiModel;
	enumAP m_enAP;

	// Helpers
	CEntityProvider* m_pEntityProvider;

protected: // Members

	// Cache
	map<ExpressID, _geometry*> m_mapExpressID2Geometry;
	map<ExpressID, _ap_instance*> m_mapExpressID2Instance;

public: // Methods

	_ap_model(enumAP enAP)
		: _model()
		, m_sdaiModel(0)
		, m_enAP(enAP)
		, m_pEntityProvider(nullptr)
		, m_mapExpressID2Geometry()
		, m_mapExpressID2Instance()
	{}

	virtual ~_ap_model()
	{
		clean();
	}

public: // Methods

	bool openModel(const wchar_t* szPath)
	{
		assert((szPath != nullptr) && (wcslen(szPath) > 0));

		SdaiModel sdaiModel = sdaiOpenModelBNUnicode(0, szPath, L"");
		if (m_sdaiModel == 0)
		{
			return false;
		}

		attachModel(szPath, sdaiModel);

		return true;
	}

	void attachModel(const wchar_t* szPath, SdaiModel sdaiModel)
	{
		assert((szPath != nullptr) && (wcslen(szPath) > 0));
		assert(sdaiModel != 0);

		clean();

		m_strPath = szPath;
		m_sdaiModel = sdaiModel;

		attachModelCore();
	}

	_ap_instance* getInstanceByExpressID(int64_t iExpressID) const
	{
		assert(iExpressID != 0);

		auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
		if (itExpressID2Instance != m_mapExpressID2Instance.end())
		{
			return itExpressID2Instance->second;
		}

		return nullptr;
	}

	template<typename T>
	T* getInstanceByExpressIDAs(int64_t iExpressID) const
	{
		assert(iExpressID != 0);

		auto itExpressID2Instance = m_mapExpressID2Instance.find(iExpressID);
		if (itExpressID2Instance != m_mapExpressID2Instance.end())
		{
			return dynamic_cast<T*>(itExpressID2Instance->second);
		}

		return nullptr;
	}

protected: // Methods

	virtual void attachModelCore() PURE;

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
					getOwlModel(),
					m_dVertexBuffersOffsetX,
					m_dVertexBuffersOffsetY,
					m_dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
				ClearedExternalBuffers(getOwlModel());

				m_bUpdteVertexBuffers = false;
			}
		} // if (m_bUpdteVertexBuffers)
	}

	virtual void clean() override
	{
		_model::clean();

		m_strPath = L"";

		if (m_sdaiModel != 0)
		{
			sdaiCloseModel(m_sdaiModel);
			m_sdaiModel = 0;
		}

		m_mapExpressID2Geometry.clear();
		m_mapExpressID2Instance.clear();

		delete m_pEntityProvider;
		m_pEntityProvider = nullptr;
	}

public: // Properties

	// _model
	virtual OwlModel getOwlModel() const override
	{
		assert(m_sdaiModel != 0);

		OwlModel owlModel = 0;
		owlGetModel(m_sdaiModel, &owlModel);
		assert(owlModel != 0);

		return owlModel;
	}

	// Model
	SdaiModel getSdaiInstance() const { return m_sdaiModel; }
	enumAP getAP() const { return m_enAP; }

	// Cache	
	const map<ExpressID, _geometry*>& getExpressID2Geometry() const { return m_mapExpressID2Geometry; }

	// Helpers
	CEntityProvider* getEntityProvider()
	{
		if ((m_pEntityProvider == nullptr) && (m_sdaiModel != 0))
		{
			m_pEntityProvider = new CEntityProvider(m_sdaiModel);
		}

		return m_pEntityProvider;
	}
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

