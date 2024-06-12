#include "pch.h"
#include "IFCAttribute.h"

// ************************************************************************************************
CIFCAttribute::CIFCAttribute(SdaiAttr pInstance)
	: m_pInstance(pInstance)
{
	ASSERT(m_pInstance != 0);
}

/*virtual*/ CIFCAttribute::~CIFCAttribute()
{}

SdaiPrimitiveType CIFCAttribute::GetType() const
{
	SdaiPrimitiveType iType = engiGetAttrType(m_pInstance);

	if ((iType & engiTypeFlagAggr) ||
		(iType & engiTypeFlagAggrOption))
	{
		iType = sdaiAGGR;
	}

	return iType;
}

// ************************************************************************************************
CIFCAttributeProvider::CIFCAttributeProvider()
	: m_mapInstanceAttributes()
{}

/*virtual*/ CIFCAttributeProvider::~CIFCAttributeProvider()
{
	for (auto& itInstanceAttributes : m_mapInstanceAttributes)
	{
		for (auto pAttribute : itInstanceAttributes.second)
		{
			delete pAttribute;
		}
	}
}

const vector<CIFCAttribute*>& CIFCAttributeProvider::GetInstanceAttributes(SdaiInstance iInstance)
{
	ASSERT(iInstance != 0);

	auto itInstanceAttributes = m_mapInstanceAttributes.find(iInstance);
	if (itInstanceAttributes != m_mapInstanceAttributes.end())
	{
		return itInstanceAttributes->second;
	}

	SdaiEntity iEntity = sdaiGetInstanceType(iInstance);
	ASSERT(iEntity != 0);

	vector<CIFCAttribute*> vecAttributes;
	LoadInstanceAttributes(iEntity, iInstance, vecAttributes);

	m_mapInstanceAttributes[iInstance] = vecAttributes;

	return m_mapInstanceAttributes.at(iInstance);
}

void CIFCAttributeProvider::LoadInstanceAttributes(SdaiEntity iEntity, SdaiInstance iInstance, vector<CIFCAttribute*>& vecAttributes)
{
	if (iEntity == 0)
	{
		return;
	}

	ASSERT(iInstance != 0);	

	LoadInstanceAttributes(engiGetEntityParent(iEntity), iInstance, vecAttributes);

	SdaiInteger iIndex = 0;
	SdaiAttr pAttribute = engiGetEntityAttributeByIndex(
		iEntity,
		iIndex++,
		false,
		true);

	while (pAttribute != nullptr)
	{
		auto pIFCAttribute = new CIFCAttribute(pAttribute);
		vecAttributes.push_back(pIFCAttribute);

		pAttribute = engiGetEntityAttributeByIndex(
			iEntity,
			iIndex++,
			false,
			true);
	}
}