#include "stdafx.h"

#include "AP242OpenGLView.h"
#include "Resource.h"

#include "_instance.h"

// ************************************************************************************************
const wchar_t BOUNDING_BOX_VAO[] = L"BOUNDING_BOX_VAO";
const wchar_t BOUNDING_BOX_VBO[] = L"BOUNDING_BOX_VBO";
const wchar_t BOUNDING_BOX_IBO[] = L"BOUNDING_BOX_IBO";

// ************************************************************************************************
CAP242OpenGLView::CAP242OpenGLView(CWnd* pWnd)
	: _oglView()
{
	_initialize(
		pWnd,
		16,
#ifdef _BLINN_PHONG_SHADERS
		IDR_TEXTFILE_VERTEX_SHADER2,
		IDR_TEXTFILE_FRAGMENT_SHADER2,
#else
		IDR_TEXTFILE_VERTEX_SHADER3,
		IDR_TEXTFILE_FRAGMENT_SHADER3,
#endif
		TEXTFILE,
		false);

	// Default settings
	m_strCullFaces = CULL_FACES_FRONT;
}

CAP242OpenGLView::~CAP242OpenGLView()
{
}

/*virtual*/ void CAP242OpenGLView::_reset() /*override*/
{
	_oglRenderer::_reset();

	m_strCullFaces = CULL_FACES_FRONT;
}

/*virtual*/ void CAP242OpenGLView::_postDraw() /*override*/
{
	for (auto pModel : getController()->getModels()) {
		DrawBoundingBoxes(pModel);
	}
}

void CAP242OpenGLView::DrawBoundingBoxes(_model* pModel)
{
	if (pModel == nullptr) {
		return;
	}

	if (!getShowBoundingBoxes()) {
		return;
	}

#ifdef _BLINN_PHONG_SHADERS
	m_pOGLProgram->_enableBlinnPhongModel(false);
#else
	m_pOGLProgram->_enableLighting(false);
#endif
	m_pOGLProgram->_setAmbientColor(0.f, 0.f, 0.f);
	m_pOGLProgram->_setTransparency(1.f);

	_oglUtils::checkForErrors();

	bool bIsNew = false;
	GLuint iVAO = m_worldBuffers.getVAOcreateNewIfNeeded(BOUNDING_BOX_VAO, bIsNew);

	if (iVAO == 0) {
		assert(false);

		return;
	}

	GLuint iVBO = 0;

	if (bIsNew) {
		glBindVertexArray(iVAO);

		iVBO = m_worldBuffers.getBufferCreateNewIfNeeded(BOUNDING_BOX_VBO, bIsNew);
		if ((iVBO == 0) || !bIsNew) {
			assert(false);

			return;
		}

		glBindBuffer(GL_ARRAY_BUFFER, iVBO);
		m_worldBuffers.setVBOAttributes(m_pOGLProgram);

		GLuint iIBO = m_worldBuffers.getBufferCreateNewIfNeeded(BOUNDING_BOX_IBO, bIsNew);
		if ((iIBO == 0) || !bIsNew) {
			assert(false);

			return;
		}

		vector<unsigned int> vecIndices =
		{
			0, 1,
			1, 2,
			2, 3,
			3, 0,
			4, 5,
			5, 6,
			6, 7,
			7, 4,
			0, 6,
			3, 5,
			1, 7,
			2, 4,
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * vecIndices.size(), vecIndices.data(), GL_STATIC_DRAW);

		glBindVertexArray(0);

		_oglUtils::checkForErrors();
	} // if (bIsNew)
	else {
		iVBO = m_worldBuffers.getBuffer(BOUNDING_BOX_VBO);
		if (iVBO == 0) {
			assert(false);

			return;
		}
	}

	for (auto pGeometry : pModel->getGeometries()) {
		if (!pGeometry->getShow()) {
			continue;
		}

		for (auto pInstance : pGeometry->getInstances()) {
			if (!pInstance->getEnable()) {
				continue;
			}

			float fXmin = FLT_MAX;
			float fXmax = -FLT_MAX;
			float fYmin = FLT_MAX;
			float fYmax = -FLT_MAX;
			float fZmin = FLT_MAX;
			float fZmax = -FLT_MAX;
			if (pGeometry->getTriangles().empty()) {
				pGeometry->calculateBB(
					pInstance,
					fXmin, fXmax,
					fYmin, fYmax,
					fZmin, fZmax);
			}
			else {
				pGeometry->calculateBB_Faces(
					pInstance,
					fXmin, fXmax,
					fYmin, fYmax,
					fZmin, fZmax);
			}

			if ((fXmin == FLT_MAX) ||
				(fXmax == -FLT_MAX) ||
				(fYmin == FLT_MAX) ||
				(fYmax == -FLT_MAX) ||
				(fZmin == FLT_MAX) ||
				(fZmax == -FLT_MAX)) {
				continue;
			}

			_vector3d vecBoundingBoxMin = { fXmin, fYmin, fZmin };
			_vector3d vecBoundingBoxMax = { fXmax, fYmax, fZmax };

			// Bottom face
			/*
			Min1						Min2
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			|								|
			|								|
			|								|
			|								|
			|								|
			<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			Min4						Min3
			*/

			_matrix4x3 matIdentity;
			_matrix4x3Identity(&matIdentity);

			_vector3d vecMin1 = { vecBoundingBoxMin.x, vecBoundingBoxMin.y, vecBoundingBoxMin.z };
			_vector3d vecMin2 = { vecBoundingBoxMax.x, vecBoundingBoxMin.y, vecBoundingBoxMin.z };
			_vector3d vecMin3 = { vecBoundingBoxMax.x, vecBoundingBoxMin.y, vecBoundingBoxMax.z };
			_vector3d vecMin4 = { vecBoundingBoxMin.x, vecBoundingBoxMin.y, vecBoundingBoxMax.z };

			// Top face
			/*
			Max3						Max4
			>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
			|								|
			|								|
			|								|
			|								|
			|								|
			<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
			Max2						Max1
			*/

			_vector3d vecMax1 = { vecBoundingBoxMax.x, vecBoundingBoxMax.y, vecBoundingBoxMax.z };
			_vector3d vecMax2 = { vecBoundingBoxMin.x, vecBoundingBoxMax.y, vecBoundingBoxMax.z };
			_vector3d vecMax3 = { vecBoundingBoxMin.x, vecBoundingBoxMax.y, vecBoundingBoxMin.z };
			_vector3d vecMax4 = { vecBoundingBoxMax.x, vecBoundingBoxMax.y, vecBoundingBoxMin.z };

			// X, Y, Z, Nx, Ny, Nz
			vector<float> vecVertices =
			{
				(GLfloat)vecMin1.x, (GLfloat)vecMin1.y, (GLfloat)vecMin1.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMin2.x, (GLfloat)vecMin2.y, (GLfloat)vecMin2.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMin3.x, (GLfloat)vecMin3.y, (GLfloat)vecMin3.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMin4.x, (GLfloat)vecMin4.y, (GLfloat)vecMin4.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMax1.x, (GLfloat)vecMax1.y, (GLfloat)vecMax1.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMax2.x, (GLfloat)vecMax2.y, (GLfloat)vecMax2.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMax3.x, (GLfloat)vecMax3.y, (GLfloat)vecMax3.z, 0.f, 0.f, 0.f,
				(GLfloat)vecMax4.x, (GLfloat)vecMax4.y, (GLfloat)vecMax4.z, 0.f, 0.f, 0.f,
			};

			glBindBuffer(GL_ARRAY_BUFFER, iVBO);
			glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vecVertices.size(), vecVertices.data(), GL_DYNAMIC_DRAW);

			glBindVertexArray(iVAO);

			glDrawElementsBaseVertex(GL_LINES,
				(GLsizei)24,
				GL_UNSIGNED_INT,
				(void*)0,
				0);

			glBindVertexArray(0);
		} // for (auto pInstance : pGeometry->getInstances())
	} // for (auto pGeometry : pModel->getGeometries())

	_oglUtils::checkForErrors();
}




