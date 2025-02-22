#include "stdafx.h"
#include "BCFViewPointMgr.h"
#include "STEPViewerDoc.h"
#include "STEPViewerView.h"


void CBCFViewPointMgr::SetViewFromComment(BCFComment& comment)
{
	if (auto vp = comment.GetViewPoint()) {
		if (auto viewer = m_view.GetViewerView()) {

			//set camera
			BCFCamera camera = vp->GetCameraType();
			BCFPoint viewPoint;
			BCFPoint direction;
			BCFPoint upVector;
			vp->GetCameraViewPoint(viewPoint);
			vp->GetCameraDirection(direction);
			vp->GetCameraUpVector(upVector);
			double viewToWorldScale = vp->GetViewToWorldScale();
			double fieldOfView = vp->GetFieldOfView();
			double aspectRatio = vp->GetAspectRatio();
			viewer->SetBCFView(camera, viewPoint, direction, upVector, viewToWorldScale, fieldOfView, aspectRatio);
		}
		
		ViewSelection(*vp);
	}
}


bool CBCFViewPointMgr::SaveCurrentViewToComent(BCFComment&comment)
{
	bool ok = true;

	auto vp = comment.GetViewPoint();
	if (!vp) {
		vp = comment.GetTopic().AddViewPoint();
		if (vp) {
			ok = comment.SetViewPoint(vp) && ok;
		}
		else {
			ok = false;
		}
	}

	if (vp) {

		if (auto view = m_view.GetViewerView()) {

			//save camera
			BCFCamera camera = BCFCameraOrthogonal;
			BCFPoint viewPoint;
			BCFPoint direction;
			BCFPoint upVector;
			double viewToWorldScale = 1;
			double fieldOfView = 90;
			double aspectRatio = 1;

			view->GetBCFView(camera, viewPoint, direction, upVector, viewToWorldScale, fieldOfView, aspectRatio);

			ok = vp->SetCameraType(camera) && ok;
			ok = vp->SetCameraViewPoint(&viewPoint) && ok;
			ok = vp->SetCameraUpVector(&upVector) && ok;
			ok = vp->SetViewToWorldScale(viewToWorldScale) && ok;
			ok = vp->SetFieldOfView(fieldOfView) && ok;
			ok = vp->SetAspectRatio(aspectRatio) && ok;
		}

		SaveSelection(*vp);
	}

	return ok;

}

void CBCFViewPointMgr::ViewSelection(BCFViewPoint& vp)
{
	auto& viewer = m_view.GetViewerDoc();
	viewer.selectInstance(NULL, NULL);

	int i = 0;
	while (auto comp = vp.GetSelection(i++)) {
		if (comp) {
			if (auto inst = SearchComponent(*comp)) {
				viewer.selectInstance(NULL, inst, true);
			}
		}
	}
}

_instance* CBCFViewPointMgr::SearchComponent(BCFComponent& comp)
{
	auto ifcGuid = comp.GetIfcGuid();
	if (ifcGuid && *ifcGuid) {
		if (auto inst = SearchIfcComponent(ifcGuid)) {
			return inst;
		}
	}

	auto appId = comp.GetAuthoringToolId();
	if (appId && *appId) {
		//...
	}

	return NULL;
}

_instance* CBCFViewPointMgr::SearchIfcComponent(const char* ifcGuid)
{
	for (auto amodel : m_view.GetViewerDoc().getModels()) {
		if (auto model = dynamic_cast<_ap_model*>(amodel)) {
			if (model->getAP() == enumAP::IFC) {
				if (auto sdaiModel = model->getSdaiModel()) {

					if (auto rootEntity = sdaiGetEntity(sdaiModel, "IfcRoot")) {
						if (auto globalIdAttr = sdaiGetAttrDefinition(rootEntity, "GlobalId")) {

							for (auto inst : model->getInstances()) {
								if (auto apInst = dynamic_cast<_ap_instance*>(inst)) {
									if (auto sdaiInst = apInst->getSdaiInstance()) {

										const char* globalId = NULL;
										if (sdaiGetAttr(sdaiInst, globalIdAttr, sdaiSTRING, &globalId) && globalId) {

											if (0 == strcmp(ifcGuid, globalId)) {
												return inst;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return NULL;
}

bool CBCFViewPointMgr::SaveSelection(BCFViewPoint& vp)
{
	return true;
}
