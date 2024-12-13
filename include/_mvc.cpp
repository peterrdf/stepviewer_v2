#include "stdafx.h"

#include "_mvc.h"

// ------------------------------------------------------------------------------------------------
void _view::SetController(_controller* pController)
{
	ASSERT(pController != nullptr);

	m_pController = pController;

	OnControllerChanged();
}


/*virtual*/ void _view::OnTargetInstanceChanged(_view* /*pSender*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void _view::OnInstanceSelected(_view* /*pSender*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void _view::OnInstancesEnabledStateChanged(_view* /*pSender*/)
{
}

/*virtual*/ void _view::OnInstanceAttributeEdited(_view* /*pSender*/, SdaiInstance /*iInstance*/, SdaiAttr /*pAttribute*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void _view::OnViewRelations(_view* /*pSender*/, SdaiInstance /*iInstance*/)
{
}

// ------------------------------------------------------------------------------------------------
///*virtual*/ void _view::OnViewRelations(_view* /*pSender*/, CEntity* /*pEntity*/)
//{
//}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void _view::OnApplicationPropertyChanged(_view* /*pSender*/, enumApplicationProperty /*enApplicationProperty*/)
{
}

// ------------------------------------------------------------------------------------------------
/*virtual*/ void _view::OnControllerChanged()
{
}

// ------------------------------------------------------------------------------------------------
_controller* _view::GetController() const
{
	return m_pController;
}

// ************************************************************************************************
_controller::_controller()
	: m_pModel(nullptr)
	, m_setViews()
	, m_pSettingsStorage(new _settings_storage())
	, m_bUpdatingModel(false)
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

_instance* _controller::LoadInstance(OwlInstance iInstance)
{
	ASSERT(iInstance != 0);
	ASSERT(m_pModel != nullptr);

	m_pSelectedInstance = nullptr;

	/*if ((m_pTargetInstance != nullptr) && (dynamic_cast<_ap_instance*>(m_pTargetInstance)->getSdaiInstance() == iInstance))
	{
		return nullptr;
	}*/

	m_pTargetInstance = nullptr;

	m_bUpdatingModel = true;

	auto pInstance = dynamic_cast<_model*>(m_pModel)->LoadInstance(iInstance);

	auto itView = m_setViews.begin();
	for (; itView != m_setViews.end(); itView++)
	{
		(*itView)->OnModelUpdated();
	}

	m_bUpdatingModel = false;

	return pInstance;
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
	_instance::BuildInstanceNames(m_pModel->getOwlInstance(), iInstance, strName, strUniqueName);

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