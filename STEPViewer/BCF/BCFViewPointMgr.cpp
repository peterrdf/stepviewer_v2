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

}

bool CBCFViewPointMgr::SaveSelection(BCFViewPoint& vp)
{
	return true;
}
