#include "stdafx.h"
#include "Controller.h"

// ************************************************************************************************
CController::CController()
	: _controller()
{
	wchar_t szAppPath[_MAX_PATH];
	::GetModuleFileName(::GetModuleHandle(nullptr), szAppPath, sizeof(szAppPath));

	fs::path pthExe = szAppPath;
	auto pthRootFolder = pthExe.parent_path();

	wstring strSettingsFile = pthRootFolder.wstring();
	strSettingsFile += L"\\STEPViewer_STEP.settings";

	m_pSettingsStorage->loadSettings(strSettingsFile);
}

/*virtual*/ CController::~CController()
{
}

