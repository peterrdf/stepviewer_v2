#pragma once

#include "_ifc_geometry.h"
#include "_oglUtils.h"

#include <vector>
using namespace std;

// ************************************************************************************************
class _node
{

private: // Members

	uint32_t m_iBufferByteLength; // buffer: byteLength
	string m_strBufferBinFile; // buffer: uri (*.bin)	
	uint32_t m_iVerticesBufferViewByteLength; // vertices/ARRAY_BUFFER/POSITION
	uint32_t m_iNormalsBufferViewByteLength; // normals/ARRAY_BUFFER/NORMAL
	uint32_t m_iTexturesBufferViewByteLength; // normals/ARRAY_BUFFER/TEXTURE
	vector<uint32_t> m_vecIndicesBufferViewsByteLength; // indices/ELEMENT_ARRAY_BUFFER: conceptual faces, ..., conceptual face polygons, ..., lines..., points...
	vector<uint32_t> m_vecAccessors; // accessors: bufferView vertices, bufferView indices conceptual faces, ...
	vector<uint32_t> m_vecMeshes; // meshes: conceptual face 1, 2, ...
	const vector<_matrix*>* m_pvecTransformations;

public: // Methods

	_node();
	virtual ~_node();

	// glTF
	uint32_t& bufferByteLength() { return m_iBufferByteLength; }
	string& bufferBinFile() { return m_strBufferBinFile; }
	uint32_t& verticesBufferViewByteLength() { return m_iVerticesBufferViewByteLength; }
	uint32_t& normalsBufferViewByteLength() { return m_iNormalsBufferViewByteLength; }
	uint32_t& texturesBufferViewByteLength() { return m_iTexturesBufferViewByteLength; }
	vector<uint32_t>& indicesBufferViewsByteLength() { return m_vecIndicesBufferViewsByteLength; }
	vector<uint32_t>& accessors() { return m_vecAccessors; }
	vector<uint32_t>& meshes() { return m_vecMeshes; }
	const vector<_matrix*>* getTransformations() const { return m_pvecTransformations; }

protected: // Methods

	void UpdateName();
};
