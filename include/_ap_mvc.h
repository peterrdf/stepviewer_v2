#pragma once

#include "_mvc.h"
#include "_settings_storage.h"

#include <string>
using namespace std;

// ************************************************************************************************
enum class enumAPModelType : int
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
	enumAPModelType m_enType;
	map<ExpressID, _geometry*> m_mapExpressID2Geometry;

public: // Methods

	_ap_model(enumAPModelType enType)
		: _model()
		, m_enType(enType)
		, m_mapExpressID2Geometry()
	{}

	virtual ~_ap_model()
	{
		clean();
	}

public: // Properties

	enumAPModelType getType() const { return m_enType; }

protected: // Methods

	virtual void clean() override
	{
		_model::clean();

		for (auto& itGeometry : m_mapExpressID2Geometry)
		{
			delete itGeometry.second;
		}
		m_mapExpressID2Geometry.clear();
	}

public: // Properties

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

