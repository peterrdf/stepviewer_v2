#include "stdafx.h"
#include "Controller.h"

#include "_rdf_mvc.h"

// ************************************************************************************************
CController::CController()
	: _ap_controller()
{
	wchar_t szAppPath[_MAX_PATH];
	::GetModuleFileName(::GetModuleHandle(nullptr), szAppPath, sizeof(szAppPath));

	fs::path pthExe = szAppPath;
	auto pthRootFolder = pthExe.parent_path();

	wstring strSettingsFile = pthRootFolder.wstring();
	strSettingsFile += L"\\STEPViewer_STEP.settings";

	getSettingsStorage()->loadSettings(strSettingsFile);
}

/*virtual*/ CController::~CController()
{
}

/*virtual*/ void CController::loadDecorationModels() /*override*/
{
	//addDecorationModel(new _coordinate_system_model(nullptr));//#todo
	addDecorationModel(new _coordinate_system_model(this));
	addDecorationModel(new _navigator_model());
	//addDecorationModel(new _navigator_coordinate_system_model());//#todo
}

