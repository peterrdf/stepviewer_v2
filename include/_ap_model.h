#pragma once

#include "_ptr.h"
#include "_mvc.h"
#include "_attribute.h"
#include "_ap_instance.h"

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
	_entity_provider* m_pEntityProvider;
	_attribute_provider* m_pAttributeProvider;

	// Cache
	map<ExpressID, _geometry*> m_mapExpressID2Geometry;
	map<SdaiInstance, _geometry*> m_mapGeometries;

public: // Methods

	_ap_model(enumAP enAP)
		: _model()
		, m_sdaiModel(0)
		, m_enAP(enAP)
		, m_pEntityProvider(nullptr)
		, m_pAttributeProvider(nullptr)
		, m_mapExpressID2Geometry()
		, m_mapGeometries()
	{}

	virtual ~_ap_model()
	{
		clean();
	}

	// _model
	virtual OwlModel getOwlModel() const override
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
		if (sdaiModel == 0)
		{
			return false;
		}

		attachModel(szPath, sdaiModel, nullptr);

		return true;
	}

	void attachModel(const wchar_t* szPath, SdaiModel sdaiModel, _model* pWorld)
	{
		assert((szPath != nullptr) && (wcslen(szPath) > 0));
		assert(sdaiModel != 0);

		clean();

		m_strPath = szPath;
		m_sdaiModel = sdaiModel;
		m_pWorld = pWorld;

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

	void getGeometriesByType(const char* szType, vector<_ap_geometry*>& vecGeometries)
	{
		assert((szType != nullptr) && (strlen(szType) > 0));

		vecGeometries.clear();

		SdaiAggr sdaiGroupAggr = sdaiGetEntityExtentBN(getSdaiModel(), szType);
		ASSERT(sdaiGroupAggr != nullptr);

		SdaiInteger iInstancesCount = sdaiGetMemberCount(sdaiGroupAggr);
		for (SdaiInteger i = 0; i < iInstancesCount; i++)
		{
			SdaiInstance sdaiInstance = 0;
			sdaiGetAggrByIndex(sdaiGroupAggr, i, sdaiINSTANCE, &sdaiInstance);
			assert(sdaiInstance != 0);

			auto pGeometry = getGeometryByInstance(sdaiInstance);
			assert(pGeometry != nullptr);

			vecGeometries.push_back(_ptr<_ap_geometry>(pGeometry));
		}
	}

protected: // Methods

	virtual void attachModelCore() PURE;

	virtual void preLoadInstance(OwlInstance owlInstance)
	{
		setVertexBufferOffset(owlInstance);
	}

	void addGeometry(_ap_geometry* pGeometry)
	{
		_model::addGeometry(pGeometry);

		assert(m_mapGeometries.find(pGeometry->getSdaiInstance()) == m_mapGeometries.end());
		m_mapGeometries[pGeometry->getSdaiInstance()] = pGeometry;

		assert(m_mapExpressID2Geometry.find(pGeometry->getExpressID()) == m_mapExpressID2Geometry.end());
		m_mapExpressID2Geometry[pGeometry->getExpressID()] = pGeometry;
	}

	virtual void clean(bool bCloseModel = true) override
	{
		_model::clean(bCloseModel);

		if (bCloseModel)
		{
			if (m_sdaiModel != 0)
			{
				sdaiCloseModel(m_sdaiModel);
				m_sdaiModel = 0;
			}

			delete m_pEntityProvider;
			m_pEntityProvider = nullptr;

			delete m_pAttributeProvider;
			m_pAttributeProvider = nullptr;
		}

		m_mapGeometries.clear();
		m_mapExpressID2Geometry.clear();
	}

public: // Properties

	// Model
	SdaiModel getSdaiModel() const { return m_sdaiModel; }
	enumAP getAP() const { return m_enAP; }

	// Cache	
	const map<ExpressID, _geometry*>& getExpressID2Geometry() const { return m_mapExpressID2Geometry; }

	// Helpers
	_entity_provider* getEntityProvider()
	{
		if ((m_pEntityProvider == nullptr) && (m_sdaiModel != 0))
		{
			m_pEntityProvider = new _entity_provider(m_sdaiModel);
		}

		return m_pEntityProvider;
	}

	_attribute_provider* getAttributeProvider()
	{
		if (m_pAttributeProvider == nullptr)
		{
			m_pAttributeProvider = new _attribute_provider();
		}

		return m_pAttributeProvider;
	}
};
