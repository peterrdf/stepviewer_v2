#include "stdafx.h"
#include "_ifc_attribute.h"

// ************************************************************************************************
_ifc_attribute::_ifc_attribute(SdaiAttr sdaiAttr)
	: m_sdaiAttr(sdaiAttr)
{
	assert(m_sdaiAttr != 0);
}

/*virtual*/ _ifc_attribute::~_ifc_attribute()
{}

SdaiPrimitiveType _ifc_attribute::getType() const
{
	SdaiPrimitiveType sdaiPrimitiveType = engiGetAttrType(m_sdaiAttr);
	if ((sdaiPrimitiveType & engiTypeFlagAggr) ||
		(sdaiPrimitiveType & engiTypeFlagAggrOption))
	{
		sdaiPrimitiveType = sdaiAGGR;
	}

	return sdaiPrimitiveType;
}

// ************************************************************************************************
_ifc_attribute_provider::_ifc_attribute_provider()
	: m_mapInstanceAttributes()
{}

/*virtual*/ _ifc_attribute_provider::~_ifc_attribute_provider()
{
	for (auto& itInstanceAttributes : m_mapInstanceAttributes)
	{
		for (auto pAttribute : itInstanceAttributes.second)
		{
			delete pAttribute;
		}
	}
}

const vector<_ifc_attribute*>& _ifc_attribute_provider::getInstanceAttributes(SdaiInstance sdaiInstance)
{
	assert(sdaiInstance != 0);

	auto itInstanceAttributes = m_mapInstanceAttributes.find(sdaiInstance);
	if (itInstanceAttributes != m_mapInstanceAttributes.end())
	{
		return itInstanceAttributes->second;
	}

	SdaiEntity sdaiEntity = sdaiGetInstanceType(sdaiInstance);
	assert(sdaiEntity != 0);

	vector<_ifc_attribute*> vecAttributes;
	loadInstanceAttributes(sdaiEntity, sdaiInstance, vecAttributes);

	m_mapInstanceAttributes[sdaiInstance] = vecAttributes;

	return m_mapInstanceAttributes.at(sdaiInstance);
}

void _ifc_attribute_provider::loadInstanceAttributes(SdaiEntity sdaiEntity, SdaiInstance sdaiInstance, vector<_ifc_attribute*>& vecAttributes)
{
	if (sdaiEntity == 0)
	{
		return;
	}

	assert(sdaiInstance != 0);

	loadInstanceAttributes(engiGetEntityParent(sdaiEntity), sdaiInstance, vecAttributes);

	SdaiInteger iIndex = 0;
	SdaiAttr pAttribute = engiGetEntityAttributeByIndex(
		sdaiEntity,
		iIndex++,
		false,
		true);

	while (pAttribute != nullptr)
	{
		auto pIFCAttribute = new _ifc_attribute(pAttribute);
		vecAttributes.push_back(pIFCAttribute);

		pAttribute = engiGetEntityAttributeByIndex(
			sdaiEntity,
			iIndex++,
			false,
			true);
	}
}