#include "stdafx.h"

#include "_mvc.h"
#include "_owl_instance.h"

// ************************************************************************************************
_model::_model()
	: m_strPath(L"")
	, m_vecGeometries()
	, m_vecInstances()
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

	// Scale
	for (auto pGeometry : m_vecGeometries)
	{
		if (!pGeometry->hasGeometry())
		{
			continue;
		}

		pGeometry->scale(m_fBoundingSphereDiameter / 2.f);
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

void _model::getWorldDimensions(float& fXmin, float& fXmax, float& fYmin, float& fYmax, float& fZmin, float& fZmax) const
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
	: m_pModel(nullptr)
	, m_setViews()
	, m_pSettingsStorage(new _settings_storage())
	, m_bUpdatingModel(false)
	, m_pTargetInstance(nullptr)
	, m_pSelectedInstance(nullptr)
	, m_bScaleAndCenter(FALSE)
{
}

/*virtual*/ _controller::~_controller()
{
	delete m_pSettingsStorage;
}

// ------------------------------------------------------------------------------------------------
_model* _controller::GetModel() const
{
	return getModel();
}

// ------------------------------------------------------------------------------------------------
void _controller::SetModel(_model* pModel)
{
	ASSERT(pModel != nullptr);

	m_pModel = pModel;

	m_pSelectedInstance = nullptr;

	m_bUpdatingModel = true;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnModelChanged();
	}

	m_bUpdatingModel = false;
}

_instance* _controller::LoadInstance(OwlInstance /*iInstance*/)
{
	ASSERT(FALSE); // #todo

	return nullptr;
}

// ------------------------------------------------------------------------------------------------
void _controller::RegisterView(_view* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) == m_setViews.end());

	m_setViews.insert(pView);
}

// ------------------------------------------------------------------------------------------------
void _controller::UnRegisterView(_view* pView)
{
	ASSERT(pView != nullptr);
	ASSERT(m_setViews.find(pView) != m_setViews.end());

	m_setViews.erase(pView);
}

// ------------------------------------------------------------------------------------------------
const set<_view*>& _controller::GetViews()
{
	return m_setViews;
}

// ------------------------------------------------------------------------------------------------
void _controller::ZoomToInstance()
{
	ASSERT(m_pModel != nullptr);
	ASSERT(m_pSelectedInstance != nullptr);

	GetModel()->zoomTo(m_pSelectedInstance);

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}
}

// ------------------------------------------------------------------------------------------------
void _controller::ZoomOut()
{
	ASSERT(m_pModel != nullptr);

	GetModel()->zoomOut();

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}
}

void _controller::SaveInstance(OwlInstance iInstance)
{
	ASSERT(iInstance != 0);

	wstring strName;
	wstring strUniqueName;
	_owl_instance::buildInstanceNames(m_pModel->getOwlInstance(), iInstance, strName, strUniqueName);

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

	SaveInstanceTreeW(iInstance, dlgFile.GetPathName());
}

// ------------------------------------------------------------------------------------------------
void _controller::ScaleAndCenter()
{
	ASSERT(FALSE); // OBSOLETE
	/*m_pModel->ScaleAndCenter();

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnWorldDimensionsChanged();
	}*/
}

// ------------------------------------------------------------------------------------------------
void _controller::ShowMetaInformation(_instance* /*pInstance*/)
{
	ASSERT(FALSE); // OBSOLETE
}

// ------------------------------------------------------------------------------------------------
void _controller::SetTargetInstance(_view* pSender, _instance* pInstance)
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
		(*itView)->OnTargetInstanceChanged(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
_instance* _controller::GetTargetInstance() const
{
	return m_pTargetInstance;
}


void _controller::SelectInstance(_view* pSender, _instance* pInstance)
{
	if (m_bUpdatingModel)
	{
		return;
	}

	m_pSelectedInstance = pInstance;

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstanceSelected(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
_instance* _controller::GetSelectedInstance() const
{
	return m_pSelectedInstance;
}

// ------------------------------------------------------------------------------------------------
BOOL _controller::GetScaleAndCenter() const
{
	return m_bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void _controller::SetScaleAndCenter(BOOL bScaleAndCenter)
{
	m_bScaleAndCenter = bScaleAndCenter;
}

// ------------------------------------------------------------------------------------------------
void _controller::OnInstancesEnabledStateChanged(_view* pSender)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstancesEnabledStateChanged(pSender);
	}
}

// ------------------------------------------------------------------------------------------------
void _controller::OnApplicationPropertyChanged(_view* pSender, enumApplicationProperty enApplicationProperty)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnApplicationPropertyChanged(pSender, enApplicationProperty);
	}
}

// ------------------------------------------------------------------------------------------------
void _controller::OnViewRelations(_view* pSender, SdaiInstance iInstance)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnViewRelations(pSender, iInstance);
	}
}

// ------------------------------------------------------------------------------------------------
//void _controller::OnViewRelations(_view* pSender, CEntity* pEntity)
//{
//	m_pTargetInstance = nullptr;
//
//	auto itView = m_setViews.begin();
//	for (; itView != m_setViews.end(); itView++)
//	{
//		(*itView)->OnViewRelations(pSender, pEntity);
//	}
//}


void _controller::OnInstanceAttributeEdited(_view* pSender, SdaiInstance iInstance, SdaiAttr pAttribute)
{
	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnInstanceAttributeEdited(pSender, iInstance, pAttribute);
	}
}