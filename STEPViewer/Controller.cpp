#include "stdafx.h"
#include "Controller.h"

#include "_rdf_mvc.h"

// ************************************************************************************************
CController::CController()
	: _ap_controller()
{
	wchar_t szAppPath[_MAX_PATH];
	::GetModuleFileNameW(::GetModuleHandleW(nullptr), szAppPath, _MAX_PATH);

	fs::path pthExe = szAppPath;
	auto pthRootFolder = pthExe.parent_path();

	wstring strSettingsFile = pthRootFolder.wstring();
	strSettingsFile += L"\\STEPViewer_STEP.settings";

	getSettingsStorage()->loadSettings(strSettingsFile);

	loadSettings();
}

/*virtual*/ CController::~CController()
{
}

/*virtual*/ void CController::loadDecorationModels() /*override*/
{
	addDecorationModel(new _world_coordinate_system_model(this));
	addDecorationModel(new _model_coordinate_system_model(this));
	addDecorationModel(new _navigator_model(this));
}

