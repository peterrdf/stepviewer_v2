#include "stdafx.h"
#include "BCFViewPointMgr.h"
#include "STEPViewerDoc.h"
#include "STEPViewerView.h"
#include "_ifc_instance.h"
#include "_ifc_model.h"

#define IFC_ROOT "IfcRoot"
#define IFC_SPACE "IfcSpace"
#define IFC_OPENING "IfcOpeningElement"
#define GLOBAL_ID "GlobalId"

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
		ApplyVisibilityToViewer(*vp);
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
			ok = vp->SetCameraDirection(&direction) && ok;
			ok = vp->SetCameraUpVector(&upVector) && ok;
			ok = vp->SetViewToWorldScale(viewToWorldScale) && ok;
			ok = vp->SetFieldOfView(fieldOfView) && ok;
			ok = vp->SetAspectRatio(aspectRatio) && ok;
		}

		ok = SaveSelection(*vp) && ok;
		ok = SaveVisibility(*vp) && ok;
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
	for (auto model : m_view.GetViewerDoc().getModels()) {
		if (auto ifcModel = dynamic_cast<_ifc_model*>(model)) {
			if (auto sdaiModel = ifcModel->getSdaiModel()) {
				if (auto rootEntity = sdaiGetEntity(sdaiModel, IFC_ROOT)) {
					if (auto globalIdAttr = sdaiGetAttrDefinition(rootEntity, GLOBAL_ID)) {

						for (auto inst : ifcModel->getInstances()) {
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
	return NULL;
}

bool CBCFViewPointMgr::SaveSelection(BCFViewPoint& vp)
{
	bool ok = true;

	int i = 0;
	while (auto sel = vp.GetSelection(i)) {
		if (!sel->Remove()) {
			ok = false;
			i++;
		}
	}

	for (auto inst : m_view.GetViewerDoc().getSelectedInstances()) {
		if (auto ifcInst = dynamic_cast<_ifc_instance*>(inst)) {
			if (auto sdaiInst = ifcInst->getSdaiInstance()) {
				const char* globalId = NULL;
				if (sdaiGetAttrBN(sdaiInst, GLOBAL_ID, sdaiSTRING, &globalId) && globalId && *globalId) {
					ok = vp.AddSelection(globalId) && ok;
				}
			}
		}
	}
	
	return ok;
}

void CBCFViewPointMgr::ApplyVisibilityToViewer(BCFViewPoint& vp)
{
	std::unordered_set<_instance*> exceptions;
	int i = 0;
	while (auto comp = vp.GetException(i++)) {
		if (auto inst = SearchComponent(*comp)) {
			exceptions.insert(inst);
		}
	}

	for (auto model : m_view.GetViewerDoc().getModels()) {
		if (model) {

			SdaiEntity space = NULL;
			SdaiEntity opening = NULL;
			if (auto ifcModel = dynamic_cast<_ifc_model*>(model)) {
				if (auto sdaiModel = ifcModel->getSdaiModel()) {
					space = sdaiGetEntity(sdaiModel, IFC_SPACE);
					opening = sdaiGetEntity(sdaiModel, IFC_OPENING);
				}
			}

			for (auto inst : model->getInstances()) {

				bool visible = vp.GetDefaultVisibility();

				if (auto ifcInst = dynamic_cast<_ifc_instance*>(inst)) {
					if (auto sdaiInst = ifcInst->getSdaiInstance()) {
						if (sdaiIsInstanceOf(sdaiInst, space)) {
							visible = vp.GetSpaceVisible();
						}
						else if (sdaiIsInstanceOf(sdaiInst, opening)) {
							visible = vp.GetOpeningsVisible();
						}
						else if (IsSpaceBoundary(sdaiInst)) {
							visible = vp.GetSpaceBoundariesVisible();
						}
					}
				}

				if (exceptions.find(inst) != exceptions.end()) {
					visible = !visible;
				}

				inst->setVisible(visible);
			}
		}
	}
}

bool CBCFViewPointMgr::SaveVisibility(BCFViewPoint& vp)
{
	bool ok = true;

	int allInstances[2] = { 0,0 };
	int spaces[2] = { 0,0 };
	int boundaries[2] = { 0, 0 };
	int openings[2] = { 0, 0 };

	// calculate visible/invisible instances
	//
	for (auto model : m_view.GetViewerDoc().getModels()) {
		if (model) {

			SdaiEntity space = NULL;
			SdaiEntity opening = NULL;
			if (auto ifcModel = dynamic_cast<_ifc_model*>(model)) {
				if (auto sdaiModel = ifcModel->getSdaiModel()) {
					space = sdaiGetEntity(sdaiModel, IFC_SPACE);
					opening = sdaiGetEntity(sdaiModel, IFC_OPENING);
				}
			}

			for (auto inst : model->getInstances()) {

				int ind = inst->getVisible() ? 0 : 1;

				allInstances[ind]++;

				if (auto ifcInst = dynamic_cast<_ifc_instance*>(inst)) {
					if (auto sdaiInst = ifcInst->getSdaiInstance()) {
						if (sdaiIsInstanceOf(sdaiInst, space)) {
							spaces[ind]++;
						}
						else if (sdaiIsInstanceOf(sdaiInst, opening)) {
							openings[ind]++;
						}
						else if (IsSpaceBoundary(sdaiInst)) {
							boundaries[ind]++;
						}
					}
				}
			}
		}
	}

	// set default visibilities
	//
	ok = vp.SetDefaultVisibility(allInstances[0] >= allInstances[1]) && ok;
	ok = vp.SetSpaceVisible(spaces[0] > spaces[1]) && ok;
	ok = vp.SetSpaceBoundariesVisible(boundaries[0] > boundaries[1]) && ok;
	ok = vp.SetOpeningsVisible(openings[0] > openings[1]) && ok;

	//set exceptions
	//
	int i = 0;
	while (auto exception = vp.GetException(i)) {
		if (!exception->Remove()) {
			ok = false;
			i++;
		}
	}

	for (auto model : m_view.GetViewerDoc().getModels()) {
		if (model) {
			SdaiEntity space = NULL;
			SdaiEntity opening = NULL;
			SdaiAttr attrGlobalId = NULL;
			if (auto ifcModel = dynamic_cast<_ifc_model*>(model)) {
				if (auto sdaiModel = ifcModel->getSdaiModel()) {
					space = sdaiGetEntity(sdaiModel, IFC_SPACE);
					opening = sdaiGetEntity(sdaiModel, IFC_OPENING);

					if (auto root = sdaiGetEntity(sdaiModel, IFC_ROOT))
						attrGlobalId = sdaiGetAttrDefinition(root, GLOBAL_ID);
				}
			}

			for (auto inst : model->getInstances()) {

				bool visible = vp.GetDefaultVisibility();

				if (auto ifcInst = dynamic_cast<_ifc_instance*>(inst)) {
					if (auto sdaiInst = ifcInst->getSdaiInstance()) {
						if (sdaiIsInstanceOf(sdaiInst, space)) {
							visible = vp.GetSpaceVisible();
						}
						else if (sdaiIsInstanceOf(sdaiInst, opening)) {
							visible = vp.GetOpeningsVisible();
						}
						else if (IsSpaceBoundary(sdaiInst)) {
							visible = vp.GetSpaceBoundariesVisible();
						}

						if (visible != inst->getVisible()) {
							const char* globalId = NULL;
							if (sdaiGetAttr(sdaiInst, attrGlobalId, sdaiSTRING, &globalId) && globalId && *globalId)
								ok = vp.AddException(globalId) && ok;
						}
					}
				}
			}
		}
	}

	return ok;
}
