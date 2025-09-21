#include "_host.h"
#include "_webgpuUtils.h"
#include "_instance.h"
#include "_ptr.h"

#ifdef __EMSCRIPTEN__
#include "../../gisengine/Parsers/_string.h"
#endif

#ifdef _WINDOWS
// ************************************************************************************************
_webgpuRendererSettings::_webgpuRendererSettings()
	: m_enProjection(enumProjection::Perspective)
	, m_enRotationMode(enumRotationMode::XYZ)
	, m_fXAngle(0.f)
	, m_fYAngle(0.f)
	, m_fZAngle(0.f)
	, m_rotation(_quaterniond::toQuaternion(0., 0., 0.))
	, m_bGhostView(FALSE)
	, m_fGhostViewTransparency(1.f)
	, m_bShowFaces(TRUE)
	, m_strCullFaces(CULL_FACES_NONE)
	, m_bShowFacesPolygons(FALSE)
	, m_bShowConceptualFacesPolygons(TRUE)
	, m_bShowLines(TRUE)
	, m_fLineWidth(1.f)
	, m_bShowPoints(TRUE)
	, m_fPointSize(1.f)
	, m_bShowBoundingBoxes(FALSE)
	, m_bShowNormalVectors(FALSE)
	, m_bShowTangenVectors(FALSE)
	, m_bShowBiNormalVectors(FALSE)
	, m_fScale(1.f)
	, m_fTranslation(DEFAULT_TRANSLATION)
	, m_strBackgroundColor(L"darkGray")
	, m_fXMousePositionLast(0.f)
	, m_fYMousePositionLast(0.f)
	, m_bRMBPressed(FALSE)
	, m_bMoverMode(FALSE)
{
}

_webgpuRendererSettings::~_webgpuRendererSettings()
{
}

void _webgpuRendererSettings::_reset()
{
	m_enProjection = enumProjection::Perspective;
	m_enRotationMode = enumRotationMode::XYZ;

	m_fXAngle = 0.f;
	m_fYAngle = 0.f;
	m_fZAngle = 0.f;
	m_rotation = _quaterniond::toQuaternion(0., 0., 0.);

	m_bGhostView = FALSE;
	m_fGhostViewTransparency = 1.f;

	m_bShowFaces = TRUE;
	m_strCullFaces = CULL_FACES_NONE;
	m_bShowFacesPolygons = FALSE;
	m_bShowConceptualFacesPolygons = TRUE;

	m_bShowLines = TRUE;
	m_fLineWidth = 1.f;

	m_bShowPoints = TRUE;
	m_fPointSize = 1.f;

	m_bShowBoundingBoxes = FALSE;
	m_bShowNormalVectors = FALSE;
	m_bShowTangenVectors = FALSE;
	m_bShowBiNormalVectors = FALSE;

	m_fScale = 1.f;
	m_fTranslation = DEFAULT_TRANSLATION;

	m_strBackgroundColor = L"darkGray";

	m_fXMousePositionLast = 0.f;
	m_fYMousePositionLast = 0.f;

	m_bRMBPressed = FALSE;
	m_bMoverMode = FALSE;
}

// ************************************************************************************************
void _webgpuView::addUserDefinedMaterial(const vector<_instance*>& vecInstances, float fR, float fG, float fB)
{
	// Create material
	_material* pMaterial = new _material();
	pMaterial->ambient._r = fR;
	pMaterial->ambient._g = fG;
	pMaterial->ambient._b = fB;
	pMaterial->diffuse._r = fR;
	pMaterial->diffuse._g = fG;
	pMaterial->diffuse._b = fB;

	// Associate with instances
	for (_instance* pInstance : vecInstances) {
		m_mapUserDefinedMaterials[pInstance] = pMaterial;
	}
}

void _webgpuView::removeUserDefinedMaterials()
{
	// Delete all materials
	set<_material*> setMaterials;
	for (const auto& pair : m_mapUserDefinedMaterials) {
		setMaterials.insert(pair.second);
	}

	for (_material* pMaterial : setMaterials) {
		delete pMaterial;
	}

	m_mapUserDefinedMaterials.clear();
}

#endif // _WINDOWS