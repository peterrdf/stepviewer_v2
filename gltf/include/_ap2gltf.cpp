#include "_host.h"
#include "_ap2gltf.h"

#include "_base64.h"
#include "_dateTime.h"
#include "_ifc_geometry.h"
#include "_ifc_instance.h"
#include "_ifc_model.h"

// ************************************************************************************************
namespace _ap2gltf
{
	_exporter::_exporter(_model* pModel, const char* szOutputFile, bool bEmbeddedBuffers)
		: _log_client()
		, m_pModel(pModel)
		, m_bEmbeddedBuffers(bEmbeddedBuffers)
		, m_pPolygonsMaterial(nullptr)
		, m_vecMaterials()
		, m_mapMaterials()
		, m_mapImages()
		, m_vecNodes()
		, m_mapNodes()
		, m_vecSceneRootNodes()
		, m_strOutputFile("")
		, m_strOutputFolder("")
		, m_pOutputStream(nullptr)
		, m_iIndent(-1)
		, m_iBuffersCount(0)
		, m_iBufferViewsCount(0)
		, m_iMeshesCount(0)
	{
		VERIFY_POINTER(m_pModel);
		VERIFY_POINTER(szOutputFile);

		m_strOutputFile = szOutputFile;
		VERIFY_STLOBJ_IS_NOT_EMPTY(m_strOutputFile);

		fs::path pthOutputFile = szOutputFile;
		m_strOutputFolder = pthOutputFile.parent_path().string();
		VERIFY_STLOBJ_IS_NOT_EMPTY(m_strOutputFolder);

		m_pPolygonsMaterial = new _material();
		addMaterial(m_pPolygonsMaterial);
	}

	/*virtual*/ _exporter::~_exporter()
	{
		delete m_pPolygonsMaterial;

		for (auto pNode : m_vecNodes) {
			delete pNode;
		}

		delete getOutputStream();
	}

	void _exporter::execute()
	{
		if (!preExecute()) {
			getLog()->logWrite(enumLogEvent::error, "Invalid model.");
			return;
		}

		// {root
		indent()++;
		{
			writeStartObjectTag(false);

			indent()++;
			{
				// asset
				{
					writeAssetProperty();
					*getOutputStream() << COMMA;
				}

				// buffers
				{
					writeBuffersProperty();
					*getOutputStream() << COMMA;
				}

				// bufferViews
				{
					writeBufferViewsProperty();
					*getOutputStream() << COMMA;
				}

				// accessors
				{
					writeAccessorsProperty();
					*getOutputStream() << COMMA;
				}

				// meshes
				{
					writeMeshesProperty();
					*getOutputStream() << COMMA;
				}

				// nodes
				{
					writeNodesProperty();
					*getOutputStream() << COMMA;
				}

				// scene
				{
					writeSceneProperty();
					*getOutputStream() << COMMA;
				}

				// scenes
				{
					writeScenesProperty();
					*getOutputStream() << COMMA;
				}

				// materials
				{
					writeMaterials();
				}

				// images/textures
				{
					if (!m_mapImages.empty()) {
						*getOutputStream() << COMMA;
						writeImages();
						*getOutputStream() << COMMA;
						writeSamplers();
						*getOutputStream() << COMMA;
						writeTextures();
					}
				}
			}
			indent()--;

			writeEndObjectTag();
			indent()--;
		}
		// root}

		getLog()->logWrite(enumLogEvent::info, _string::format("Buffers count: %d", m_iBuffersCount));
		getLog()->logWrite(enumLogEvent::info, _string::format("Buffer Views count: %d", m_iBufferViewsCount));
		getLog()->logWrite(enumLogEvent::info, _string::format("Meshes count: %d", m_iMeshesCount));

		postExecute();
	}

	/*virtual*/ bool _exporter::createOuputStream()
	{
		if (getOutputStream() != nullptr) {
			delete getOutputStream();
		}

		m_pOutputStream = new ofstream(m_strOutputFile, std::ios::out | std::ios::trunc);
		std::locale loc(std::locale::classic());
		getOutputStream()->imbue(loc);

		return getOutputStream()->good();
	}

	/*virtual*/ bool _exporter::ignoreGeometry(_geometry* pGeometry)
	{
		assert(pGeometry != nullptr);

		if (!pGeometry->hasGeometry()) {
			return true;
		}

		wstring strEntity = _ptr<_ap_geometry>(pGeometry)->getEntityName();
		std::transform(strEntity.begin(), strEntity.end(), strEntity.begin(), ::towupper);

		return (strEntity == L"IFCSPACE") ||
			(strEntity == L"IFCRELSPACEBOUNDARY") ||
			(strEntity == L"IFCOPENINGELEMENT") ||
			(strEntity == L"IFCALIGNMENTVERTICAL") ||
			(strEntity == L"IFCALIGNMENTHORIZONTAL") ||
			(strEntity == L"IFCALIGNMENTSEGMENT") ||
			(strEntity == L"IFCALIGNMENTCANT");
	}

	/*virtual*/ bool _exporter::preExecute()
	{
		if (!createOuputStream()) {
			getLog()->logWrite(enumLogEvent::error, "Cannot create output stream.");
			return false;
		}

		for (auto pGeometry : m_pModel->getGeometries()) {
			if (!ignoreGeometry(pGeometry)) {
				auto pNode = new _node(pGeometry);
				m_vecNodes.push_back(pNode);

				assert(m_mapNodes.find(pGeometry) == m_mapNodes.end());
				m_mapNodes[pGeometry] = pNode;
			}
		}

		return !m_vecNodes.empty();
	}

	/*virtual*/ void _exporter::postExecute()
	{
		if (getOutputStream() != nullptr) {
			delete getOutputStream();
		}

		fs::path pthMetadataFile = m_strOutputFile;
		pthMetadataFile += ".json";

		m_pOutputStream = new ofstream(pthMetadataFile.string(), std::ios::out | std::ios::trunc);
		std::locale loc(std::locale::classic());
		getOutputStream()->imbue(loc);

		if (!getOutputStream()->good()) {
			getLog()->logWrite(enumLogEvent::error, "Cannot create output stream.");
			return;
		}

		// {root
		indent()++;
		{
			writeStartObjectTag(false);

			indent()++;
			{
				// Global metadata
				{
					writeMetadata();
					*getOutputStream() << COMMA;
				}

				// Properties
				{
					writeMetadataProperties();
					//*getOutputStream() << COMMA;
				}

				// Units
				{
					//writeMetadataProperties();
				}
			}
			indent()--;

			writeEndObjectTag();
			indent()--;
		}
		// root}
	}

	/*virtual*/ void _exporter::writeIndent()
	{
		for (int iTab = 0; iTab < m_iIndent; iTab++) {
			*getOutputStream() << TAB;
		}
	}

	void _exporter::writeStartObjectTag(bool bNewLine/* = true*/)
	{
		if (bNewLine) {
			*getOutputStream() << getNewLine();
			writeIndent();
		}

		*getOutputStream() << "{";
	}

	void _exporter::writeEndObjectTag()
	{
		*getOutputStream() << getNewLine();
		writeIndent();
		*getOutputStream() << "}";
	}

	void _exporter::writeStartArrayTag(bool bNewLine/* = true*/)
	{
		if (bNewLine) {
			*getOutputStream() << getNewLine();
			writeIndent();
		}

		*getOutputStream() << "[";
	}

	void _exporter::writeEndArrayTag()
	{
		*getOutputStream() << getNewLine();
		writeIndent();
		*getOutputStream() << "]";
	}

	string _exporter::buildStringProperty(const string& strName, const string& strValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);
		VERIFY_STLOBJ_IS_NOT_EMPTY(strValue);

		string strProperty;

		strProperty += DOULE_QUOT_MARK;
		strProperty += strName.c_str();
		strProperty += DOULE_QUOT_MARK;
		strProperty += COLON;
		strProperty += SPACE;
		strProperty += DOULE_QUOT_MARK;
		strProperty += strValue.c_str();
		strProperty += DOULE_QUOT_MARK;

		return strProperty;
	}

	string _exporter::buildNumberProperty(const string& strName, const string& strValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);
		VERIFY_STLOBJ_IS_NOT_EMPTY(strValue);

		string strProperty;

		strProperty += DOULE_QUOT_MARK;
		strProperty += strName.c_str();
		strProperty += DOULE_QUOT_MARK;
		strProperty += COLON;
		strProperty += SPACE;
		strProperty += strValue.c_str();

		return strProperty;
	}

	string _exporter::buildArrayProperty(const string& strName, const vector<string>& vecValues)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);
		VERIFY_STLOBJ_IS_NOT_EMPTY(vecValues);

		string strProperty;

		strProperty += DOULE_QUOT_MARK;
		strProperty += strName.c_str();
		strProperty += DOULE_QUOT_MARK;
		strProperty += COLON;
		strProperty += SPACE;

		strProperty += "[";
		for (size_t iIndex = 0; iIndex < vecValues.size(); iIndex++) {
			if (iIndex > 0) {
				strProperty += COMMA;
				strProperty += SPACE;
			}

			strProperty += vecValues[iIndex];
		}
		strProperty += "]";

		return strProperty;
	}

	void _exporter::writeStringProperty(const string& strName, const string& strValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strValue.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
	}

	void _exporter::writeIntProperty(const string& strName, int iValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << iValue;
	}

	void _exporter::writeUIntProperty(const string& strName, uint32_t iValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << iValue;
	}

	void _exporter::writeFloatProperty(const string& strName, float fValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << fValue;
	}

	void _exporter::writeDoubleProperty(const string& strName, double dValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << dValue;
	}

	void _exporter::writeBoolProperty(const string& strName, bool bValue)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;
		*getOutputStream() << (bValue ? "true" : "false");
	}

	void _exporter::writeObjectProperty(const string& strName, const vector<string>& vecProperties)
	{
		VERIFY_STLOBJ_IS_NOT_EMPTY(strName);
		VERIFY_STLOBJ_IS_NOT_EMPTY(vecProperties);

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << strName.c_str();
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;

		writeStartObjectTag(false);

		indent()++;
		for (size_t iIndex = 0; iIndex < vecProperties.size(); iIndex++) {
			if (iIndex > 0) {
				*getOutputStream() << COMMA;
			}

			*getOutputStream() << getNewLine();
			writeIndent();
			*getOutputStream() << vecProperties[iIndex].c_str();
		}
		indent()--;

		writeEndObjectTag();
	}

	void _exporter::writeAssetProperty()
	{
		vector<string> vecProperties;
		vecProperties.push_back(buildStringProperty(GENERATOR_PROP, GENERATOR));
		vecProperties.push_back(buildStringProperty(VERSION_PROP, VERSION));

		writeObjectProperty(ASSET_PROP, vecProperties);
	}

	/*virtual*/ void _exporter::writeBuffersProperty()
	{
		float fLengthConversionFactor = (float)getProjectUnitConversionFactor(
			_ptr<_ap_model>(m_pModel)->getSdaiModel(), "LENGTHUNIT", nullptr, nullptr, nullptr);

		_vector3d vecVertexBufferOffset;
		GetVertexBufferOffset(m_pModel->getOwlModel(), (double*)&vecVertexBufferOffset);

		float fScaleFactor = (float)m_pModel->getOriginalBoundingSphereDiameter() / 2.f;
		fScaleFactor *= fLengthConversionFactor;
		vecVertexBufferOffset.x *= fLengthConversionFactor;
		vecVertexBufferOffset.y *= fLengthConversionFactor;
		vecVertexBufferOffset.z *= fLengthConversionFactor;

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << BUFFERS_PROP;
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;

		writeStartArrayTag(false);

		for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
			auto pNode = m_vecNodes[iNodeIndex];
			auto pGeometry = pNode->getGeometry();

			if (pGeometry->isPlaceholder()) {
				continue;
			}

			const auto VERTEX_LENGTH = pGeometry->getVertexLength();

			_ptr<_ifc_geometry> ifcGeometry(pGeometry, false);
			bool bIsMappeditem = ifcGeometry && ifcGeometry->getIsMappedItem();

			// buffer: byteLength
			uint32_t iBufferByteLength = 0;

			// buffer: uri
			string strBinFileName = _string::sformat("%lld.bin", getGeometryID(pGeometry));
			fs::path pthNodeBinData = m_strOutputFolder;
			pthNodeBinData.append(strBinFileName);

			pNode->bufferBinFile() = strBinFileName;

			std::ostream* pNodeBinDataStream = nullptr;
			if (m_bEmbeddedBuffers) {
				pNodeBinDataStream = new std::stringstream();
			}
			else {
				pNodeBinDataStream = new std::ofstream();
				((std::ofstream*)pNodeBinDataStream)->open(pthNodeBinData.string(), std::ios::out | std::ios::binary | std::ios::trunc);
			}

			// vertices/POSITION
			for (int64_t iVertex = 0; iVertex < pGeometry->getVerticesCount(); iVertex++) {
				float fValue = pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 0] * fScaleFactor;
				fValue -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.x;
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&fValue), sizeof(float));

				fValue = pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 1] * fScaleFactor;
				fValue -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.y;
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&fValue), sizeof(float));

				fValue = pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 2] * fScaleFactor;
				fValue -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.z;
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&fValue), sizeof(float));
			}

			pNode->verticesBufferViewByteLength() = (uint32_t)pGeometry->getVerticesCount() * 3 * (uint32_t)sizeof(float);
			iBufferByteLength += pNode->verticesBufferViewByteLength();

			// vertices/NORMAL
			float arNormalizedVector[3] = { 0.f, 0.f, 0.f };
			for (int64_t iVertex = 0; iVertex < pGeometry->getVerticesCount(); iVertex++) {
				_normalizeVector(
					pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 3],
					pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 4],
					pGeometry->getVertices()[(iVertex * VERTEX_LENGTH) + 5],
					arNormalizedVector);
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&arNormalizedVector[0]), sizeof(float));
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&arNormalizedVector[1]), sizeof(float));
				pNodeBinDataStream->write(reinterpret_cast<const char*>(&arNormalizedVector[2]), sizeof(float));
			}

			pNode->normalsBufferViewByteLength() = (uint32_t)pGeometry->getVerticesCount() * 3 * (uint32_t)sizeof(float);
			iBufferByteLength += pNode->normalsBufferViewByteLength();

			// Conceptual faces/indices
			for (auto pCohort : pGeometry->concFacesCohorts()) {
				uint32_t iIndicesByteLength = (uint32_t)pCohort->indices().size() * (uint32_t)sizeof(GLuint);
				pNode->indicesBufferViewsByteLength().push_back(iIndicesByteLength);

				iBufferByteLength += iIndicesByteLength;

				for (size_t i = 0; i < pCohort->indices().size(); i++) {
					GLuint iIndexValue = pCohort->indices()[i];
					pNodeBinDataStream->write(reinterpret_cast<const char*>(&iIndexValue), sizeof(GLuint));
				}
			}

			// Conceptual faces polygons/indices
			for (auto pCohort : pGeometry->concFacePolygonsCohorts()) {
				uint32_t iIndicesByteLength = (uint32_t)pCohort->indices().size() * (uint32_t)sizeof(GLuint);
				pNode->indicesBufferViewsByteLength().push_back(iIndicesByteLength);

				iBufferByteLength += iIndicesByteLength;

				for (size_t i = 0; i < pCohort->indices().size(); i++) {
					GLuint iIndexValue = pCohort->indices()[i];
					pNodeBinDataStream->write(reinterpret_cast<const char*>(&iIndexValue), sizeof(GLuint));
				}
			}

			// Lines
			for (auto pCohort : pGeometry->linesCohorts()) {
				uint32_t iIndicesByteLength = (uint32_t)pCohort->indices().size() * (uint32_t)sizeof(GLuint);
				pNode->indicesBufferViewsByteLength().push_back(iIndicesByteLength);

				iBufferByteLength += iIndicesByteLength;

				for (size_t i = 0; i < pCohort->indices().size(); i++) {
					GLuint iIndexValue = pCohort->indices()[i];
					pNodeBinDataStream->write(reinterpret_cast<const char*>(&iIndexValue), sizeof(GLuint));
				}
			}

			// Points
			for (auto pCohort : pGeometry->pointsCohorts()) {
				uint32_t iIndicesByteLength = (uint32_t)pCohort->indices().size() * (uint32_t)sizeof(GLuint);
				pNode->indicesBufferViewsByteLength().push_back(iIndicesByteLength);

				iBufferByteLength += iIndicesByteLength;

				for (size_t i = 0; i < pCohort->indices().size(); i++) {
					GLuint iIndexValue = pCohort->indices()[i];
					pNodeBinDataStream->write(reinterpret_cast<const char*>(&iIndexValue), sizeof(GLuint));
				}
			}

			pNode->bufferByteLength() = iBufferByteLength;

			if (iNodeIndex > 0) {
				*getOutputStream() << COMMA;
			}

			indent()++;
			writeStartObjectTag();

			indent()++;
			writeUIntProperty("byteLength", iBufferByteLength);
			*getOutputStream() << COMMA;
			if (m_bEmbeddedBuffers) {
				std::string strBufferData = ((std::stringstream*)pNodeBinDataStream)->str();
				std::string strBase64BufferData = "data:application/octet-stream;base64,";
				strBase64BufferData += base64_encode(reinterpret_cast<const unsigned char*>(strBufferData.data()), (unsigned int)strBufferData.size());
				writeStringProperty("uri", strBase64BufferData);
			}
			else {
				writeStringProperty("uri", pthNodeBinData.stem().string() + ".bin");
			}
			indent()--;

			writeEndObjectTag();
			indent()--;

			delete pNodeBinDataStream;

			m_iBuffersCount++;
		} // for (for (size_t iIndex = ...

		writeEndArrayTag();
	}

	/*virtual*/ void _exporter::writeBufferViewsProperty()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << BUFFER_VIEWS_PROP;
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;

		writeStartArrayTag(false);

		// ARRAY_BUFFER/ELEMENT_ARRAY_BUFFER
		size_t iBufferIndex = 0;
		for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
			auto pNode = m_vecNodes[iNodeIndex];
			auto pGeometry = pNode->getGeometry();

			if (pGeometry->isPlaceholder()) {
				continue;
			}

			assert(pNode->indicesBufferViewsByteLength().size() ==
				pNode->getGeometry()->concFacesCohorts().size() +
				pNode->getGeometry()->concFacePolygonsCohorts().size() +
				pNode->getGeometry()->linesCohorts().size() +
				pNode->getGeometry()->pointsCohorts().size());

			if (iBufferIndex > 0) {
				*getOutputStream() << COMMA;
			}

			uint32_t iByteOffset = 0;

			// vertices/ARRAY_BUFFER/POSITION
			{
				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("buffer", (uint32_t)iBufferIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteLength", pNode->verticesBufferViewByteLength());
				*getOutputStream() << COMMA;
				writeIntProperty("byteOffset", iByteOffset);
				*getOutputStream() << COMMA;
				writeIntProperty("target", 34962/*ARRAY_BUFFER*/);
				indent()--;

				writeEndObjectTag();
				indent()--;

				iByteOffset += pNode->verticesBufferViewByteLength();
			}

			*getOutputStream() << COMMA;

			// normals/ARRAY_BUFFER/NORMAL
			{
				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("buffer", (uint32_t)iBufferIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteLength", pNode->normalsBufferViewByteLength());
				*getOutputStream() << COMMA;
				writeIntProperty("byteOffset", iByteOffset);
				*getOutputStream() << COMMA;
				writeIntProperty("target", 34962/*ARRAY_BUFFER*/);
				indent()--;

				writeEndObjectTag();
				indent()--;

				iByteOffset += pNode->normalsBufferViewByteLength();
			}

			// indices/ELEMENT_ARRAY_BUFFER
			for (size_t iIndicesBufferViewIndex = 0; iIndicesBufferViewIndex < pNode->indicesBufferViewsByteLength().size(); iIndicesBufferViewIndex++) {
				uint32_t iByteLength = pNode->indicesBufferViewsByteLength()[iIndicesBufferViewIndex];

				*getOutputStream() << COMMA;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("buffer", (uint32_t)iBufferIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteLength", iByteLength);
				*getOutputStream() << COMMA;
				writeIntProperty("byteOffset", iByteOffset);
				*getOutputStream() << COMMA;
				writeIntProperty("target", 34963/*ELEMENT_ARRAY_BUFFER*/);
				indent()--;

				writeEndObjectTag();
				indent()--;

				iByteOffset += iByteLength;
			} // for (size_t iIndicesBufferViewIndex = ...

			iBufferIndex++;
			m_iBufferViewsCount++;
		} // for (size_t iNodeIndex = ...

		writeEndArrayTag();
	}

	void _exporter::writeAccessorsProperty()
	{
		float fLengthConversionFactor = (float)getProjectUnitConversionFactor(
			_ptr<_ap_model>(m_pModel)->getSdaiModel(), "LENGTHUNIT", nullptr, nullptr, nullptr);

		_vector3d vecVertexBufferOffset;
		GetVertexBufferOffset(m_pModel->getOwlModel(), (double*)&vecVertexBufferOffset);

		float fScaleFactor = (float)m_pModel->getOriginalBoundingSphereDiameter() / 2.f;
		fScaleFactor *= fLengthConversionFactor;
		vecVertexBufferOffset.x *= fLengthConversionFactor;
		vecVertexBufferOffset.y *= fLengthConversionFactor;
		vecVertexBufferOffset.z *= fLengthConversionFactor;

		*getOutputStream() << getNewLine();
		writeIndent();

		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << ACCESSORS_PROP;
		*getOutputStream() << DOULE_QUOT_MARK;
		*getOutputStream() << COLON;
		*getOutputStream() << SPACE;

		writeStartArrayTag(false);

		int iBufferViewIndex = 0;
		for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
			auto pNode = m_vecNodes[iNodeIndex];
			auto pGeometry = pNode->getGeometry();

			if (pGeometry->isPlaceholder()) {
				continue;
			}

			assert(pNode->indicesBufferViewsByteLength().size() ==
				pGeometry->concFacesCohorts().size() +
				pGeometry->concFacePolygonsCohorts().size() +
				pGeometry->linesCohorts().size() +
				pGeometry->pointsCohorts().size());

			if (iNodeIndex > 0) {
				*getOutputStream() << COMMA;
			}

			// vertices/ARRAY_BUFFER/POSITION
			{
				_ptr<_ifc_geometry> ifcGeometry(pGeometry, false);
				bool bIsMappeditem = ifcGeometry && ifcGeometry->getIsMappedItem();

				float fXmin = FLT_MAX;
				float fXmax = -FLT_MAX;
				float fYmin = FLT_MAX;
				float fYmax = -FLT_MAX;
				float fZmin = FLT_MAX;
				float fZmax = -FLT_MAX;
				pGeometry->calculateVerticesMinMax(fXmin, fXmax, fYmin, fYmax, fZmin, fZmax);

				fXmin *= fScaleFactor;
				fXmin -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.x;

				fYmin *= fScaleFactor;
				fYmin -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.y;

				fZmin *= fScaleFactor;
				fZmin -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.z;

				fXmax *= fScaleFactor;
				fXmax -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.x;

				fYmax *= fScaleFactor;
				fYmax -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.y;

				fZmax *= fScaleFactor;
				fZmax -= bIsMappeditem ? 0.f : (float)vecVertexBufferOffset.z;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5126/*FLOAT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pGeometry->getVerticesCount());
				*getOutputStream() << COMMA;
				*getOutputStream() << getNewLine();
				writeIndent();
				*getOutputStream() << buildArrayProperty("min", vector<string>
				{
					_string::format("%.10g", fXmin),
						_string::format("%.10g", fYmin),
						_string::format("%.10g", fZmin),
				}).c_str();
				*getOutputStream() << COMMA;
				*getOutputStream() << getNewLine();
				writeIndent();
				*getOutputStream() << buildArrayProperty("max", vector<string>
				{
					_string::format("%.10g", fXmax),
						_string::format("%.10g", fYmax),
						_string::format("%.10g", fZmax),
				}).c_str();
				*getOutputStream() << COMMA;
				writeStringProperty("type", "VEC3");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}
			// vertices/ARRAY_BUFFER/POSITION

			*getOutputStream() << COMMA;

			// normals/ARRAY_BUFFER/NORMAL
			{
				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5126/*FLOAT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pGeometry->getVerticesCount());
				*getOutputStream() << COMMA;
				writeStringProperty("type", "VEC3");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}
			// normals/ARRAY_BUFFER/NORMAL

			/*
			* indices/ELEMENT_ARRAY_BUFFER
			*/

			// Conceptual faces
			for (auto pCohort : pGeometry->concFacesCohorts()) {
				*getOutputStream() << COMMA;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5125/*UNSIGNED_INT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pCohort->indices().size());
				*getOutputStream() << COMMA;
				writeStringProperty("type", "SCALAR");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}

			// Conceptual faces polygons
			for (auto pCohort : pGeometry->concFacePolygonsCohorts()) {
				*getOutputStream() << COMMA;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5125/*UNSIGNED_INT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pCohort->indices().size());
				*getOutputStream() << COMMA;
				writeStringProperty("type", "SCALAR");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}

			// Lines
			for (auto pCohort : pGeometry->linesCohorts()) {
				*getOutputStream() << COMMA;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5125/*UNSIGNED_INT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pCohort->indices().size());
				*getOutputStream() << COMMA;
				writeStringProperty("type", "SCALAR");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}

			// Points
			for (auto pCohort : pGeometry->pointsCohorts()) {
				*getOutputStream() << COMMA;

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeUIntProperty("bufferView", (uint32_t)iBufferViewIndex);
				*getOutputStream() << COMMA;
				writeUIntProperty("byteOffset", 0);
				*getOutputStream() << COMMA;
				writeUIntProperty("componentType", 5125/*UNSIGNED_INT*/);
				*getOutputStream() << COMMA;
				writeUIntProperty("count", (uint32_t)pCohort->indices().size());
				*getOutputStream() << COMMA;
				writeStringProperty("type", "SCALAR");
				indent()--;

				writeEndObjectTag();
				indent()--;

				pNode->accessors().push_back(iBufferViewIndex);

				iBufferViewIndex++;
			}
		} // for (size_t iNodeIndex = ...

		writeEndArrayTag();
	}

	void _exporter::writeMeshesProperty()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// meshes
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << MESHES_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			writeStartArrayTag(false);

			uint32_t iMeshIndex = 0;
			for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
				auto pNode = m_vecNodes[iNodeIndex];
				auto pGeometry = pNode->getGeometry();

				if (pGeometry->isPlaceholder()) {
					continue;
				}

				assert(pNode->accessors().size() ==
					2/*vertices & normals bufferView-s*/ +
					pGeometry->concFacesCohorts().size() +
					pGeometry->concFacePolygonsCohorts().size() +
					pGeometry->linesCohorts().size() +
					pGeometry->pointsCohorts().size());

				// Conceptual faces
				for (size_t iConcFacesCohortIndex = 0; iConcFacesCohortIndex < pGeometry->concFacesCohorts().size(); iConcFacesCohortIndex++) {
					if (iMeshIndex > 0) {
						*getOutputStream() << COMMA;
					}

					// mesh
					{
						indent()++;
						writeStartObjectTag();

						indent()++;
						/*writeStringProperty("name", _string::sformat("%lld-conceptual-face-%lld", getGeometryID(pGeometry), iConcFacesCohortIndex));
						*getOutputStream() << COMMA;*/

						*getOutputStream() << getNewLine();
						writeIndent();

						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << "primitives";
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << COLON;
						*getOutputStream() << SPACE;

						writeStartArrayTag(false);

						// attributes
						{
							indent()++;
							writeStartObjectTag();

							// POSITION, NORMAL, etc.
							{
								indent()++;

								auto pMaterial = pGeometry->concFacesCohorts()[iConcFacesCohortIndex]->getMaterial();
								size_t iMaterialIndex = addMaterial(pMaterial);

								vector<string> vecAttributes;
								vecAttributes.push_back(buildNumberProperty("POSITION", to_string(pNode->accessors()[0])));
								vecAttributes.push_back(buildNumberProperty("NORMAL", to_string(pNode->accessors()[1])));
								if (pMaterial->hasTexture()) {
									vecAttributes.push_back(buildNumberProperty("TEXCOORD_0", to_string(pNode->accessors()[2])));
								}
								writeObjectProperty("attributes", vecAttributes);

								*getOutputStream() << COMMA;
								writeUIntProperty("indices", pNode->accessors()[
									iConcFacesCohortIndex +
										2/*skip vertices & normals accessor-s*/]);
								*getOutputStream() << COMMA;
								writeUIntProperty("material", (uint32_t)iMaterialIndex);
								*getOutputStream() << COMMA;
								writeUIntProperty("mode", 4/*TRIANGLES*/);

								indent()--;
							}

							writeEndObjectTag();
							indent()--;
						}
						// attributes

						writeEndArrayTag();
						indent()--;

						writeEndObjectTag();
						indent()--;
					}
					// mesh					

					pNode->meshes().push_back(iMeshIndex);

					iMeshIndex++;

					m_iMeshesCount++;
				} // for (size_t iConcFacesCohortIndex = ...

				// Conceptual faces polygons
				for (size_t iConcFacePolygonsCohortIndex = 0; iConcFacePolygonsCohortIndex < pGeometry->concFacePolygonsCohorts().size(); iConcFacePolygonsCohortIndex++) {
					if (iMeshIndex > 0) {
						*getOutputStream() << COMMA;
					}

					// mesh
					{
						indent()++;
						writeStartObjectTag();

						indent()++;
						/*writeStringProperty("name", _string::sformat("%lld-conceptual-face-polygons-%lld", getGeometryID(pGeometry), iConcFacePolygonsCohortIndex));
						*getOutputStream() << COMMA;*/

						*getOutputStream() << getNewLine();
						writeIndent();

						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << "primitives";
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << COLON;
						*getOutputStream() << SPACE;

						writeStartArrayTag(false);

						// attributes
						{
							indent()++;
							writeStartObjectTag();

							// POSITION
							{
								indent()++;

								vector<string> vecAttributes;
								vecAttributes.push_back(buildNumberProperty("POSITION", to_string(pNode->accessors()[0])));
								writeObjectProperty("attributes", vecAttributes);

								*getOutputStream() << COMMA;
								writeUIntProperty("indices", pNode->accessors()[
									iConcFacePolygonsCohortIndex +
										2/*skip vertices & normals accessor-s*/ +
										pGeometry->concFacesCohorts().size()]);
								*getOutputStream() << COMMA;
								writeUIntProperty("material", 0);
								*getOutputStream() << COMMA;
								writeUIntProperty("mode", 1/*LINES*/);

								indent()--;
							}

							writeEndObjectTag();
							indent()--;
						}
						// attributes

						writeEndArrayTag();
						indent()--;

						writeEndObjectTag();
						indent()--;
					}
					// mesh					

					pNode->meshes().push_back(iMeshIndex);

					iMeshIndex++;

					m_iMeshesCount++;
				} // for (size_t iConcFacePolygonsCohortIndex = ...

				// Lines
				for (size_t iLinesCohortIndex = 0; iLinesCohortIndex < pGeometry->linesCohorts().size(); iLinesCohortIndex++) {
					if (iMeshIndex > 0) {
						*getOutputStream() << COMMA;
					}

					// mesh
					{
						indent()++;
						writeStartObjectTag();

						indent()++;
						/*writeStringProperty("name", _string::sformat("%lld-lines-%lld", getGeometryID(pGeometry), iLinesCohortIndex));
						*getOutputStream() << COMMA;*/

						*getOutputStream() << getNewLine();
						writeIndent();

						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << "primitives";
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << COLON;
						*getOutputStream() << SPACE;

						writeStartArrayTag(false);

						// attributes
						{
							indent()++;
							writeStartObjectTag();

							// POSITION
							{
								indent()++;

								auto pMaterial = pGeometry->linesCohorts()[iLinesCohortIndex]->getMaterial();
								size_t iMaterialIndex = addMaterial(pMaterial);

								vector<string> vecAttributes;
								vecAttributes.push_back(buildNumberProperty("POSITION", to_string(pNode->accessors()[0])));
								writeObjectProperty("attributes", vecAttributes);

								*getOutputStream() << COMMA;
								writeUIntProperty("indices", pNode->accessors()[
									iLinesCohortIndex +
										2/*skip vertices & normals accessor-s*/ +
										pGeometry->concFacesCohorts().size() +
										pGeometry->concFacePolygonsCohorts().size()]);
								*getOutputStream() << COMMA;
								writeUIntProperty("material", (uint32_t)iMaterialIndex);
								*getOutputStream() << COMMA;
								writeUIntProperty("mode", 1/*LINES*/);

								indent()--;
							}

							writeEndObjectTag();
							indent()--;
						}
						// attributes

						writeEndArrayTag();
						indent()--;

						writeEndObjectTag();
						indent()--;
					}
					// mesh					

					pNode->meshes().push_back(iMeshIndex);

					iMeshIndex++;

					m_iMeshesCount++;
				} // for (size_t iLinesCohortIndex = ...

				// Points
				for (size_t iPointsCohortIndex = 0; iPointsCohortIndex < pGeometry->pointsCohorts().size(); iPointsCohortIndex++) {
					if (iMeshIndex > 0) {
						*getOutputStream() << COMMA;
					}

					// mesh
					{
						indent()++;
						writeStartObjectTag();

						indent()++;
						*getOutputStream() << getNewLine();
						writeIndent();
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << "primitives";
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << COLON;
						*getOutputStream() << SPACE;

						writeStartArrayTag(false);

						// attributes
						{
							indent()++;
							writeStartObjectTag();

							// POSITION
							{
								indent()++;

								auto pMaterial = pGeometry->pointsCohorts()[iPointsCohortIndex]->getMaterial();
								size_t iMaterialIndex = addMaterial(pMaterial);

								vector<string> vecAttributes;
								vecAttributes.push_back(buildNumberProperty("POSITION", to_string(pNode->accessors()[0])));
								writeObjectProperty("attributes", vecAttributes);

								*getOutputStream() << COMMA;
								writeUIntProperty("indices", pNode->accessors()[
									iPointsCohortIndex +
										2/*skip vertices & normals accessor-s*/ +
										pGeometry->concFacesCohorts().size() +
										pGeometry->concFacePolygonsCohorts().size() +
										pGeometry->linesCohorts().size()]);
								*getOutputStream() << COMMA;
								writeUIntProperty("material", (uint32_t)iMaterialIndex);
								*getOutputStream() << COMMA;
								writeUIntProperty("mode", 0/*POINTS*/);

								indent()--;
							}

							writeEndObjectTag();
							indent()--;
						}
						// attributes

						writeEndArrayTag();
						indent()--;

						writeEndObjectTag();
						indent()--;
					}
					// mesh					

					pNode->meshes().push_back(iMeshIndex);

					iMeshIndex++;

					m_iMeshesCount++;
				} // for (size_t iPointsCohortIndex = ...
			} // for (size_t iNodeIndex = ...

			writeEndArrayTag();
		}
		// meshes
	}

	void _exporter::writeNodesProperty()
	{
		float fLengthConversionFactor = (float)getProjectUnitConversionFactor(
			_ptr<_ap_model>(m_pModel)->getSdaiModel(), "LENGTHUNIT", nullptr, nullptr, nullptr);

		_vector3d vecVertexBufferOffset;
		GetVertexBufferOffset(m_pModel->getOwlModel(), (double*)&vecVertexBufferOffset);

		float fScaleFactor = (float)m_pModel->getOriginalBoundingSphereDiameter() / 2.f;
		fScaleFactor *= fLengthConversionFactor;
		vecVertexBufferOffset.x *= fLengthConversionFactor;
		vecVertexBufferOffset.y *= fLengthConversionFactor;
		vecVertexBufferOffset.z *= fLengthConversionFactor;

		_matrix4x4 mtxDefaultViewTransformation;
		_matrix4x4Identity(&mtxDefaultViewTransformation);
		_matrixRotateByEulerAngles4x4(&mtxDefaultViewTransformation, 2 * PI * -90. / 360., 0, 0);

		*getOutputStream() << getNewLine();
		writeIndent();

		// nodes
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << NODES_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			writeStartArrayTag(false);

			uint32_t iSceneNodeIndex = 0;
			for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
				auto pNode = m_vecNodes[iNodeIndex];
				auto pGeometry = pNode->getGeometry();
				_ptr<_ap_geometry> apGeometry(pGeometry);

				_ptr<_ifc_geometry> ifcGeometry(pGeometry, false);
				if (ifcGeometry && ifcGeometry->getIsMappedItem()) {
					continue;
				}

				assert(pNode->meshes().size() ==
					pGeometry->concFacesCohorts().size() +
					pGeometry->concFacePolygonsCohorts().size() +
					pGeometry->linesCohorts().size() +
					pGeometry->pointsCohorts().size());
				assert(!pGeometry->getInstances().empty());

				if (pGeometry->isPlaceholder()) {
					assert(ifcGeometry);
					assert(ifcGeometry->getInstances().size() == 1);

					vector<string> vecPlaceholderNodeChildren;

					auto pPlaceholderInstance = ifcGeometry->getInstances().front();
					assert(pPlaceholderInstance != nullptr);

					// Transformations
					for (auto pMappedGeometry : ifcGeometry->getMappedGeometries()) {
						for (auto pInstance : pMappedGeometry->getInstances()) {
							_ptr<_ifc_instance> ifcInstance(pInstance);
							if (ifcInstance->getOwner() != pPlaceholderInstance) {
								continue;
							}

							auto pTransformation = pInstance->getTransformationMatrix();

							if (iSceneNodeIndex > 0) {
								*getOutputStream() << COMMA;
							}

							vecPlaceholderNodeChildren.push_back(to_string(iSceneNodeIndex++));

							auto itMappedItem = m_mapNodes.find(pMappedGeometry);
							assert(itMappedItem != m_mapNodes.end());
							auto pMappedNode = itMappedItem->second;

							// Mapped Item
							{
								vector<string> vecNodeChildren;
								for (size_t iMeshIndex = 0; iMeshIndex < pMappedNode->meshes().size(); iMeshIndex++) {
									vecNodeChildren.push_back(to_string(iSceneNodeIndex++));
								}

								indent()++;
								writeStartObjectTag();

								indent()++;
								*getOutputStream() << getNewLine();
								writeIndent();
								*getOutputStream() << buildArrayProperty("children", vecNodeChildren).c_str();
								if (pTransformation != nullptr) {
									_matrix4x4 mtxInstanceTransformation;
									memcpy(&mtxInstanceTransformation, pTransformation, sizeof(_matrix4x4));
									mtxInstanceTransformation._41 = (pTransformation->_41 * fScaleFactor) - vecVertexBufferOffset.x;
									mtxInstanceTransformation._42 = (pTransformation->_42 * fScaleFactor) - vecVertexBufferOffset.y;
									mtxInstanceTransformation._43 = (pTransformation->_43 * fScaleFactor) - vecVertexBufferOffset.z;

									_matrix4x4 mtxTransformation;
									_matrix4x4Multiply(&mtxTransformation, &mtxInstanceTransformation, &mtxDefaultViewTransformation);

									*getOutputStream() << COMMA;
									*getOutputStream() << getNewLine();
									writeIndent();
									*getOutputStream() << buildArrayProperty("matrix", vector<string>
									{
										to_string(mtxTransformation._11),
											to_string(mtxTransformation._12),
											to_string(mtxTransformation._13),
											to_string(mtxTransformation._14),
											to_string(mtxTransformation._21),
											to_string(mtxTransformation._22),
											to_string(mtxTransformation._23),
											to_string(mtxTransformation._24),
											to_string(mtxTransformation._31),
											to_string(mtxTransformation._32),
											to_string(mtxTransformation._33),
											to_string(mtxTransformation._34),
											to_string(mtxTransformation._41),
											to_string(mtxTransformation._42),
											to_string(mtxTransformation._43),
											to_string(mtxTransformation._44)
									}).c_str();
								} // if (pTransformation != nullptr)	
								else {
									*getOutputStream() << COMMA;
									*getOutputStream() << getNewLine();
									writeIndent();
									*getOutputStream() << buildArrayProperty("matrix", vector<string>
									{
										to_string(mtxDefaultViewTransformation._11),
											to_string(mtxDefaultViewTransformation._12),
											to_string(mtxDefaultViewTransformation._13),
											to_string(mtxDefaultViewTransformation._14),
											to_string(mtxDefaultViewTransformation._21),
											to_string(mtxDefaultViewTransformation._22),
											to_string(mtxDefaultViewTransformation._23),
											to_string(mtxDefaultViewTransformation._24),
											to_string(mtxDefaultViewTransformation._31),
											to_string(mtxDefaultViewTransformation._32),
											to_string(mtxDefaultViewTransformation._33),
											to_string(mtxDefaultViewTransformation._34),
											to_string(mtxDefaultViewTransformation._41),
											to_string(mtxDefaultViewTransformation._42),
											to_string(mtxDefaultViewTransformation._43),
											to_string(mtxDefaultViewTransformation._44)
									}).c_str();
								}
								indent()--;

								writeEndObjectTag();
								indent()--;
							}
							// Mapped Item

							// children
							{
								for (size_t iMeshIndex = 0; iMeshIndex < pMappedNode->meshes().size(); iMeshIndex++) {
									*getOutputStream() << COMMA;

									indent()++;
									writeStartObjectTag();

									indent()++;
									writeUIntProperty("mesh", pMappedNode->meshes()[iMeshIndex]);
									indent()--;

									writeEndObjectTag();
									indent()--;
								}
							}
							// children
						} // for (size_t iTransformation = ...
					} // for (auto pMappedGeometry : ...

					// Placeholder root
					{
						if (iSceneNodeIndex > 0) {
							*getOutputStream() << COMMA;
						}

						m_vecSceneRootNodes.push_back(iSceneNodeIndex);

						indent()++;
						writeStartObjectTag();
						char* szGlobalId = nullptr;
						sdaiGetAttrBN(apGeometry->getSdaiInstance(), "GlobalId", sdaiSTRING, &szGlobalId);
						assert(szGlobalId != nullptr);

						indent()++;
						writeStringProperty("name", szGlobalId != nullptr ? szGlobalId : "$");
						*getOutputStream() << COMMA;
						*getOutputStream() << getNewLine();
						writeIndent();
						*getOutputStream() << buildArrayProperty("children", vecPlaceholderNodeChildren).c_str();
						indent()--;
						writeEndObjectTag();
						indent()--;
					}
					// Placeholder root

					// next root
					iSceneNodeIndex++;
				} // if (pGeometry->isPlaceholder())
				else {
					// Transformations
					for (auto pInstance : pGeometry->getInstances()) {
						auto pTransformation = pInstance->getTransformationMatrix();

						if (iSceneNodeIndex > 0) {
							*getOutputStream() << COMMA;
						}

						// root	
						{
							m_vecSceneRootNodes.push_back(iSceneNodeIndex);

							vector<string> vecNodeChildren;
							for (size_t iMeshIndex = 0; iMeshIndex < pNode->meshes().size(); iMeshIndex++) {
								vecNodeChildren.push_back(to_string(++iSceneNodeIndex));
							}

							indent()++;
							writeStartObjectTag();

							char* szGlobalId = nullptr;
							sdaiGetAttrBN(apGeometry->getSdaiInstance(), "GlobalId", sdaiSTRING, &szGlobalId);
							//assert(szGlobalId != nullptr); //#todo STEP file may contain entities with no GlobalId attribute assigned

							indent()++;
							writeStringProperty("name", szGlobalId != nullptr ? szGlobalId : "$");
							*getOutputStream() << COMMA;
							*getOutputStream() << getNewLine();
							writeIndent();
							*getOutputStream() << buildArrayProperty("children", vecNodeChildren).c_str();
							if (pTransformation != nullptr) {
								_matrix4x4 mtxInstanceTransformation;
								memcpy(&mtxInstanceTransformation, pTransformation, sizeof(_matrix4x4));
								mtxInstanceTransformation._41 = (pTransformation->_41 * fScaleFactor) - vecVertexBufferOffset.x;
								mtxInstanceTransformation._42 = (pTransformation->_42 * fScaleFactor) - vecVertexBufferOffset.y;
								mtxInstanceTransformation._43 = (pTransformation->_43 * fScaleFactor) - vecVertexBufferOffset.z;

								_matrix4x4 mtxTransformation;
								_matrix4x4Multiply(&mtxTransformation, &mtxInstanceTransformation, &mtxDefaultViewTransformation);

								*getOutputStream() << COMMA;
								*getOutputStream() << getNewLine();
								writeIndent();
								*getOutputStream() << buildArrayProperty("matrix", vector<string>
								{
									to_string(mtxTransformation._11),
										to_string(mtxTransformation._12),
										to_string(mtxTransformation._13),
										to_string(mtxTransformation._14),
										to_string(mtxTransformation._21),
										to_string(mtxTransformation._22),
										to_string(mtxTransformation._23),
										to_string(mtxTransformation._24),
										to_string(mtxTransformation._31),
										to_string(mtxTransformation._32),
										to_string(mtxTransformation._33),
										to_string(mtxTransformation._34),
										to_string(mtxTransformation._41),
										to_string(mtxTransformation._42),
										to_string(mtxTransformation._43),
										to_string(mtxTransformation._44)
								}).c_str();
							} // if (pTransformation != nullptr)	
							else {
								*getOutputStream() << COMMA;
								*getOutputStream() << getNewLine();
								writeIndent();
								*getOutputStream() << buildArrayProperty("matrix", vector<string>
								{
									to_string(mtxDefaultViewTransformation._11),
										to_string(mtxDefaultViewTransformation._12),
										to_string(mtxDefaultViewTransformation._13),
										to_string(mtxDefaultViewTransformation._14),
										to_string(mtxDefaultViewTransformation._21),
										to_string(mtxDefaultViewTransformation._22),
										to_string(mtxDefaultViewTransformation._23),
										to_string(mtxDefaultViewTransformation._24),
										to_string(mtxDefaultViewTransformation._31),
										to_string(mtxDefaultViewTransformation._32),
										to_string(mtxDefaultViewTransformation._33),
										to_string(mtxDefaultViewTransformation._34),
										to_string(mtxDefaultViewTransformation._41),
										to_string(mtxDefaultViewTransformation._42),
										to_string(mtxDefaultViewTransformation._43),
										to_string(mtxDefaultViewTransformation._44)
								}).c_str();
							}
							indent()--;

							writeEndObjectTag();
							indent()--;
						}
						// root

						// children
						{
							for (size_t iMeshIndex = 0; iMeshIndex < pNode->meshes().size(); iMeshIndex++) {
								*getOutputStream() << COMMA;

								indent()++;
								writeStartObjectTag();

								indent()++;
								writeUIntProperty("mesh", pNode->meshes()[iMeshIndex]);
								indent()--;

								writeEndObjectTag();
								indent()--;
							}
						}
						// children

						// next root
						iSceneNodeIndex++;
					} //for (size_t iTransformation = ...
				} // else if (pGeometry->isPlaceholder())
			} // for (size_t iNodeIndex = ...

			writeEndArrayTag();
		}
		// nodes
	}

	void _exporter::writeSceneProperty()
	{
		writeIntProperty(SCENE_PROP, 0);
	}

	void _exporter::writeScenesProperty()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// scenes
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << SCENES_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			// [
			{
				writeStartArrayTag(false);

				indent()++;
				writeStartObjectTag();

				vector<string> vecRootNodes;
				for (size_t iRootNodeIndex = 0; iRootNodeIndex < m_vecSceneRootNodes.size(); iRootNodeIndex++) {
					vecRootNodes.push_back(to_string(m_vecSceneRootNodes[iRootNodeIndex]));
				}

				*getOutputStream() << getNewLine();
				indent()++;
				writeIndent();
				*getOutputStream() << buildArrayProperty("nodes", vecRootNodes).c_str();
				indent()--;

				writeEndObjectTag();
				indent()--;

				writeEndArrayTag();
			}
			// ]			
		}
		// scenes
	}

	void _exporter::writeMaterials()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// materials
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << MATERIALS_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			// [
			{
				writeStartArrayTag(false);

				for (size_t iMaterialIndex = 0; iMaterialIndex < m_vecMaterials.size(); iMaterialIndex++) {
					auto pMaterial = m_vecMaterials[iMaterialIndex];

					if (iMaterialIndex > 0) {
						*getOutputStream() << COMMA;
					}

					if (pMaterial->hasTexture()) {
						uint32_t iImageIndex = (uint32_t)-1;

						wstring strTexture = pMaterial->texture();

						// \ => /
						size_t iIndex = 0;
						while (true) {
							iIndex = strTexture.find(L"\\", iIndex);
							if (iIndex == string::npos) {
								break;
							}

							strTexture.replace(iIndex, wcslen(L"\\"), L"/");

							iIndex += wcslen(L"/");
						}

						auto itImage = m_mapImages.find(strTexture);
						if (itImage == m_mapImages.end()) {
							iImageIndex = (uint32_t)m_mapImages.size();
							m_mapImages[strTexture] = iImageIndex;
						}
						else {
							iImageIndex = itImage->second;
						}

						// texture
						{
							indent()++;
							writeStartObjectTag();

							indent()++;
							*getOutputStream() << getNewLine();
							writeIndent();
							*getOutputStream() << DOULE_QUOT_MARK;
							*getOutputStream() << "pbrMetallicRoughness";
							*getOutputStream() << DOULE_QUOT_MARK;
							*getOutputStream() << COLON;
							*getOutputStream() << SPACE;

							// pbrMetallicRoughness
							writeStartObjectTag();

							// baseColorTexture
							{
								indent()++;
								writeObjectProperty("baseColorTexture", vector<string>{ buildNumberProperty("index", to_string(iImageIndex)) });
								indent()--;
							}
							// baseColorTexture

							writeEndObjectTag();
							indent()--;

							*getOutputStream() << COMMA;

							// doubleSided
							{
								indent()++;
								writeBoolProperty("doubleSided", true);
								indent()--;
							}
							// doubleSided

							writeEndObjectTag();
							indent()--;
							// pbrMetallicRoughness							
						}
						// texture
					}
					else {
						// material					
						indent()++;
						writeStartObjectTag();

						// pbrMetallicRoughness
						indent()++;
						*getOutputStream() << getNewLine();
						writeIndent();
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << "pbrMetallicRoughness";
						*getOutputStream() << DOULE_QUOT_MARK;
						*getOutputStream() << COLON;
						*getOutputStream() << SPACE;

						// baseColorFactor						
						{
							writeStartObjectTag();

							*getOutputStream() << getNewLine();
							indent()++;
							writeIndent();
							*getOutputStream() << buildArrayProperty("baseColorFactor", vector<string>
							{
								to_string(pMaterial->getAmbientColor().r()),
									to_string(pMaterial->getAmbientColor().g()),
									to_string(pMaterial->getAmbientColor().b()),
									to_string(pMaterial->getA()),
							}).c_str();
							indent()--;
							writeEndObjectTag();
						}
						// baseColorFactor

						*getOutputStream() << COMMA;

						// emissiveFactor
						{
							*getOutputStream() << getNewLine();
							writeIndent();
							*getOutputStream() << buildArrayProperty("emissiveFactor", vector<string>
							{
								to_string(pMaterial->getEmissiveColor().r()),
									to_string(pMaterial->getEmissiveColor().g()),
									to_string(pMaterial->getEmissiveColor().b())
							}).c_str();
						}
						// emissiveFactor					

						*getOutputStream() << COMMA;

						// doubleSided
						{
							writeBoolProperty("doubleSided", true);
						}
						// doubleSided

						// alphaMode: OPAQUE/BLEND
						{
							if (pMaterial->getA() < 1.f) {
								*getOutputStream() << COMMA;
								writeStringProperty("alphaMode", "BLEND");
							}
						}
						// alphaMode: OPAQUE/BLEND

						indent()--;
						writeEndObjectTag();

						indent()--;
						// material
					}
				} // for (size_t iMaterialIndex = ...

				writeEndArrayTag();
			}
			// ]			
		}
		// materials
	}

	void _exporter::writeImages()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// images
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << IMAGES_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			// [
			{
				writeStartArrayTag(false);

				map<uint32_t, wstring> mapImageURIs;
				for (auto itImage : m_mapImages) {
					mapImageURIs[itImage.second] = itImage.first;
				}

				uint32_t iIndex = 0;
				for (auto itImageURI : mapImageURIs) {
					if (iIndex++ > 0) {
						*getOutputStream() << COMMA;
					}

					// image
					{
						indent()++;
						writeStartObjectTag();

						// uri
						{
							indent()++;
							writeStringProperty("uri", (const char*)CW2A(itImageURI.second.c_str()));
							indent()--;
						}

						writeEndObjectTag();
						indent()--;
					}
					// image
				}

				writeEndArrayTag();
			}
			// ]
		}
		// images
	}

	void _exporter::writeSamplers()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// samplers
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << SAMPLERS_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			// [
			{
				writeStartArrayTag(false);

				*getOutputStream() << getNewLine();
				writeIndent();
				*getOutputStream() << "{}";

				writeEndArrayTag();
			}
			// ]
		}
		// samplers
	}

	void _exporter::writeTextures()
	{
		*getOutputStream() << getNewLine();
		writeIndent();

		// images
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << TEXTURES_PROP;
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			// [
			{
				writeStartArrayTag(false);

				map<uint32_t, wstring> mapTextureSource;
				for (auto itImage : m_mapImages) {
					mapTextureSource[itImage.second] = itImage.first;
				}

				uint32_t iIndex = 0;
				for (auto itTextureSource : mapTextureSource) {
					if (iIndex++ > 0) {
						*getOutputStream() << COMMA;
					}

					// image
					{
						indent()++;
						writeStartObjectTag();

						// sampler, source
						{
							indent()++;
							writeUIntProperty("sampler", 0);
							*getOutputStream() << COMMA;
							writeUIntProperty("source", itTextureSource.first);
							indent()--;
						}

						writeEndObjectTag();
						indent()--;
					}
					// image
				}

				writeEndArrayTag();
			}
			// ]
		}
		// images
	}

	size_t _exporter::addMaterial(const _material* pMaterial)
	{
		VERIFY_POINTER(pMaterial);

		auto itMaterial = m_mapMaterials.find(*pMaterial);
		if (itMaterial != m_mapMaterials.end()) {
			return itMaterial->second;
		}

		size_t iMaterialIndex = m_vecMaterials.size();

		m_mapMaterials.insert(pair<_material, size_t>(*pMaterial, iMaterialIndex));
		m_vecMaterials.push_back(pMaterial);

		return iMaterialIndex;
	}

	void _exporter::writeMetadata()
	{
		_ptr<_ap_model> apModel(m_pModel);

		char* szFileSchema = 0;
		GetSPFFHeaderItem(apModel->getSdaiModel(), 9, 0, sdaiSTRING, (char**)&szFileSchema);

		char* szProjectGlobalId = nullptr;
		SdaiAggr sdaiAggr = sdaiGetEntityExtentBN(apModel->getSdaiModel(), "IFCPROJECT");
		SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
		if (iMembersCount > 0) {
			SdaiInstance sdaiProjectInstance = 0;
			engiGetAggrElement(sdaiAggr, 0, sdaiINSTANCE, &sdaiProjectInstance);
			sdaiGetAttrBN(sdaiProjectInstance, "GlobalId", sdaiSTRING, &szProjectGlobalId);
			assert(szProjectGlobalId != nullptr);
		}

		writeStringProperty("id", "0001"); //#todo
		writeStringProperty("projectId", szProjectGlobalId != nullptr ? szProjectGlobalId : "$");
		writeStringProperty("createdAt", _dateTime::iso8601DateTimeStamp());
		writeStringProperty("schema", szFileSchema != nullptr ? szFileSchema : "$");
		writeStringProperty("creatingApplication", "STEP2glTF Convertor 1.0, RDF LTD"); //#todo
	}

	void _exporter::writeMetadataProperties()
	{
		_ptr<_ifc_model> ifcModel(m_pModel, false);
		if (!ifcModel) {
			return;
		}

		if (ifcModel == nullptr) {
			return;
		}

		auto pPropertyProvider = ifcModel->getPropertyProvider();
		if (pPropertyProvider == nullptr) {
			return;
		}

		//
		// Collect property sets & properties
		//

		int iPropertySetIndex = 0;
		map<SdaiInstance, pair<_ifc_property_set*, int>> mapPropertySets;

		int iPropertyIndex = 0;
		map<SdaiInstance, pair<_ifc_property*, int>> mapProperties;

		for (size_t iNodeIndex = 0; iNodeIndex < m_vecNodes.size(); iNodeIndex++) {
			auto pGeometry = m_vecNodes[iNodeIndex]->getGeometry();

			auto pPropertySetCollection = pPropertyProvider->getPropertySetCollection(_ptr<_ifc_geometry>(pGeometry)->getSdaiInstance());
			if (pPropertySetCollection == nullptr) {
				continue;
			}

			// Property sets
			for (auto pPropertySet : pPropertySetCollection->propertySets()) {
				auto itPropertySet = mapPropertySets.find(pPropertySet->getSdaiInstance());
				if (itPropertySet == mapPropertySets.end()) {
					mapPropertySets[pPropertySet->getSdaiInstance()] = { pPropertySet, iPropertySetIndex++ };
#ifdef _WINDOWS
					TRACE("Property set: %s\n", (const char*)CW2A(pPropertySet->getName().c_str()));
#endif
				}
				else {
					assert(itPropertySet->second.first->getName() == pPropertySet->getName());
				}

				// Properties
				for (auto pProperty : pPropertySet->properties()) {
					auto itProperty = mapProperties.find(pProperty->getSdaiInstance());
					if (itProperty == mapProperties.end()) {
						mapProperties[pProperty->getSdaiInstance()] = { pProperty, iPropertyIndex++ };
#ifdef _WINDOWS
						TRACE("Property set: %s, property: %s, value: %s\n",
							(const char*)CW2A(pPropertySet->getName().c_str()),
							(const char*)CW2A(pProperty->getName().c_str()),
							(const char*)CW2A(pProperty->getValue().c_str()));
#endif
					}
					else {
						assert(itProperty->second.first->getName() == pProperty->getName());
						assert(itProperty->second.first->getValue() == pProperty->getValue());
					}
				}
			} // for (auto pPropertySet : ...
		} // for (size_t iNodeIndex = ...

		//
		// Write metadata
		//

		*getOutputStream() << getNewLine();
		writeIndent();

		// properties
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << "properties";
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			writeStartArrayTag(false);

			iPropertyIndex = 0;
			for (const auto& itProperty : mapProperties) {
				if (iPropertyIndex++ > 0) {
					*getOutputStream() << COMMA;
				}

				char* szEntityName = nullptr;
				engiGetEntityName(sdaiGetInstanceType(itProperty.second.first->getSdaiInstance()), sdaiSTRING, (const char**)&szEntityName);

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeStringProperty("name", (const char*)CW2A(itProperty.second.first->getName().c_str()));
				*getOutputStream() << COMMA;
				writeStringProperty("ifcValueType", szEntityName);
				*getOutputStream() << COMMA;
				writeStringProperty("value", (const char*)CW2A(itProperty.second.first->getValue().c_str()));
				*getOutputStream() << COMMA;
				writeStringProperty("valueType", "#todo");
				indent()--;

				writeEndObjectTag();
				indent()--;
			} // for (auto itProperty : ...

			writeEndArrayTag();
		}
		// properties

		*getOutputStream() << COMMA;
		*getOutputStream() << getNewLine();
		writeIndent();

		// propertySets
		{
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << "propertySets";
			*getOutputStream() << DOULE_QUOT_MARK;
			*getOutputStream() << COLON;
			*getOutputStream() << SPACE;

			writeStartArrayTag(false);

			iPropertySetIndex = 0;
			for (const auto& itPropertySet : mapPropertySets) {
				if (iPropertySetIndex++ > 0) {
					*getOutputStream() << COMMA;
				}

				char* szGlobalId = nullptr;
				sdaiGetAttrBN(itPropertySet.second.first->getSdaiInstance(), "GlobalId", sdaiSTRING, &szGlobalId);
				assert(szGlobalId != nullptr);

				char* szEntityName = nullptr;
				engiGetEntityName(sdaiGetInstanceType(itPropertySet.second.first->getSdaiInstance()), sdaiSTRING, (const char**)&szEntityName);

				indent()++;
				writeStartObjectTag();

				indent()++;
				writeStringProperty("id", szGlobalId != nullptr ? szGlobalId : "$");
				*getOutputStream() << COMMA;
				writeStringProperty("name", (const char*)CW2A(itPropertySet.second.first->getName().c_str()));
				*getOutputStream() << COMMA;
				writeStringProperty("type", szEntityName != nullptr ? szEntityName : "$");
				*getOutputStream() << COMMA;
				{
					*getOutputStream() << getNewLine();
					writeIndent();
					*getOutputStream() << DOULE_QUOT_MARK;
					*getOutputStream() << "properties";
					*getOutputStream() << DOULE_QUOT_MARK;
					*getOutputStream() << COLON;
					*getOutputStream() << SPACE;

					writeStartArrayTag(false);

					indent()++;
					for (size_t iIndex = 0; iIndex < itPropertySet.second.first->properties().size(); iIndex++) {
						auto pProperty = itPropertySet.second.first->properties()[iIndex];

						if (iIndex > 0) {
							*getOutputStream() << COMMA;
						}

						auto itProperty = mapProperties.find(pProperty->getSdaiInstance());
						assert(itProperty != mapProperties.end());

						*getOutputStream() << getNewLine();
						writeIndent();
						*getOutputStream() << itProperty->second.second;
					}
					indent()--;

					writeEndArrayTag();
				}
				indent()--;

				writeEndObjectTag();
				indent()--;
			} // for (const auto& itPropertySet : ...

			writeEndArrayTag();
		}
		// propertySets		
	}
};
