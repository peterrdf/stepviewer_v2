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
	map<SdaiInstance, _geometry*> m_mapGeometries;

public: // Methods

	_ap_model(enumAP enAP)
		: _model()
		, m_sdaiModel(0)
		, m_enAP(enAP)
		, m_pEntityProvider(nullptr)
		, m_mapExpressID2Geometry()
		, m_mapGeometries()
	{}

	virtual ~_ap_model()
	{
		clean();
	}

	// _model
	virtual OwlModel getOwlInstance() const override
	{
		assert(m_sdaiModel != 0);

		OwlModel owlModel = 0;
		owlGetModel(m_sdaiModel, &owlModel);
		assert(owlModel != 0);

		return owlModel;
	}

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

	_geometry* getGeometryByInstance(SdaiInstance sdaiInstance)
	{
		auto itGeometry = m_mapGeometries.find(sdaiInstance);
		if (itGeometry != m_mapGeometries.end())
		{
			return itGeometry->second;
		}

		return nullptr;
	}

	_geometry* getGeometryByExpressID(ExpressID iExpressID) const
	{
		assert(iExpressID != 0);

		auto itExpressID2Geometry = m_mapExpressID2Geometry.find(iExpressID);
		if (itExpressID2Geometry != m_mapExpressID2Geometry.end())
		{
			return itExpressID2Geometry->second;
		}

		return nullptr;
	}

	template<typename T>
	T* getGeometryByExpressIDAs(ExpressID iExpressID) const
	{
		assert(iExpressID != 0);

		auto itExpressID2Geometry = m_mapExpressID2Geometry.find(iExpressID);
		if (itExpressID2Geometry != m_mapExpressID2Geometry.end())
		{
			return dynamic_cast<T*>(itExpressID2Geometry->second);
		}

		return nullptr;
	}

	void getInstancesByType(const wchar_t* szType, vector<_ap_instance*>& vecInstances)
	{
		vecInstances.clear();

		CString strTargetType = szType;
		strTargetType.MakeUpper();

		for (auto pInstance : m_vecInstances)
		{
			auto pAPInstance = dynamic_cast<_ap_instance*>(pInstance);
			ASSERT(pAPInstance != nullptr);

			CString strType = pAPInstance->getEntityName();
			strType.MakeUpper();

			if (strType == strTargetType)
			{
				vecInstances.push_back(pAPInstance);
			}
		}
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
					getOwlInstance(),
					m_dVertexBuffersOffsetX,
					m_dVertexBuffersOffsetY,
					m_dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
				ClearedExternalBuffers(getOwlInstance());

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

		m_mapGeometries.clear();
		m_mapExpressID2Geometry.clear();

		delete m_pEntityProvider;
		m_pEntityProvider = nullptr;
	}

public: // Properties

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
