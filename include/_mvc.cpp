#include "stdafx.h"

#include "_mvc.h"
#include "_rdf_instance.h"

// ************************************************************************************************
_model::_model()
	: m_strPath(L"")
	, m_pWorld(nullptr)
	, m_mapID2Instance()
	, m_bUpdteVertexBuffers(true)
	, m_dOriginalBoundingSphereDiameter(2.)
	, m_fXmin(-1.f)
	, m_fXmax(1.f)
	, m_fYmin(-1.f)
	, m_fYmax(1.f)
	, m_fZmin(-1.f)
	, m_fZmax(1.f)
	, m_fBoundingSphereDiameter(2.f)
	, m_vecGeometries()
	, m_vecInstances()
{
}

/*virtual*/ _model::~_model()
{
	clean();
}

void _model::scale()
{
	// World
	m_dOriginalBoundingSphereDiameter = 2.;
	m_fBoundingSphereDiameter = 2.f;

	if (m_pWorld == nullptr)
	{
		// Min/Max
		m_fXmin = FLT_MAX;
		m_fXmax = -FLT_MAX;
		m_fYmin = FLT_MAX;
		m_fYmax = -FLT_MAX;
		m_fZmin = FLT_MAX;
		m_fZmax = -FLT_MAX;

		for (auto pGeometry : m_vecGeometries)
		{
			if (!pGeometry->hasGeometry())
			{
				continue;
			}

			for (auto pInstance : pGeometry->getInstances())
			{
				pGeometry->calculateMinMaxTransform(
					pInstance,
					m_fXmin, m_fXmax,
					m_fYmin, m_fYmax,
					m_fZmin, m_fZmax);
			}
		} // for (auto pGeometry : ...

		if ((m_fXmin == FLT_MAX) ||
			(m_fXmax == -FLT_MAX) ||
			(m_fYmin == FLT_MAX) ||
			(m_fYmax == -FLT_MAX) ||
			(m_fZmin == FLT_MAX) ||
			(m_fZmax == -FLT_MAX))
		{
			// TODO: new status bar for messages

			return;
		}

		// World
		m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
		m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
		m_fBoundingSphereDiameter = fmax(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

		m_dOriginalBoundingSphereDiameter = m_fBoundingSphereDiameter;

		TRACE(L"\n*** Scale I *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
			m_fXmin,
			m_fXmax,
			m_fYmin,
			m_fYmax,
			m_fZmin,
			m_fZmax);
		TRACE(L"\n*** Scale, Bounding sphere I *** =>  %.16f", m_fBoundingSphereDiameter);
	} // if (m_pWorld == nullptr)
	else
	{
		m_dOriginalBoundingSphereDiameter = m_pWorld->getOriginalBoundingSphereDiameter();
	}

	// Scale
	for (auto pGeometry : m_vecGeometries)
	{
		if (!pGeometry->hasGeometry())
		{
			continue;
		}

		pGeometry->scale((float)m_dOriginalBoundingSphereDiameter / 2.f);
	}

	// Min/Max
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	for (auto pGeometry : m_vecGeometries)
	{
		if (!pGeometry->hasGeometry())
		{
			continue;
		}

		for (auto pInstance : pGeometry->getInstances())
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			pGeometry->calculateMinMaxTransform(
				pInstance,
				m_fXmin, m_fXmax,
				m_fYmin, m_fYmax,
				m_fZmin, m_fZmax);
		}
	} // for (auto pGeometry : ...

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		// TODO: new status bar for messages

		return;
	}

	// World
	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);

	TRACE(L"\n*** Scale II *** => Xmin/max, Ymin/max, Zmin/max: %.16f, %.16f, %.16f, %.16f, %.16f, %.16f",
		m_fXmin,
		m_fXmax,
		m_fYmin,
		m_fYmax,
		m_fZmin,
		m_fZmax);
	TRACE(L"\n*** Scale, Bounding sphere II *** =>  %.16f", m_fBoundingSphereDiameter);
}

/*virtual*/ void _model::zoomTo(_instance* pInstance)
{
	assert(pInstance != nullptr);
	assert(pInstance->getGeometry() != nullptr);

	// World
	m_fBoundingSphereDiameter = 2.f;

	// Min/Max
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	pInstance->getGeometry()->calculateMinMaxTransform(
		pInstance,
		m_fXmin, m_fXmax,
		m_fYmin, m_fYmax,
		m_fZmin, m_fZmax);

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.f;
		m_fXmax = 1.f;
		m_fYmin = -1.f;
		m_fYmax = 1.f;
		m_fZmin = -1.f;
		m_fZmax = 1.f;
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
}

/*virtual*/ void _model::zoomOut()
{
	// World
	m_fBoundingSphereDiameter = 2.f;

	// Min/Max
	m_fXmin = FLT_MAX;
	m_fXmax = -FLT_MAX;
	m_fYmin = FLT_MAX;
	m_fYmax = -FLT_MAX;
	m_fZmin = FLT_MAX;
	m_fZmax = -FLT_MAX;

	for (auto pGeometry : m_vecGeometries)
	{
		for (auto pInstance : pGeometry->getInstances())
		{
			if (!pInstance->getEnable())
			{
				continue;
			}

			pGeometry->calculateMinMaxTransform(
				pInstance,
				m_fXmin, m_fXmax,
				m_fYmin, m_fYmax,
				m_fZmin, m_fZmax);
		}
	}

	if ((m_fXmin == FLT_MAX) ||
		(m_fXmax == -FLT_MAX) ||
		(m_fYmin == FLT_MAX) ||
		(m_fYmax == -FLT_MAX) ||
		(m_fZmin == FLT_MAX) ||
		(m_fZmax == -FLT_MAX))
	{
		m_fXmin = -1.f;
		m_fXmax = 1.f;
		m_fYmin = -1.f;
		m_fYmax = 1.f;
		m_fZmin = -1.f;
		m_fZmax = 1.f;
	}

	m_fBoundingSphereDiameter = m_fXmax - m_fXmin;
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fYmax - m_fYmin);
	m_fBoundingSphereDiameter = max(m_fBoundingSphereDiameter, m_fZmax - m_fZmin);
}

void _model::getDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
{
	fXmin = m_fXmin;
	fXmax = m_fXmax;
	fYmin = m_fYmin;
	fYmax = m_fYmax;
	fZmin = m_fZmin;
	fZmax = m_fZmax;
}

_instance* _model::getInstanceByID(int64_t iID) const
{
	auto itInstance = m_mapID2Instance.find(iID);
	if (itInstance == m_mapID2Instance.end())
	{
		return nullptr;
	}

	return itInstance->second;
}

void _model::setVertexBufferOffset(OwlInstance owlInstance)
{
	if (owlInstance == 0)
	{
		return;
	}

	if (m_bUpdteVertexBuffers)
	{
		if (m_pWorld == nullptr)
		{
			_vector3d vecOriginalBBMin;
			_vector3d vecOriginalBBMax;
			if (GetInstanceGeometryClass(owlInstance) &&
				GetBoundingBox(
					owlInstance,
					(double*)&vecOriginalBBMin,
					(double*)&vecOriginalBBMax))
			{
				double dVertexBuffersOffsetX = -(vecOriginalBBMin.x + vecOriginalBBMax.x) / 2.;
				double dVertexBuffersOffsetY = -(vecOriginalBBMin.y + vecOriginalBBMax.y) / 2.;
				double dVertexBuffersOffsetZ = -(vecOriginalBBMin.z + vecOriginalBBMax.z) / 2.;

				TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
					dVertexBuffersOffsetX,
					dVertexBuffersOffsetY,
					dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
				SetVertexBufferOffset(
					getOwlModel(),
					dVertexBuffersOffsetX,
					dVertexBuffersOffsetY,
					dVertexBuffersOffsetZ);

				// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
				ClearedExternalBuffers(getOwlModel());

				m_bUpdteVertexBuffers = false;
			}
		} // if (m_pWorld == nullptr)
		else
		{
			_vector3d vecVertexBufferOffset;
			GetVertexBufferOffset(m_pWorld->getOwlModel(), (double*)&vecVertexBufferOffset);

			double dVertexBuffersOffsetX = vecVertexBufferOffset.x;
			double dVertexBuffersOffsetY = vecVertexBufferOffset.y;
			double dVertexBuffersOffsetZ = vecVertexBufferOffset.z;

			TRACE(L"\n*** SetVertexBufferOffset *** => x/y/z: %.16f, %.16f, %.16f",
				dVertexBuffersOffsetX,
				dVertexBuffersOffsetY,
				dVertexBuffersOffsetZ);

			// http://rdf.bg/gkdoc/CP64/SetVertexBufferOffset.html
			SetVertexBufferOffset(
				getOwlModel(),
				dVertexBuffersOffsetX,
				dVertexBuffersOffsetY,
				dVertexBuffersOffsetZ);

			// http://rdf.bg/gkdoc/CP64/ClearedExternalBuffers.html
			ClearedExternalBuffers(getOwlModel());

			m_bUpdteVertexBuffers = false;
		} // else if (m_pWorld == nullptr)		
	} // if (m_bUpdteVertexBuffers)
}

void _model::addGeometry(_geometry* pGeometry)
{
	assert(pGeometry != nullptr);

	m_vecGeometries.push_back(pGeometry);
}

void _model::addInstance(_instance* pInstance)
{
	assert(pInstance != nullptr);
	assert(pInstance->getGeometry() != nullptr);

	pInstance->getGeometry()->addInstance(pInstance);

	m_vecInstances.push_back(pInstance);

	ASSERT(m_mapID2Instance.find(pInstance->getID()) == m_mapID2Instance.end());
	m_mapID2Instance[pInstance->getID()] = pInstance;
}

/*virtual*/ void _model::clean()
{
	for (auto pGeometry : m_vecGeometries)
	{
		delete pGeometry;
	}
	m_vecGeometries.clear();

	for (auto pInstance : m_vecInstances)
	{
		delete pInstance;
	}
	m_vecInstances.clear();
	m_mapID2Instance.clear();
}

// ************************************************************************************************
_controller::_controller()
	: m_vecModels()
	, m_setViews()
	, m_pSettingsStorage(new _settings_storage())
	, m_bUpdatingModel(false)
	, m_pTargetInstance(nullptr)
	, m_pSelectedInstance(nullptr)
{
}

/*virtual*/ _controller::~_controller()
{
	clean();

	delete m_pSettingsStorage;
}

void _controller::getWorldDimensions(float& fWorldXmin, float& fWorldXmax, float& fWorldYmin, float& fWorldYmax, float& fWorldZmin, float& fWorldZmax) const
{
	fWorldXmin = FLT_MAX;
	fWorldXmax = -FLT_MAX;
	fWorldYmin = FLT_MAX;
	fWorldYmax = -FLT_MAX;
	fWorldZmin = FLT_MAX;
	fWorldZmax = -FLT_MAX;
	for (auto pModel : getModels())
	{
		float fXmin = FLT_MAX;
		float fXmax = -FLT_MAX;
		float fYmin = FLT_MAX;
		float fYmax = -FLT_MAX;
		float fZmin = FLT_MAX;
		float fZmax = -FLT_MAX;
		pModel->getDimensions(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

		fWorldXmin = (float)fmin(fWorldXmin, fXmin);
		fWorldXmax = (float)fmax(fWorldXmax, fXmax);
		fWorldYmin = (float)fmin(fWorldYmin, fYmin);
		fWorldYmax = (float)fmax(fWorldYmax, fYmax);
		fWorldZmin = (float)fmin(fWorldZmin, fZmin);
		fWorldZmax = (float)fmax(fWorldZmax, fZmax);
	}
}

float _controller::getWorldBoundingSphereDiameter() const
{
	float fWorldXmin = FLT_MAX;
	float fWorldXmax = -FLT_MAX;
	float fWorldYmin = FLT_MAX;
	float fWorldYmax = -FLT_MAX;
	float fWorldZmin = FLT_MAX;
	float fWorldZmax = -FLT_MAX;
	getWorldDimensions(fWorldXmin, fWorldXmax, fWorldYmin, fWorldYmax, fWorldZmin, fWorldZmax);

	float fWorldBoundingSphereDiameter = fWorldXmax - fWorldXmin;
	fWorldBoundingSphereDiameter = fmax(fWorldBoundingSphereDiameter, fWorldYmax - fWorldYmin);
	fWorldBoundingSphereDiameter = fmax(fWorldBoundingSphereDiameter, fWorldZmax - fWorldZmin);

	return fWorldBoundingSphereDiameter;
}

_model* _controller::getModelByInstance(OwlModel owlModel) const
{
	assert(owlModel != 0);

	auto itModel = find_if(m_vecModels.begin(), m_vecModels.end(), [&](_model* pModel)
		{
			return pModel->getOwlModel() == owlModel;
		});

	if (itModel != m_vecModels.end())
	{
		return *itModel;
	}

	return nullptr;
}

_instance* _controller::getInstanceByID(int64_t iID) const
{
	for (auto pModel : m_vecModels)
	{
		auto pInstance = pModel->getInstanceByID(iID);
		if (pInstance != nullptr)
		{
			return pInstance;
		}
	}
	
	return nullptr;
}

void _controller::registerView(_view* pView)
{
	assert(pView != nullptr);
	assert(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

void _controller::unRegisterView(_view* pView)
{
	assert(pView != nullptr);
	assert(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

const set<_view*>& _controller::getViews()
{
	return m_setViews;
}

void _controller::zoomToSelectedInstance()
{
	assert(m_pSelectedInstance != nullptr);

	auto pModel = getModelByInstance(m_pSelectedInstance->getOwlModel());
	if (pModel == nullptr)
	{
		ASSERT(FALSE);

		return;
	}

	pModel->zoomTo(m_pSelectedInstance);

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onWorldDimensionsChanged();
	}
}

void _controller::zoomOut()
{
	getModel()->zoomOut();

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onWorldDimensionsChanged();
	}
}

void _controller::saveInstance(OwlInstance owlInstance)
{
	assert(owlInstance != 0);

	wstring strName;
	wstring strUniqueName;
	_rdf_instance::buildInstanceNames(GetModel(owlInstance), owlInstance, strName, strUniqueName);

	CString strValidPath = strUniqueName.c_str();
	strValidPath.Replace(_T("\\"), _T("-"));
	strValidPath.Replace(_T("/"), _T("-"));
	strValidPath.Replace(_T(":"), _T("-"));
	strValidPath.Replace(_T("*"), _T("-"));
	strValidPath.Replace(_T("?"), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("\""), _T("-"));
	strValidPath.Replace(_T("<"), _T("-"));
	strValidPath.Replace(_T(">"), _T("-"));
	strValidPath.Replace(_T("|"), _T("-"));

	TCHAR szFilters[] = _T("BIN Files (*.bin)|*.bin|All Files (*.*)|*.*||");
	CFileDialog dlgFile(FALSE, _T("bin"), strValidPath,
		OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY, szFilters);

	if (dlgFile.DoModal() != IDOK)
	{
		return;
	}

	SaveInstanceTreeW(owlInstance, dlgFile.GetPathName());
}

void _controller::setTargetInstance(_view* pSender, _instance* pInstance)
{
	if (m_bUpdatingModel)
	{
		return;
	}

	if (m_pTargetInstance == pInstance)
	{
		return;
	}

	m_pTargetInstance = pInstance;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onTargetInstanceChanged(pSender);
	}
}

_instance* _controller::getTargetInstance() const
{
	return m_pTargetInstance;
}

void _controller::selectInstance(_view* pSender, _instance* pInstance)
{
	if (m_bUpdatingModel)
	{
		return;
	}

	m_pSelectedInstance = pInstance;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onInstanceSelected(pSender);
	}
}

_instance* _controller::getSelectedInstance() const
{
	return m_pSelectedInstance;
}

void _controller::onInstancesEnabledStateChanged(_view* pSender)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onInstancesEnabledStateChanged(pSender);
	}
}

void _controller::onApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onApplicationPropertyChanged(pSender, enApplicationProperty);
	}
}

void _controller::onViewRelations(_view* pSender, SdaiInstance sdaiInstance)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onViewRelations(pSender, sdaiInstance);
	}
}

void _controller::onViewRelations(_view* pSender, _entity* pEntity)
{
	m_pTargetInstance = nullptr;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onViewRelations(pSender, pEntity);
	}
}

void _controller::onInstanceAttributeEdited(_view* pSender, SdaiInstance sdaiInstance, SdaiAttr sdaiAttr)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onInstanceAttributeEdited(pSender, sdaiInstance, sdaiAttr);
	}
}

/*virtual*/ void _controller::clean()
{
	for (auto pModel : m_vecModels)
	{
		delete pModel;
	}
	m_vecModels.clear();

	s_iInstanceID = 1;
}

_model* _controller::getModel() const 
{ 
	if (!m_vecModels.empty())
	{
		return m_vecModels.front();
	}

	return nullptr; 
}

void _controller::setModel(_model* pModel)
{
	clean();

	if (pModel != nullptr)
	{
		m_vecModels.push_back(pModel);
	}

	m_pSelectedInstance = nullptr;

	m_bUpdatingModel = true;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onModelLoaded();
	}

	m_bUpdatingModel = false;
}

void _controller::addModel(_model* pModel)
{
	assert(pModel != nullptr);

	m_vecModels.push_back(pModel);

	m_bUpdatingModel = true;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->onModelLoaded();
	}

	m_bUpdatingModel = false;
}