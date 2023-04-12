#pragma once

#include "_geometry.h"
#include "_quaterniond.h"

#include "glew.h"
#include "wglew.h"
#include "vec3.hpp"
#include "vec4.hpp"
#include "mat4x4.hpp"
#include "gtc/matrix_transform.hpp"
#include "gtc/type_ptr.hpp"
#include "glew.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <limits>
#include <map>

#undef max
using namespace std;

class _oglUtils
{

public: // Methods
	
	static GLsizei getVerticesCountLimit(GLint iVertexLengthBytes)
	{
#ifdef WIN64
		return numeric_limits<GLint>::max() / iVertexLengthBytes;
#else
		return 6500000;
#endif
	}

	static GLsizei getIndicesCountLimit()
	{
		return 64800;
	}

	static void checkForErrors()
	{
		GLenum errLast = GL_NO_ERROR;

		for (;;)
		{
			GLenum err = glGetError();
			if (err == GL_NO_ERROR)
				return;

			// normally the error is reset by the call to glGetError() but if
			// glGetError() itself returns an error, we risk looping forever here
			// so check that we get a different error than the last time
			if (err == errLast)
			{
#ifdef _LINUX
				wxLogError(wxT("OpenGL error state couldn't be reset."));
#else
				::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"OpenGL error state couldn't be reset.", L"OpenGL", MB_ICONERROR | MB_OK);

				PostQuitMessage(0);
#endif // _LINUX

				return;
			}

			errLast = err;

#ifdef _LINUX
			wxLogError(wxT("OpenGL error %d"), err);
#else
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), (const wchar_t*)gluErrorStringWIN(errLast), L"OpenGL", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
#endif // _LINUX
		}
	}
};

class _oglShader
{

protected: // Members

	GLenum	m_iType; // GL_VERTEX_SHADER or GL_FRAGMENT_SHADER
	GLuint	m_iID;
	char* m_szCode;

public: // Methods

	_oglShader(GLenum iShaderType)
		: m_iType(iShaderType)
		, m_szCode(nullptr)
	{
		m_iID = glCreateShader(m_iType);
		ASSERT(m_iID != 0);
	}

	virtual ~_oglShader(void)
	{
		if (m_szCode)
		{
			delete[] m_szCode;
		}
			
		if (m_iID != 0)
		{
			glDeleteShader(m_iID);
		}		
	}

	GLuint _getID() const 
	{ 
		return m_iID; 
	}

	static char* getResource(int iResource, int iType)
	{
		HMODULE hModule = ::GetModuleHandleW(nullptr);
		HRSRC hResource = ::FindResourceW(hModule, MAKEINTRESOURCEW(iResource), MAKEINTRESOURCEW(iType));
		HGLOBAL rcData = ::LoadResource(hModule, hResource);

		char* szData = static_cast<char*>(::LockResource(rcData));
		DWORD dwSize = ::SizeofResource(hModule, hResource);

		char* szBuffer = new char[dwSize + 1];
		::memcpy(szBuffer, szData, dwSize);
		szBuffer[dwSize] = 0;

		return szBuffer;
	}

	bool load(int iResource, int iType)
	{
		m_szCode = getResource(iResource, iType);
		
		return (m_szCode != nullptr);
	}

	bool compile(void)
	{
		if (m_szCode == nullptr)
		{
			return false;
		}

		glShaderSource(m_iID, 1, &m_szCode, nullptr);
		glCompileShader(m_iID);

		int iParam;
		glGetShaderiv(m_iID, GL_COMPILE_STATUS, &iParam);

		if (iParam == GL_TRUE)
		{
			return true;
		}
			
		printInfoLog();

		return false;
	}	

	void getInfoLog(CString& strInfoLog)
	{
		strInfoLog = L"NA";

		int iLength = 0;
		glGetShaderiv(m_iID, GL_INFO_LOG_LENGTH, &iLength);

		if (iLength > 0)
		{
			int iCharsWritten = 0;
			char* szInfoLog = new char[iLength];

			glGetShaderInfoLog(m_iID, iLength, &iCharsWritten, szInfoLog);

			strInfoLog = szInfoLog;
			delete[] szInfoLog;
		}
	}

	void printInfoLog()
	{
		CString strInfoLog;
		getInfoLog(strInfoLog);

		::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), strInfoLog, L"Error", MB_ICONERROR | MB_OK);
	}
};

class _oglProgram
{

private: // Members

	GLuint m_iID;

public: // Methods

	_oglProgram(void)
	{
		m_iID = glCreateProgram();
		ASSERT(m_iID > 0);
	}

	virtual ~_oglProgram()
	{
		if (m_iID != 0)
		{
			glDeleteProgram(m_iID);
		}		
	}

	GLuint _getID() const
	{
		return m_iID;
	}

	void _attachShader(_oglShader* pShader) const
	{
		glAttachShader(m_iID, pShader->_getID());
	}

	void _detachShader(_oglShader* pShader) const
	{
		glDetachShader(m_iID, pShader->_getID());
	}

	virtual bool _link()
	{
		glLinkProgram(m_iID);

		int param;
		glGetProgramiv(m_iID, GL_LINK_STATUS, &param);

		return param == GL_TRUE;
	}

	void _use() const
	{ 
		glUseProgram(m_iID); 
	}

protected: // Methods

	GLint _getUniformLocation(char* szName) const
	{
		return glGetUniformLocation(m_iID, szName);
	}

	void _setUniform(char* szName, int iVal) const
	{
		GLint iLocation = glGetUniformLocation(m_iID, szName);
		glUniform1i(iLocation, iVal);
	}

	void _setUniform(GLint iLocation, int iVal) const
	{
		glUniform1i(iLocation, iVal);
	}

	void _setUniform(char* szName, int* val, int varDim, int count) const
	{
		GLint iLocation = glGetUniformLocation(m_iID, szName);
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}			

		if (varDim == 4)
			glUniform4iv(iLocation, count, val);
		else if (varDim == 3)
			glUniform3iv(iLocation, count, val);
		else if (varDim == 2)
			glUniform2iv(iLocation, count, val);
		else if (varDim == 1)
			glUniform1iv(iLocation, count, val);
	}

	void _setUniform(GLint iLocation, int* val, int varDim, int count) const
	{
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		if (varDim == 4)
			glUniform4iv(iLocation, count, val);
		else if (varDim == 3)
			glUniform3iv(iLocation, count, val);
		else if (varDim == 2)
			glUniform2iv(iLocation, count, val);
		else if (varDim == 1)
			glUniform1iv(iLocation, count, val);
	}

	void _setUniform(char* szName, float val) const
	{
		GLint iLocation = glGetUniformLocation(m_iID, szName);
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		glUniform1f(iLocation, val);
	}

	void _setUniform(GLint iLocation, float val) const
	{
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		glUniform1f(iLocation, val);
	}

	void _setUniform(char* szName, float* val, int varDim, int count) const
	{
		GLint iLocation = glGetUniformLocation(m_iID, szName);
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}			

		if (varDim == 4)
			glUniform4fv(iLocation, count, val);
		else if (varDim == 3)
			glUniform3fv(iLocation, count, val);
		else if (varDim == 2)
			glUniform2fv(iLocation, count, val);
		else if (varDim == 1)
			glUniform1fv(iLocation, count, val);
	}

	void _setUniformMatrix(char* szName, float* mat, int dimX, int dimY, bool bTranspose) const
	{
		GLint iLocation = glGetUniformLocation(m_iID, szName);
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		if ((dimX == 4) && (dimY == 4))
		{
			glUniformMatrix4fv(iLocation, 1, bTranspose, mat);
		}
		else
		{
			ASSERT(FALSE);
		}			
	}

	void _setUniformMatrix(GLint iLocation, float* mat, int dimX, int dimY, bool bTranspose) const
	{
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		if ((dimX == 4) && (dimY == 4))
		{
			glUniformMatrix4fv(iLocation, 1, bTranspose, mat);
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	void _setUniform(GLint iLocation, float* val, int varDim, int count) const
	{
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		if (varDim == 4)
			glUniform4fv(iLocation, count, val);
		else if (varDim == 3)
			glUniform3fv(iLocation, count, val);
		else if (varDim == 2)
			glUniform2fv(iLocation, count, val);
		else if (varDim == 1)
			glUniform1fv(iLocation, count, val);
	}

	GLint _getAttribLocation(char* szName) const
	{ 
		return glGetAttribLocation(m_iID, szName);
	}

	void _setAttrib(GLint iLocation, float val) const
	{ 
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		glVertexAttrib1f(iLocation, val);
	}

	void _setAttrib(GLint iLocation, float* val, int varDim) const
	{
		if (iLocation == -1)
		{
			ASSERT(FALSE);

			return;
		}

		if (varDim == 4)
			glVertexAttrib4fv(iLocation, val);
		else if (varDim == 3)
			glVertexAttrib3fv(iLocation, val);
		else if (varDim == 2)
			glVertexAttrib2fv(iLocation, val);
		else if (varDim == 1)
			glVertexAttrib1fv(iLocation, val);
	}

	void _bindAttribLocation(unsigned int iIndex, char* szName) const
	{
		glBindAttribLocation(m_iID, iIndex, szName);

		unsigned int iError = glGetError();
		VERIFY(iError == 0);

		DWORD dwError = GetLastError();
		VERIFY(dwError == 0);
	}

	GLint _enableVertexAttribArray(char* szName) const
	{
		GLint iLocation = glGetAttribLocation(m_iID, szName);
		if (iLocation != -1)
		{
			glEnableVertexAttribArray(iLocation);
		}			

		return iLocation;
	}

	void _disableVertexAttribArray(GLint iLocation) const
	{
		glDisableVertexAttribArray(iLocation);
	}

	void _getInfoLog(CString& strInfoLog) const
	{
		strInfoLog = L"NA";

		int iLength = 0;
		glGetProgramiv(m_iID, GL_INFO_LOG_LENGTH, &iLength);

		if (iLength > 0)
		{
			int iCharsWritten = 0;
			char* szInfoLog = new char[iLength];

			glGetProgramInfoLog(m_iID, iLength, &iCharsWritten, szInfoLog);

			strInfoLog = szInfoLog;
			delete[] szInfoLog;
		}
	}

	void _printInfoLog() const
	{
		CString strInfoLog;
		_getInfoLog(strInfoLog);

		::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), strInfoLog, L"Error", MB_ICONERROR | MB_OK);
	}

	glm::vec3 _getUniform3f(GLint iUniform) const
	{
		float arValue[3] = { 0.f, 0.f, 0.f };
		glGetUniformfv(_getID(),
			iUniform,
			arValue);

		return glm::vec3(arValue[0], arValue[1], arValue[2]);
	}	

	void _setUniform3f(GLint iUniform, const glm::vec3& value) const
	{
		_setUniform3f(
			iUniform,
			value.x,
			value.y,
			value.z);
	}

	void _setUniform3f(GLint iUniform, float fX, float fY, float fZ) const
	{
		glProgramUniform3f(
			_getID(),
			iUniform,
			fX,
			fY,
			fZ);
	}

	float _getUniform1f(GLint iUniform) const
	{
		float fValue = 0.f;

		glGetUniformfv(_getID(),
			iUniform,
			&fValue);

		return fValue;
	}

	void _setUniform1f(GLint iUniform, float fValue) const
	{
		glProgramUniform1f(
			_getID(),
			iUniform,
			fValue);
	}
};

class _oglBlinnPhongProgram : public _oglProgram
{

#pragma region Members

private: // Members

	// OpenGL

	bool m_bSupportsTexture;

	GLint m_iUseBlinnPhongModel;

	GLint m_iUseTexture;
	GLint m_iSampler;

	GLint m_iMVMatrix;
	GLint m_iPMatrix;
	GLint m_iNMatrix;

	GLint m_iPointLightingLocation;
	GLint m_iAmbientLightWeighting;
	GLint m_iSpecularLightWeighting;
	GLint m_iDiffuseLightWeighting;

	GLint m_iMaterialShininess;

	GLint m_iContrast;
	GLint m_iBrightness;
	GLint m_iGamma;

	GLint m_iMaterialAmbientColor;
	GLint m_iTransparency;
	GLint m_iMaterialDiffuseColor;
	GLint m_iMaterialSpecularColor;
	GLint m_iMaterialEmissiveColor;

	GLint m_iVertexPosition;
	GLint m_iVertexNormal;
	GLint m_iTextureCoord;

#pragma endregion 

public: // Methods

	_oglBlinnPhongProgram(bool bSupportsTexture)
		: _oglProgram()
		, m_bSupportsTexture(bSupportsTexture)
		, m_iUseBlinnPhongModel(-1)
		, m_iUseTexture(-1)
		, m_iSampler(-1)
		, m_iMVMatrix(-1)
		, m_iPMatrix(-1)
		, m_iNMatrix(-1)
		, m_iPointLightingLocation(-1)
		, m_iAmbientLightWeighting(-1)
		, m_iSpecularLightWeighting(-1)
		, m_iDiffuseLightWeighting(-1)
		, m_iMaterialShininess(-1)
		, m_iContrast(-1)
		, m_iBrightness(-1)
		, m_iGamma(-1)
		, m_iMaterialAmbientColor(-1)
		, m_iTransparency(-1)
		, m_iMaterialDiffuseColor(-1)
		, m_iMaterialSpecularColor(-1)
		, m_iMaterialEmissiveColor(-1)
		, m_iVertexPosition(-1)
		, m_iVertexNormal(-1)
		, m_iTextureCoord(-1)
	{
	}

	virtual ~_oglBlinnPhongProgram(void)
	{
	}

	bool _getSupportsTexture() const
	{
		return m_bSupportsTexture;
	}

	void _enableBlinnPhongModel(bool bEnable)
	{
		_setUniform1f(
			m_iUseBlinnPhongModel,
			bEnable ? 1.f : 0.f);
	}

	void _enableTexture(bool bEnable)
	{
		ASSERT(m_bSupportsTexture);

		_setUniform1f(
			m_iUseTexture,
			bEnable ? 1.f : 0.f);
	}

	glm::vec3 _getPointLightingLocation() const 
	{ 
		return _getUniform3f(m_iPointLightingLocation);
	}

	void _setPointLightingLocation(const glm::vec3& value) 
	{ 
		_setUniform3f(
			m_iPointLightingLocation, 
			value);
	}

	glm::vec3 _getAmbientLightWeighting() const
	{
		return _getUniform3f(m_iAmbientLightWeighting);
	}

	void _setAmbientLightWeighting(float fX, float fY, float fZ) const
	{
		ASSERT(m_iAmbientLightWeighting >= 0);

		_setUniform3f(
			m_iAmbientLightWeighting,
			fX,
			fY,
			fZ);
	}

	glm::vec3 _getSpecularLightWeighting() const
	{
		return _getUniform3f(m_iSpecularLightWeighting);
	}

	void _setSpecularLightWeighting(float fX, float fY, float fZ) const
	{
		_setUniform3f(
			m_iSpecularLightWeighting,
			fX,
			fY,
			fZ);
	}

	glm::vec3 _getDiffuseLightWeighting() const
	{
		return _getUniform3f(m_iDiffuseLightWeighting);
	}

	void _setDiffuseLightWeighting(float fX, float fY, float fZ) const
	{
		_setUniform3f(
			m_iDiffuseLightWeighting,
			fX,
			fY,
			fZ);
	}

	float _getMaterialShininess() const
	{
		return _getUniform1f(m_iMaterialShininess);
	}

	void _setMaterialShininess(float fValue) const
	{
		_setUniform1f(
			m_iMaterialShininess,
			fValue);
	}

	float _getContrast() const
	{
		return _getUniform1f(m_iContrast);
	}

	void _setContrast(float fValue) const
	{
		_setUniform1f(
			m_iContrast,
			fValue);
	}

	float _getBrightness() const
	{
		return _getUniform1f(m_iBrightness);
	}

	void _setBrightness(float fValue)
	{
		_setUniform1f(
			m_iBrightness,
			fValue);
	}

	float _getGamma() const
	{
		return _getUniform1f(m_iGamma);
	}

	void _setGamma(float fValue)
	{
		_setUniform1f(
			m_iGamma,
			fValue);
	}

	void _setAmbientColor(float fR, float fG, float fB)
	{
		_setUniform3f(
			m_iMaterialAmbientColor,
			fR,
			fG,
			fB);
	}

	void _setAmbientColor(const _material* pMaterial)
	{
		ASSERT(pMaterial != nullptr);

		_setAmbientColor(
			pMaterial->getAmbientColor().r(),
			pMaterial->getAmbientColor().g(),
			pMaterial->getAmbientColor().b());
	}

	void _setTransparency(float fA)
	{
		_setUniform1f(
			m_iTransparency,
			fA);
	}

	void _setDiffuseColor(const _material* pMaterial)
	{
		ASSERT(pMaterial != nullptr);

		_setUniform3f(
			m_iMaterialDiffuseColor,
			pMaterial->getDiffuseColor().r() / 2.f,
			pMaterial->getDiffuseColor().g() / 2.f,
			pMaterial->getDiffuseColor().b() / 2.f);
	}

	void _setSpecularColor(const _material* pMaterial)
	{
		ASSERT(pMaterial != nullptr);

		_setUniform3f(
			m_iMaterialSpecularColor,
			pMaterial->getSpecularColor().r() / 2.f,
			pMaterial->getSpecularColor().g() / 2.f,
			pMaterial->getSpecularColor().b() / 2.f);
	}

	void _setEmissiveColor(const _material* pMaterial)
	{
		ASSERT(pMaterial != nullptr);

		_setUniform3f(
			m_iMaterialEmissiveColor,
			pMaterial->getEmissiveColor().r() / 3.f,
			pMaterial->getEmissiveColor().g() / 3.f,
			pMaterial->getEmissiveColor().b() / 3.f);
	}

	void _setMaterial(const _material* pMaterial)
	{
		ASSERT(pMaterial != nullptr);

		_setAmbientColor(pMaterial);
		_setTransparency(pMaterial->getA());
		_setDiffuseColor(pMaterial);
		_setSpecularColor(pMaterial);
		_setEmissiveColor(pMaterial);
	}

	virtual bool _link() override
	{
		if (!_oglProgram::_link())
		{
			ASSERT(FALSE);

			return false;
		}

		m_iUseBlinnPhongModel = glGetUniformLocation(_getID(), "uUseBlinnPhongModel");
		ASSERT(m_iUseBlinnPhongModel >= 0);

		if (m_bSupportsTexture)
		{
			m_iUseTexture = glGetUniformLocation(_getID(), "uUseTexture");
			ASSERT(m_iUseTexture >= 0);

			m_iSampler = glGetUniformLocation(_getID(), "uSampler");
			ASSERT(m_iSampler >= 0);
		}

		m_iMVMatrix = glGetUniformLocation(_getID(), "uMVMatrix");
		ASSERT(m_iMVMatrix >= 0);

		m_iPMatrix = glGetUniformLocation(_getID(), "uPMatrix");
		ASSERT(m_iPMatrix >= 0);

		m_iNMatrix = glGetUniformLocation(_getID(), "uNMatrix");
		ASSERT(m_iNMatrix >= 0);

		m_iPointLightingLocation = glGetUniformLocation(_getID(), "uPointLightingLocation");
		ASSERT(m_iPointLightingLocation >= 0);

		m_iAmbientLightWeighting = glGetUniformLocation(_getID(), "uAmbientLightWeighting");
		ASSERT(m_iAmbientLightWeighting >= 0);

		m_iSpecularLightWeighting = glGetUniformLocation(_getID(), "uSpecularLightWeighting");
		ASSERT(m_iSpecularLightWeighting >= 0);

		m_iDiffuseLightWeighting = glGetUniformLocation(_getID(), "uDiffuseLightWeighting");
		ASSERT(m_iDiffuseLightWeighting >= 0);

		m_iMaterialShininess = glGetUniformLocation(_getID(), "uMaterialShininess");
		ASSERT(m_iMaterialShininess >= 0);

		m_iContrast = glGetUniformLocation(_getID(), "uContrast");
		ASSERT(m_iContrast >= 0);

		m_iBrightness = glGetUniformLocation(_getID(), "uBrightness");
		ASSERT(m_iBrightness >= 0);

		m_iGamma = glGetUniformLocation(_getID(), "uGamma");
		ASSERT(m_iGamma >= 0);

		m_iMaterialAmbientColor = glGetUniformLocation(_getID(), "uMaterialAmbientColor");
		ASSERT(m_iMaterialAmbientColor >= 0);

		m_iTransparency = glGetUniformLocation(_getID(), "uTransparency");
		ASSERT(m_iTransparency >= 0);

		m_iMaterialDiffuseColor = glGetUniformLocation(_getID(), "uMaterialDiffuseColor");
		ASSERT(m_iMaterialDiffuseColor >= 0);

		m_iMaterialSpecularColor = glGetUniformLocation(_getID(), "uMaterialSpecularColor");
		ASSERT(m_iMaterialSpecularColor >= 0);

		m_iMaterialEmissiveColor = glGetUniformLocation(_getID(), "uMaterialEmissiveColor");
		ASSERT(m_iMaterialEmissiveColor >= 0);

		m_iVertexPosition = glGetAttribLocation(_getID(), "aVertexPosition");
		ASSERT(m_iVertexPosition >= 0);

		m_iVertexNormal = glGetAttribLocation(_getID(), "aVertexNormal");
		ASSERT(m_iVertexNormal >= 0);

		if (m_bSupportsTexture)
		{
			m_iTextureCoord = glGetAttribLocation(_getID(), "aTextureCoord");
			ASSERT(m_iTextureCoord >= 0);
		}

		return true;
	}

	GLint _getVertexPosition() const
	{
		return m_iVertexPosition;
	}

	GLint _getVertexNormal() const
	{
		return m_iVertexNormal;
	}

	GLint _getTextureCoord() const
	{
		ASSERT(m_bSupportsTexture);

		return m_iTextureCoord;
	}

	void _setProjectionMatrix(glm::mat4& matProjection) const
	{
		glProgramUniformMatrix4fv(
			_getID(),
			m_iPMatrix,
			1,
			false,
			value_ptr(matProjection));
	}

	void _setModelViewMatrix(glm::mat4& matModelView) const
	{
		glProgramUniformMatrix4fv(
			_getID(),
			m_iMVMatrix,
			1,
			false,
			value_ptr(matModelView));
	}

	void _setNormalMatrix(glm::mat4& matNormal) const
	{
		glProgramUniformMatrix4fv(
			_getID(),
			m_iNMatrix,
			1,
			false,
			value_ptr(matNormal));
	}	

	void _setSampler(int iSampler) const
	{
		ASSERT(m_bSupportsTexture);

		glProgramUniform1i(
			_getID(),
			m_iSampler,
			iSampler);
	}
};

static LRESULT CALLBACK WndProc(HWND hWnd, UINT uiMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uiMsg)
	{
	case WM_CLOSE:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, uiMsg, wParam, lParam);
	}

	return 0;
}

class _oglContext
{

public: // Constants

	const int MIN_GL_MAJOR_VERSION = 3;
	const int MIN_GL_MINOR_VERSION = 3;

private: // Members

	HDC m_hDC;
	HGLRC m_hGLContext;

	int m_iSamples;

public: // Methods

	_oglContext(HDC hDC, int iSamples)
		: m_hDC(hDC)
		, m_hGLContext(nullptr)
		, m_iSamples(iSamples)
	{
		ASSERT(m_hDC != nullptr);

		create();
	}

	virtual ~_oglContext()
	{
		if (m_hGLContext != nullptr)
		{
			BOOL bResult = wglMakeCurrent(m_hDC, nullptr);
			ASSERT(bResult);

			bResult = wglDeleteContext(m_hGLContext);
			ASSERT(bResult);
		}
	}

	BOOL makeCurrent()
	{
		ASSERT(m_hDC != nullptr);
		ASSERT(m_hGLContext != nullptr);

		return wglMakeCurrent(m_hDC, m_hGLContext);
	}

	void create()
	{
		// https://www.opengl.org/wiki/Creating_an_OpenGL_Context_(WGL)

		PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd  
			1,                     // version number  
			PFD_DRAW_TO_WINDOW |   // support window  
			PFD_SUPPORT_OPENGL |   // support OpenGL  
			PFD_DOUBLEBUFFER,      // double buffered  
			PFD_TYPE_RGBA,         // RGBA type  
			32,                    // 32-bit color depth  
			0, 0, 0, 0, 0, 0,      // color bits ignored  
			0,                     // no alpha buffer  
			0,                     // shift bit ignored  
			0,                     // no accumulation buffer  
			0, 0, 0, 0,            // accum bits ignored  
			24,                    // 24-bit z-buffer
			8,                     // 8-bit stencil buffer
			0,                     // no auxiliary buffer  
			PFD_MAIN_PLANE,        // main layer  
			0,                     // reserved  
			0, 0, 0                // layer masks ignored  
		};

		DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		WNDCLASSEX WndClassEx;
		memset(&WndClassEx, 0, sizeof(WNDCLASSEX));

		WndClassEx.cbSize = sizeof(WNDCLASSEX);
		WndClassEx.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		WndClassEx.lpfnWndProc = WndProc;
		WndClassEx.hInstance = ::AfxGetInstanceHandle();
		WndClassEx.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
		WndClassEx.hIconSm = LoadIcon(nullptr, IDI_APPLICATION);
		WndClassEx.hCursor = LoadCursor(nullptr, IDC_ARROW);
		WndClassEx.lpszClassName = L"_OpenGL_Renderer_Window_";

		if (!GetClassInfoEx(::AfxGetInstanceHandle(), WndClassEx.lpszClassName, &WndClassEx))
		{
			if (RegisterClassEx(&WndClassEx) == 0)
			{
				::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"RegisterClassEx() failed.", L"Error", MB_ICONERROR | MB_OK);

				PostQuitMessage(0);
			}
		}

		HWND hWndTemp = CreateWindowEx(WS_EX_APPWINDOW, WndClassEx.lpszClassName, L"OpenGL", dwStyle, 0, 0, 600, 600, nullptr, nullptr, ::AfxGetInstanceHandle(), nullptr);

		HDC hDCTemp = ::GetDC(hWndTemp);

		// Get the best available match of pixel format for the device context   
		int iPixelFormat = ChoosePixelFormat(hDCTemp, &pfd);
		ASSERT(iPixelFormat > 0);

		// Make that the pixel format of the device context  
		BOOL bResult = SetPixelFormat(hDCTemp, iPixelFormat, &pfd);
		ASSERT(bResult);

		HGLRC hTempGLContext = wglCreateContext(hDCTemp);
		ASSERT(hTempGLContext);

		bResult = wglMakeCurrent(hDCTemp, hTempGLContext);
		ASSERT(bResult);

		_oglUtils::checkForErrors();

		/*
		* MSAA support
		*/

		BOOL bMSAASupport = false;

		int arAttributes[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			WGL_SAMPLE_BUFFERS_ARB, GL_TRUE,
			WGL_SAMPLES_ARB, 0,
			0, 0,
		};

		glewExperimental = GL_TRUE;
		if (glewInit() == GLEW_OK)
		{
			GLint glMajorVersion = 0;
			glGetIntegerv(GL_MAJOR_VERSION, &glMajorVersion);

			GLint glMinorVersion = 0;
			glGetIntegerv(GL_MINOR_VERSION, &glMinorVersion);

			bool bWrongGLVersion = false;
			if (glMajorVersion < MIN_GL_MAJOR_VERSION)
			{
				bWrongGLVersion = true;
			}
			else
			{
				if ((glMajorVersion == MIN_GL_MAJOR_VERSION) && (glMinorVersion < MIN_GL_MINOR_VERSION))
				{
					bWrongGLVersion = true;
				}
			}

			if (bWrongGLVersion)
			{
				CString strErrorMessage;
				strErrorMessage.Format(L"OpenGL version %d.%d or higher is required.", MIN_GL_MAJOR_VERSION, MIN_GL_MINOR_VERSION);

				::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), strErrorMessage, L"Error", MB_ICONERROR | MB_OK);

				PostQuitMessage(0);
			}

			/*
			* Pixel format
			*/
			iPixelFormat = 0;
			for (int iSamples = m_iSamples; iSamples >= 0; iSamples--)
			{
				float fAttributes[] = { 0, 0 };
				UINT iNumFormats = 0;

				arAttributes[17/*WGL_SAMPLES_ARB*/] = iSamples;

				if (wglChoosePixelFormatARB(hDCTemp, arAttributes, fAttributes, 1, &iPixelFormat, &iNumFormats) && (iPixelFormat > 0))
				{
					bMSAASupport = TRUE;

					break;
				}
			}
		} // if (glewInit() == GLEW_OK) 
		else
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"glewInit() failed.", L"Error", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
		}

		bResult = wglMakeCurrent(nullptr, nullptr);
		ASSERT(bResult);

		bResult = wglDeleteContext(hTempGLContext);
		ASSERT(bResult);

		if (bMSAASupport)
		{
			bResult = SetPixelFormat(m_hDC, iPixelFormat, &pfd);
			ASSERT(bResult);

			int arContextAttributes[] = {
				WGL_CONTEXT_MAJOR_VERSION_ARB, MIN_GL_MAJOR_VERSION,
				WGL_CONTEXT_MINOR_VERSION_ARB, MIN_GL_MINOR_VERSION,
#ifdef _ENABLE_OPENGL_DEBUG
				WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,
				WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
#endif
				0, 0,
			};

			m_hGLContext = wglCreateContextAttribsARB(m_hDC, 0, arContextAttributes);
			ASSERT(m_hGLContext);
		}
		else
		{
			iPixelFormat = ChoosePixelFormat(m_hDC, &pfd);
			ASSERT(iPixelFormat > 0);

			bResult = SetPixelFormat(m_hDC, iPixelFormat, &pfd);
			ASSERT(bResult);

			m_hGLContext = wglCreateContext(m_hDC);
			ASSERT(m_hGLContext);
		}

		::ReleaseDC(hWndTemp, hDCTemp);
		::DestroyWindow(hWndTemp);

#ifdef _ENABLE_OPENGL_DEBUG
		remove("_OpenGL_Debug_.txt");
#endif
	}

#ifdef _ENABLE_OPENGL_DEBUG
	void enableDebug()
	{
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

		glDebugMessageCallbackARB(&_oglContext::debugCallback, nullptr);
		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
	}

	static void debugCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei /*length*/, const GLchar* message, const void* /*userParam*/)
	{
		debugOutputToFile(source, type, id, severity, message);
	}

	static void debugOutputToFile(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, const char* message)
	{
		FILE* f = fopen("_OpenGL_Debug_.txt", "a");
		if (f)
		{
			char debSource[50], debType[50], debSev[50];

			// source
			if (source == GL_DEBUG_SOURCE_API_ARB)
				strcpy(debSource, "OpenGL");
			else if (source == GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB)
				strcpy(debSource, "Windows");
			else if (source == GL_DEBUG_SOURCE_SHADER_COMPILER_ARB)
				strcpy(debSource, "Shader Compiler");
			else if (source == GL_DEBUG_SOURCE_THIRD_PARTY_ARB)
				strcpy(debSource, "Third Party");
			else if (source == GL_DEBUG_SOURCE_APPLICATION_ARB)
				strcpy(debSource, "Application");
			else if (source == GL_DEBUG_SOURCE_OTHER_ARB)
				strcpy(debSource, "Other");

			// type
			if (type == GL_DEBUG_TYPE_ERROR_ARB)
				strcpy(debType, "Error");
			else if (type == GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB)
				strcpy(debType, "Deprecated behavior");
			else if (type == GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB)
				strcpy(debType, "Undefined behavior");
			else if (type == GL_DEBUG_TYPE_PORTABILITY_ARB)
				strcpy(debType, "Portability");
			else if (type == GL_DEBUG_TYPE_PERFORMANCE_ARB)
				strcpy(debType, "Performance");
			else if (type == GL_DEBUG_TYPE_OTHER_ARB)
				strcpy(debType, "Other");

			// severity
			if (severity == GL_DEBUG_SEVERITY_HIGH_ARB)
				strcpy(debSev, "High");
			else if (severity == GL_DEBUG_SEVERITY_MEDIUM_ARB)
				strcpy(debSev, "Medium");
			else if (severity == GL_DEBUG_SEVERITY_LOW_ARB)
				strcpy(debSev, "Low");

			fprintf(f, "Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);

			fclose(f);
		} // if (f)
	}
#endif
};

#define BUFFER_SIZE 512

class _oglFramebuffer
{

private: // Members

	GLuint m_iFrameBuffer;
	GLuint m_iTextureBuffer;
	GLuint m_iRenderBuffer;

public: // Methods

	_oglFramebuffer()
		: m_iFrameBuffer(0)
		, m_iTextureBuffer(0)
		, m_iRenderBuffer(0)
	{
	}

	virtual ~_oglFramebuffer()
	{
		if (m_iFrameBuffer != 0)
		{
			glDeleteFramebuffers(1, &m_iFrameBuffer);
		}

		if (m_iTextureBuffer != 0)
		{
			glDeleteTextures(1, &m_iTextureBuffer);
		}

		if (m_iRenderBuffer != 0)
		{
			glDeleteRenderbuffers(1, &m_iRenderBuffer);
		}
	}

	void create(GLsizei iWidth = BUFFER_SIZE, GLsizei iHeight = BUFFER_SIZE)
	{
		if (m_iFrameBuffer == 0)
		{
			ASSERT(m_iTextureBuffer == 0);
			ASSERT(m_iRenderBuffer == 0);

			/*
			* Frame buffer
			*/
			glGenFramebuffers(1, &m_iFrameBuffer);
			ASSERT(m_iFrameBuffer != 0);

			glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);

			/*
			* Texture buffer
			*/
			glGenTextures(1, &m_iTextureBuffer);
			ASSERT(m_iTextureBuffer != 0);

			glBindTexture(GL_TEXTURE_2D, m_iTextureBuffer);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iWidth, iHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

			glBindTexture(GL_TEXTURE_2D, 0);

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_iTextureBuffer, 0);

			/*
			* Depth buffer
			*/
			glGenRenderbuffers(1, &m_iRenderBuffer);
			ASSERT(m_iRenderBuffer != 0);

			glBindRenderbuffer(GL_RENDERBUFFER, m_iRenderBuffer);
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, iWidth, iHeight);

			glBindRenderbuffer(GL_RENDERBUFFER, 0);

			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_iRenderBuffer);

			GLenum arDrawBuffers[1] = { GL_COLOR_ATTACHMENT0 };
			glDrawBuffers(1, arDrawBuffers);

			glBindFramebuffer(GL_FRAMEBUFFER, 0);

			_oglUtils::checkForErrors();
		} // if (m_iFrameBuffer == 0)
	}

	virtual bool isInitialized() const
	{
		return m_iFrameBuffer != 0;
	}

	void bind()
	{
		ASSERT(m_iFrameBuffer != 0);

		glBindFramebuffer(GL_FRAMEBUFFER, m_iFrameBuffer);
	}

	void unbind()
	{
		ASSERT(m_iFrameBuffer != 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}
};

class _oglSelectionFramebuffer : public _oglFramebuffer
{

private: // Members

	map<int64_t, _color> m_mapEncoding; // ID : Color

public: // Methods

	_oglSelectionFramebuffer()
		: _oglFramebuffer()
		, m_mapEncoding()
	{
	}

	virtual ~_oglSelectionFramebuffer()
	{
	}

	map<int64_t, _color>& encoding()
	{
		return m_mapEncoding;
	}
};

// (255 * 255 * 255)[R] + (255 * 255)[G] + 255[B]
class _i64RGBCoder
{

public: // Methods

	static void encode(int64_t i, float& fR, float& fG, float& fB)
	{
		static const float STEP = 1.f / 255.f;

		fR = 0.f;
		fG = 0.f;
		fB = 0.f;

		// R
		if (i >= (255 * 255))
		{
			int64_t iR = i / (255 * 255);
			fR = iR * STEP;

			i -= iR * (255 * 255);
		}

		// G
		if (i >= 255)
		{
			int64_t iG = i / 255;
			fG = iG * STEP;

			i -= iG * 255;
		}

		// B		
		fB = i * STEP;
	}

	static int64_t decode(unsigned char R, unsigned char G, unsigned char B)
	{
		int64_t i = 0;

		// R
		i += R * (255 * 255);

		// G
		i += G * 255;

		// B
		i += B;

		return i;
	}
};

template <class Instance>
class _oglBuffers
{

private: // Members

	map<GLuint, vector<Instance*>> m_mapInstancesCohorts;
	map<wstring, GLuint> m_mapVAOs;
	map<wstring, GLuint> m_mapBuffers;

public: // Methods

	_oglBuffers()
		: m_mapInstancesCohorts()
		, m_mapVAOs()
		, m_mapBuffers()
	{
	}

	virtual ~_oglBuffers()
	{
	}

	map<GLuint, vector<Instance*>>& instancesCohorts()
	{
		return m_mapInstancesCohorts;
	}

	map<wstring, GLuint>& VAOs()
	{
		return m_mapVAOs;
	}

	map<wstring, GLuint>& buffers()
	{
		return m_mapBuffers;
	}

	GLuint findVAO(Instance* pInstance)
	{
		for (auto itCohort : m_mapInstancesCohorts)
		{
			for (auto itInstance : itCohort.second)
			{
				if (pInstance == itInstance)
				{
					return itCohort.first;
				}
			}
		}

		return 0;
	}

	GLuint getVAO(const wstring& strName)
	{
		auto itVAO = m_mapVAOs.find(strName);
		if (itVAO != m_mapVAOs.end())
		{
			return itVAO->second;
		}

		return 0;
	}

	GLuint getVAOcreateNew(const wstring& strName, bool& bIsNew)
	{
		bIsNew = false;

		GLuint iVAO = getVAO(strName);
		if (iVAO == 0)
		{
			glGenVertexArrays(1, &iVAO);
			if (iVAO == 0)
			{
				ASSERT(FALSE);

				return 0;
			}

			_oglUtils::checkForErrors();

			bIsNew = true;
			m_mapVAOs[strName] = iVAO;
		}

		return iVAO;
	}

	GLuint getBuffer(const wstring& strName)
	{
		auto itBuffer = m_mapBuffers.find(strName);
		if (itBuffer != m_mapBuffers.end())
		{
			return itBuffer->second;
		}

		return 0;
	}

	GLuint getBufferCreateNew(const wstring& strName, bool& bIsNew)
	{
		bIsNew = false;

		GLuint iBuffer = getBuffer(strName);
		if (iBuffer == 0)
		{
			glGenBuffers(1, &iBuffer);
			if (iBuffer == 0)
			{
				ASSERT(FALSE);

				return 0;
			}

			_oglUtils::checkForErrors();

			bIsNew = true;
			m_mapBuffers[strName] = iBuffer;
		}

		return iBuffer;
	}

	int64_t createIBO(const vector<_cohort*>& vecCohorts)
	{
		if (vecCohorts.empty())
		{
			ASSERT(FALSE);

			return 0;
		}

		GLuint iIBO = 0;
		glGenBuffers(1, &iIBO);

		if (iIBO == 0)
		{
			ASSERT(FALSE);

			return 0;
		}

		m_mapBuffers[to_wstring(iIBO)] = iIBO;

		int_t iIndicesCount = 0;
		unsigned int* pIndices = _cohort::merge(vecCohorts, iIndicesCount);

		if ((pIndices == nullptr) || (iIndicesCount == 0))
		{
			ASSERT(FALSE);

			return 0;
		}

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iIBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * iIndicesCount, pIndices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		delete[] pIndices;

		_oglUtils::checkForErrors();

		GLsizei iIBOOffset = 0;
		for (auto pCohort : vecCohorts)
		{
			pCohort->ibo() = iIBO;
			pCohort->iboOffset() = iIBOOffset;

			iIBOOffset += (GLsizei)pCohort->indices().size();
		}

		return iIndicesCount;
	}

	int64_t createInstancesCohort(const vector<Instance*>& vecInstances, _oglBlinnPhongProgram* pProgram)
	{
		if (vecInstances.empty() || (pProgram == nullptr))
		{
			ASSERT(FALSE);

			return 0;
		}

		int_t iVerticesCount = 0;
		float* pVertices = getVertices(vecInstances, pProgram->_getSupportsTexture(), iVerticesCount);

		if ((pVertices == nullptr) || (iVerticesCount == 0))
		{
			ASSERT(FALSE);

			return 0;
		}

		GLuint iVAO = 0;
		glGenVertexArrays(1, &iVAO);

		if (iVAO == 0)
		{
			ASSERT(FALSE);

			return 0;
		}

		m_mapVAOs[to_wstring(iVAO)] = iVAO;

		glBindVertexArray(iVAO);

		GLuint iVBO = 0;
		glGenBuffers(1, &iVBO);

		if (iVBO == 0)
		{
			ASSERT(FALSE);

			return 0;
		}

		m_mapBuffers[to_wstring(iVBO)] = iVBO;

		const int64_t _VERTEX_LENGTH = 6 + (pProgram->_getSupportsTexture() ? 2 : 0);

		glBindBuffer(GL_ARRAY_BUFFER, iVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * iVerticesCount * _VERTEX_LENGTH, pVertices, GL_STATIC_DRAW);
		delete[] pVertices;

		setVBOAttributes(pProgram);

		GLsizei iVBOOffset = 0;
		for (auto pInstance : vecInstances)
		{
			pInstance->VBO() = iVBO;
			pInstance->VBOOffset() = iVBOOffset;

			iVBOOffset += (GLsizei)pInstance->GetVerticesCount();
		}

		glBindVertexArray(0);

		_oglUtils::checkForErrors();

		m_mapInstancesCohorts[iVAO] = vecInstances;

		return iVerticesCount;
	}

	void setVBOAttributes(_oglBlinnPhongProgram* pProgram) const
	{
		const int64_t _VERTEX_LENGTH = 6 + (pProgram->_getSupportsTexture() ? 2 : 0);

		glVertexAttribPointer(pProgram->_getVertexPosition(), 3, GL_FLOAT, false, (GLsizei)(sizeof(GLfloat) * _VERTEX_LENGTH), 0);
		glVertexAttribPointer(pProgram->_getVertexNormal(), 3, GL_FLOAT, false, (GLsizei)(sizeof(GLfloat) * _VERTEX_LENGTH), (void*)(sizeof(GLfloat) * 3));
		if (pProgram->_getSupportsTexture())
		{
			glVertexAttribPointer(pProgram->_getTextureCoord(), 2, GL_FLOAT, false, (GLsizei)(sizeof(GLfloat) * _VERTEX_LENGTH), (void*)(sizeof(GLfloat) * 6));
		}

		glEnableVertexAttribArray(pProgram->_getVertexPosition());
		glEnableVertexAttribArray(pProgram->_getVertexNormal());
		if (pProgram->_getSupportsTexture())
		{
			glEnableVertexAttribArray(pProgram->_getTextureCoord());
		}

		_oglUtils::checkForErrors();
	}

	// X, Y, Z, Nx, Ny, Nz, [Tx, Ty]
	static float* getVertices(const vector<Instance*>& vecInstances, bool bSupportsTexture, int_t& iVerticesCount)
	{
		const int64_t _VERTEX_LENGTH = 6 + (bSupportsTexture ? 2 : 0);

		iVerticesCount = 0;
		for (size_t i = 0; i < vecInstances.size(); i++)
		{
			iVerticesCount += vecInstances[i]->GetVerticesCount();
		}

		float* pVertices = new float[iVerticesCount * _VERTEX_LENGTH];

		int_t iOffset = 0;
		for (size_t i = 0; i < vecInstances.size(); i++)
		{
			float* pSrcVertices = getVertices(vecInstances[i], bSupportsTexture);

			memcpy((float*)pVertices + iOffset, pSrcVertices,
				vecInstances[i]->GetVerticesCount() * _VERTEX_LENGTH * sizeof(float));

			delete[] pSrcVertices;

			iOffset += vecInstances[i]->GetVerticesCount() * _VERTEX_LENGTH;
		}

		return pVertices;
	}

	// X, Y, Z, Nx, Ny, Nz, [Tx, Ty]
	static float* getVertices(Instance* pInstance, bool bSupportsTexture)
	{
		const int64_t _SRC_VERTEX_LENGTH = pInstance->GetVertexLength();
		const int64_t _DEST_VERTEX_LENGTH = 6 + (bSupportsTexture ? 2 : 0);

		float* pVertices = new float[pInstance->GetVerticesCount() * _DEST_VERTEX_LENGTH];
		memset(pVertices, 0, pInstance->GetVerticesCount() * _DEST_VERTEX_LENGTH * sizeof(float));

		for (int64_t iVertex = 0; iVertex < pInstance->GetVerticesCount(); iVertex++)
		{
			// X, Y, Z
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 0] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 0];
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 1] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 1];
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 2] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 2];

			// Nx, Ny, Nz
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 3] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 3];
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 4] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 4];
			pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 5] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 5];

			// Tx, Ty
			if (bSupportsTexture)
			{
				pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 6] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 6];
				pVertices[(iVertex * _DEST_VERTEX_LENGTH) + 7] = pInstance->GetVertices()[(iVertex * _SRC_VERTEX_LENGTH) + 7];
			}
		}

		return pVertices;
	}

	void clear()
	{
		m_mapInstancesCohorts.clear();

		for (auto itVAO = m_mapVAOs.begin(); itVAO != m_mapVAOs.end(); itVAO++)
		{
			glDeleteVertexArrays(1, &(itVAO->second));
		}
		m_mapVAOs.clear();

		for (auto itBuffer = m_mapBuffers.begin(); itBuffer != m_mapBuffers.end(); itBuffer++)
		{
			glDeleteBuffers(1, &(itBuffer->second));
		}
		m_mapBuffers.clear();

		_oglUtils::checkForErrors();
	}
};

enum class enumProjection : int
{
	Perspective = 0,
	Orthographic,
};

enum class enumView : int
{
	Front = 0,
	Back,
	Top,
	Bottom,
	Left,
	Right,
	Isometric,
};

enum class enumRotationMode : int
{
	XY = 0, // Standard
	XYZ,	// Quaternions
};

struct _ioglRenderer
{
	virtual _oglProgram* _getOGLProgram() const PURE;

	template<class Program>
	Program* _getOGLProgramAs() const
	{
		return dynamic_cast<Program*>(_getOGLProgram());
	}

	virtual void _redraw() PURE;
};

const float ZOOM_SPEED_MOUSE = 0.025f;
const float ZOOM_SPEED_MOUSE_WHEEL = 0.0125f;
const float ZOOM_SPEED_KEYS = 0.025f;
const float PAN_SPEED_MOUSE = 4.f;
const float PAN_SPEED_KEYS = 40.f;
const float ROTATION_SPEED = 1.f / 25.f;

template <class Instance>
class _oglRenderer : public _ioglRenderer
{

protected: // Members

	CWnd* m_pWnd;
	_oglContext* m_pOGLContext;
	_oglBlinnPhongProgram* m_pOGLProgram;
	_oglShader* m_pVertexShader;
	_oglShader* m_pFragmentShader;
	enumProjection m_enProjection;
	glm::mat4 m_matModelView;	

	_oglBuffers<Instance> m_oglBuffers;

	// Rotation
	enumRotationMode m_enRotationMode;

	// degrees
	float m_fXAngle;
	float m_fYAngle;
	float m_fZAngle;

	// radians
	_quaterniond m_rotation;

	// Translation
	float m_fXTranslation;
	float m_fYTranslation;
	float m_fZTranslation;

public: // Methods

	_oglRenderer()
		: m_pWnd(nullptr)
		, m_pOGLContext(nullptr)
		, m_pOGLProgram(nullptr)
		, m_pVertexShader(nullptr)
		, m_pFragmentShader(nullptr)
		, m_enProjection(enumProjection::Perspective)
		, m_matModelView()
		, m_oglBuffers()
		, m_enRotationMode(enumRotationMode::XY)
		, m_fXAngle(0.f)
		, m_fYAngle(0.f)
		, m_fZAngle(0.f)
		, m_rotation(_quaterniond::toQuaternion(0., 0., 0.))
		, m_fXTranslation(0.0f)
		, m_fYTranslation(0.0f)
		, m_fZTranslation(-5.0f)
	{
		_setView(enumView::Front);
	}	

	// _ioglRenderer
	virtual _oglProgram* _getOGLProgram() const override { return m_pOGLProgram; }
	
	// _ioglRenderer
	virtual void _redraw() override
	{
#ifdef _LINUX
		m_pWnd->Refresh(false);
#else
		m_pWnd->RedrawWindow();
#endif // _LINUX		
	}

	void _initialize(CWnd* pWnd,
		int iSamples, 
		int iVertexShader, 
		int iFragmentShader, 
		int iResourceType,
		bool bSupportsTexture)
	{
		m_pWnd = pWnd;
		ASSERT(m_pWnd != nullptr);

		m_pOGLContext = new _oglContext(*(m_pWnd->GetDC()), iSamples);
		m_pOGLContext->makeCurrent();

		m_pOGLProgram = new _oglBlinnPhongProgram(bSupportsTexture);
		m_pVertexShader = new _oglShader(GL_VERTEX_SHADER);
		m_pFragmentShader = new _oglShader(GL_FRAGMENT_SHADER);

		if (!m_pVertexShader->load(iVertexShader, iResourceType))
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Vertex shader loading error!", L"Error", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
		}

		if (!m_pFragmentShader->load(iFragmentShader, iResourceType))
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Fragment shader loading error!", L"Error", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
		}

		if (!m_pVertexShader->compile())
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Vertex shader compiling error!", L"Error", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
		}

		if (!m_pFragmentShader->compile())
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Fragment shader compiling error!", L"Error", MB_ICONERROR | MB_OK);

			PostQuitMessage(0);
		}

		m_pOGLProgram->_attachShader(m_pVertexShader);
		m_pOGLProgram->_attachShader(m_pFragmentShader);

		glBindFragDataLocation(m_pOGLProgram->_getID(), 0, "FragColor");

		if (!m_pOGLProgram->_link())
		{
			::MessageBox(::AfxGetMainWnd()->GetSafeHwnd(), L"Program linking error!", L"Error", MB_ICONERROR | MB_OK);
		}

		m_matModelView = glm::identity<glm::mat4>();
	}	

	void _destroy()
	{
		m_oglBuffers.clear();

		if (m_pOGLContext != nullptr)
		{
			m_pOGLContext->makeCurrent();
		}		

		if (m_pOGLProgram != nullptr)
		{
			m_pOGLProgram->_detachShader(m_pVertexShader);
			m_pOGLProgram->_detachShader(m_pFragmentShader);

			delete m_pOGLProgram;
			m_pOGLProgram = nullptr;
		}		

		delete m_pVertexShader;
		m_pVertexShader = nullptr;

		delete m_pFragmentShader;
		m_pFragmentShader = nullptr;

		delete m_pOGLContext;
		m_pOGLContext = nullptr;
	}

	void _reset()
	{
		// Rotation
		m_fXAngle = 0.f;
		m_fYAngle = 0.f;
		m_fZAngle = 0.f;
		m_rotation = _quaterniond::toQuaternion(0., 0., 0.);

		// Translation
		m_fXTranslation = 0.0f;
		m_fYTranslation = 0.0f;
		m_fZTranslation = -5.0f;

		// TODO: all settings???
	}

	void _prepare(
		int iWidth, int iHeight,
		float& fXmin, float& fXmax, 
		float& fYmin, float& fYmax, 
		float& fZmin, float& fZmax)
	{
		BOOL bResult = m_pOGLContext->makeCurrent();
		VERIFY(bResult);

#ifdef _ENABLE_OPENGL_DEBUG
		m_pOGLContext->enableDebug();
#endif

		m_pOGLProgram->_use();

		glViewport(0, 0, iWidth, iHeight);

		glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set up the parameters
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LEQUAL);		

		// Projection Matrix
		// fovY     - Field of vision in degrees in the y direction
		// aspect   - Aspect ratio of the viewport
		// zNear    - The near clipping distance
		// zFar     - The far clipping distance
		GLdouble fovY = 45.0;
		GLdouble aspect = (GLdouble)iWidth / (GLdouble)iHeight;
		GLdouble zNear = 0.0001;
		GLdouble zFar = 1000.0;

		GLdouble fH = tan(fovY / 360 * M_PI) * zNear;
		GLdouble fW = fH * aspect;

		// Projection
		switch (m_enProjection)
		{
			case enumProjection::Perspective:
			{
				glm::mat4 matProjection = glm::frustum<GLdouble>(-fW, fW, -fH, fH, zNear, zFar);
				m_pOGLProgram->_setProjectionMatrix(matProjection);
			}
			break;

			case enumProjection::Orthographic:
			{
				glm::mat4 matProjection = glm::ortho<GLdouble>(-1.5, 1.5, -1.5, 1.5, zNear, zFar);
				m_pOGLProgram->_setProjectionMatrix(matProjection);
			}
			break;

			default:
			{
				ASSERT(FALSE);
			}
			break;
		}

		// Model-View Matrix
		m_matModelView = glm::identity<glm::mat4>();
		m_matModelView = glm::translate(m_matModelView, glm::vec3(m_fXTranslation, m_fYTranslation, m_fZTranslation));

		float fXTranslation = fXmin;
		fXTranslation += (fXmax - fXmin) / 2.f;
		fXTranslation = -fXTranslation;

		float fYTranslation = fYmin;
		fYTranslation += (fYmax - fYmin) / 2.f;
		fYTranslation = -fYTranslation;

		float fZTranslation = fZmin;
		fZTranslation += (fZmax - fZmin) / 2.f;
		fZTranslation = -fZTranslation;

		m_matModelView = glm::translate(m_matModelView, glm::vec3(-fXTranslation, -fYTranslation, -fZTranslation));

		if (m_enRotationMode == enumRotationMode::XY)
		{
			m_matModelView = glm::rotate(m_matModelView, glm::radians(m_fXAngle), glm::vec3(1.f, 0.f, 0.f));
			m_matModelView = glm::rotate(m_matModelView, glm::radians(m_fYAngle), glm::vec3(0.f, 1.f, 0.f));
			m_matModelView = glm::rotate(m_matModelView, glm::radians(m_fZAngle), glm::vec3(0.f, 0.f, 1.f));
		}
		else if (m_enRotationMode == enumRotationMode::XYZ)
		{
			// Apply rotation...
			_quaterniond rotation = _quaterniond::toQuaternion(glm::radians(m_fZAngle), glm::radians(m_fXAngle), glm::radians(m_fYAngle));
			m_rotation.cross(rotation);

			// ... and reset
			m_fXAngle = m_fYAngle = m_fZAngle = 0.f;

			const double* pRotationMatirx = m_rotation.toMatrix();
			glm::mat4 matTransformation = glm::make_mat4((GLdouble*)pRotationMatirx);
			delete pRotationMatirx;

			m_matModelView = m_matModelView * matTransformation;
		}
		else
		{
			ASSERT(FALSE);
		}

		m_matModelView = glm::translate(m_matModelView, glm::vec3(fXTranslation, fYTranslation, fZTranslation));
		m_pOGLProgram->_setModelViewMatrix(m_matModelView);

		// Normal Matrix
		glm::mat4 matNormal = m_matModelView;
		matNormal = glm::inverse(matNormal);
		matNormal = glm::transpose(matNormal);
		m_pOGLProgram->_setNormalMatrix(matNormal);

		// Model
		m_pOGLProgram->_enableBlinnPhongModel(true);
	}

	enumProjection _getProjection() const
	{
		return m_enProjection;
	}

	void _setProjection(enumProjection enProjection)
	{
		m_enProjection = enProjection;

		_setView(enumView::Front);
	}

	enumRotationMode _getRotationMode() const
	{
		return m_enRotationMode;
	}

	void _setRotationMode(enumRotationMode enRotationMode)
	{
		m_enRotationMode = enRotationMode;

		_setView(enumView::Front);
	}

	void _setView(enumView enView)
	{
		_reset();

		switch (enView)
		{
			case enumView::Front:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 270.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}				
			}
			break;

			case enumView::Back:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 90.f;
					m_fYAngle = 180.f;
					m_fZAngle = 0.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}
			}
			break;

			case enumView::Top:
			{
				m_fXAngle = 0.f;
				m_fYAngle = 0.f;
				m_fZAngle = 0.f;
			}
			break;

			case enumView::Bottom:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 180.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}
			}
			break;

			case enumView::Left:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 270.f;
					m_fYAngle = 0.f;
					m_fZAngle = 90.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}
			}
			break;

			case enumView::Right:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 270.f;
					m_fYAngle = 0.f;
					m_fZAngle = 270.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}				
			}
			break;

			case enumView::Isometric:
			{
				if (m_enRotationMode == enumRotationMode::XY)
				{
					m_fXAngle = 315.f;
					m_fYAngle = 0.f;
					m_fZAngle = 45.f;
				}
				else if (m_enRotationMode == enumRotationMode::XYZ)
				{
					m_fXAngle = 0.f;
					m_fYAngle = 0.f;
					m_fZAngle = 0.f;
				}
				else
				{
					ASSERT(FALSE);
				}
			}
			break;

			default:
			{
				ASSERT(FALSE);
			}
			break;
		} // switch (enView)
	}		

	void _rotateMouseLButton(float fXAngle, float fYAngle)
	{
		if (m_enRotationMode == enumRotationMode::XY)
		{
			if (abs(fXAngle) >= abs(fYAngle))
			{
				fYAngle = 0.;
			}
			else
			{
				if (abs(fYAngle) >= abs(fXAngle))
				{
					fXAngle = 0.;
				}
			}

			_rotate(
				fXAngle * ROTATION_SPEED,
				fYAngle * ROTATION_SPEED);
		}
		else if (m_enRotationMode == enumRotationMode::XYZ)
		{
			_rotate(
				-fYAngle * ROTATION_SPEED,
				-fXAngle * ROTATION_SPEED);
		}
		else
		{
			ASSERT(FALSE);
		}
	}

	void _zoomMouseMButton(LONG lDelta)
	{
		_zoom(
			lDelta > 0 ?
			-abs(m_fZTranslation) * ZOOM_SPEED_MOUSE :
			abs(m_fZTranslation) * ZOOM_SPEED_MOUSE);
	}

	void _panMouseRButton(float fX, float fY)
	{
		_pan(
			PAN_SPEED_MOUSE * fX,
			PAN_SPEED_MOUSE * -fY);
	}

	virtual void _onMouseWheel(UINT /*nFlags*/, short zDelta, CPoint /*pt*/)
	{
		_zoom(zDelta < 0 ?
			-abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL :
			abs(m_fZTranslation) * ZOOM_SPEED_MOUSE_WHEEL);
	}	

	virtual void _onKeyUp(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/)
	{
		CRect rcClient;
		m_pWnd->GetClientRect(&rcClient);

		switch (nChar)
		{
			case VK_UP:
			{
				_pan(
					0.f,
					PAN_SPEED_KEYS * (1.f / rcClient.Height()));
			}
			break;

			case VK_DOWN:
			{
				_pan(
					0.f,
					-(PAN_SPEED_KEYS * (1.f / rcClient.Height())));
			}
			break;

			case VK_LEFT:
			{
				_pan(
					-(PAN_SPEED_KEYS * (1.f / rcClient.Width())),
					0.f);
			}
			break;

			case VK_RIGHT:
			{
				_pan(
					PAN_SPEED_KEYS * (1.f / rcClient.Width()),
					0.f);
			}
			break;

			case VK_PRIOR:
			{
				_zoom(abs(m_fZTranslation) * ZOOM_SPEED_KEYS);
			}
			break;

			case VK_NEXT:
			{
				_zoom(-abs(m_fZTranslation) * ZOOM_SPEED_KEYS);
			}
			break;
		} // switch (nChar)
	}	

private: //  Methods

	void _rotate(float fXAngle, float fYAngle)
	{
		m_fXAngle += fXAngle * (180.f / (float)M_PI);
		if (m_fXAngle > 360.f)
		{
			m_fXAngle -= 360.f;
		}
		else if (m_fXAngle < -360.f)
		{
			m_fXAngle += 360.f;
		}

		m_fYAngle += fYAngle * (180.f / (float)M_PI);
		if (m_fYAngle > 360.f)
		{
			m_fYAngle = m_fYAngle - 360.f;
		}
		else if (m_fYAngle < -360.f)
		{
			m_fYAngle += 360.f;
		}

		_redraw();
	}

	void _zoom(float fZTranslation)
	{
		if (m_enProjection == enumProjection::Orthographic)
		{
			return;
		}

		m_fZTranslation += fZTranslation;

		_redraw();
	}

	void _pan(float fX, float fY)
	{
		m_fXTranslation += fX;
		m_fYTranslation += fY;

		_redraw();
	}
};