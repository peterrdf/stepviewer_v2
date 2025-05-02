#include "stdafx.h"
#include "_node.h"

#include <memory>

// ************************************************************************************************
_node::_node(OwlInstance owlInstance, SdaiInstance sdaiInstance, const vector<_ifc_geometry*>& vecMappedGeometries)
	: _ifc_geometry(owlInstance, sdaiInstance, vecMappedGeometries)
	, m_iBufferByteLength(0)
	, m_strBufferBinFile("")	
	, m_iVerticesBufferViewByteLength(0)
	, m_iNormalsBufferViewByteLength(0)
	, m_iTexturesBufferViewByteLength(0)
	, m_vecIndicesBufferViewsByteLength()
	, m_vecAccessors()
	, m_vecMeshes()
	, m_pvecTransformations(nullptr)
{
	UpdateName();
}

/*virtual*/ _node::~_node()
{
}

void _node::UpdateName()
{
	//#todo
	// //REMOVE????
	//OwlClass iClassInstance = GetInstanceClass(m_iInstance);
	//assert(iClassInstance != 0);

	//wchar_t* szClassName = nullptr;
	//GetNameOfClassW(iClassInstance, &szClassName);

	//wchar_t* szName = nullptr;
	//GetNameOfInstanceW(m_iInstance, &szName);

	//if (szName == nullptr)
	//{
	//	RdfProperty iTagProperty = GetPropertyByName(getModel(), "tag");
	//	if (iTagProperty != 0)
	//	{
	//		SetCharacterSerialization(getModel(), 0, 0, false);

	//		int64_t iCard = 0;
	//		wchar_t** szValue = nullptr;
	//		GetDatatypeProperty(m_iInstance, iTagProperty, (void**)&szValue, &iCard);

	//		if (iCard == 1)
	//		{
	//			szName = szValue[0];
	//		}

	//		SetCharacterSerialization(getModel(), 0, 0, true);
	//	}
	//} // if (szName == nullptr)

	//wchar_t szUniqueName[512];

	//if (szName != nullptr)
	//{
	//	m_strName = szName;
	//	swprintf(szUniqueName, 512, L"%s (%s)", szName, szClassName);
	//}
	//else
	//{
	//	m_strName = szClassName;
	//	swprintf(szUniqueName, 512, L"#%lld (%s)", m_iInstance, szClassName);
	//}

	//m_strUniqueName = szUniqueName;
}
