#include "stdafx.h"

#include "_ifc2gltf_exporter.h"
#include "_string.h"

// ************************************************************************************************
namespace _ifc2gltf
{
	_exporter::_exporter(const char* szInputFile, const char* szOutputFile, bool bEmbeddedBuffers)
		: _bin2gltf::_exporter(szInputFile, szOutputFile, bEmbeddedBuffers)
		, m_sdaiModel(0)
		//, m_pAP242(nullptr)//#todo
	{}

	/*virtual*/ _exporter::~_exporter()
	{
		if (m_sdaiModel != 0)
		{
			sdaiCloseModel(m_sdaiModel);
			m_sdaiModel = 0;
		}

		//delete m_pAP242;//#todo
	}

	/*virtual*/ void _exporter::postExecute() /*override*/
	{
		_bin2gltf::_exporter::postExecute();

		if (m_sdaiModel != 0)
		{
			sdaiCloseModel(m_sdaiModel);
			m_sdaiModel = 0;
		}

		//#todo
		/*if (m_pAP242 != nullptr)
		{
			delete m_pAP242;
			m_pAP242 = nullptr;
		}	*/	
	}

	/*virtual*/ void _exporter::GetInstancesDefaultState() /*override*/
	{
		VERIFY_INSTANCE(getInputFile());

		//
		// Geometry
		//
		m_mapInstanceDefaultState.clear();

		m_sdaiModel = sdaiOpenModelBNUnicode(0, (LPCWSTR)CA2W(getInputFile()), L"");
		if (m_sdaiModel == 0)
		{
			getLog()->logWrite(enumLogEvent::error, "Invalid model.");

			return;
		}

		fs::path pthInputFile = getInputFile();

		string strExtension = pthInputFile.extension().string();
		_string::toUpper(strExtension);

		string strTargetEntity;
		if (strExtension == ".IFC")
		{
			strTargetEntity = "IFCPRODUCT";
		}
		else if ((strExtension == ".STEP") || (strExtension == ".STP"))
		{
			strTargetEntity = "PRODUCT_DEFINITION";

			//#todo
			//m_pAP242 = new _AP242(m_sdaiModel);
		}

		if (strTargetEntity.empty())
		{
			getLog()->logWrite(enumLogEvent::error, "Unknown file extension.");

			return;
		}

		SdaiAggr sdaiAggr = xxxxGetEntityAndSubTypesExtentBN(m_sdaiModel, strTargetEntity.c_str());
		SdaiInteger iMembersCount = sdaiGetMemberCount(sdaiAggr);
		for (SdaiInteger iIndex = 0; iIndex < iMembersCount; iIndex++)
		{
			SdaiInstance sdaiInstance = 0;
			engiGetAggrElement(sdaiAggr, iIndex, sdaiINSTANCE, &sdaiInstance);

			SdaiEntity sdaiEntity = sdaiGetInstanceType(sdaiInstance);

			char* szName = nullptr;
			engiGetEntityName(sdaiEntity, sdaiSTRING, (const char**)&szName);

			string strEntity = szName;
			_string::toUpper(strEntity);

			if ((strEntity == "IFCSPACE") ||
				(strEntity == "IFCRELSPACEBOUNDARY") ||
				(strEntity == "IFCOPENINGELEMENT") ||
				(strEntity == "IFCALIGNMENTVERTICAL") ||
				(strEntity == "IFCALIGNMENTHORIZONTAL") ||
				(strEntity == "IFCALIGNMENTSEGMENT") ||
				(strEntity == "IFCALIGNMENTCANT"))
			{
				continue;
			}

			OwlInstance	owlInstance = 0;
			owlBuildInstance(m_sdaiModel, sdaiInstance, &owlInstance);

			if (owlInstance != 0)
			{
				if (GetInstanceGeometryClass(owlInstance) &&
					GetBoundingBox(owlInstance, nullptr, nullptr))
				{
					m_mapInstanceDefaultState[owlInstance] = true;
					
					//#todo
					/*if (m_pAP242 != nullptr)
					{
						m_pAP242->addProductDefinitionInstance(sdaiInstance);
					}	*/				
				}
			}
		} // for (SdaiInteger iIndex = ...

		//
		// Transformations
		//
		// //#todo
		//if (m_pAP242 != nullptr)
		//{
		//	m_pAP242->load();

		//	for (auto& itProductDefinition : m_pAP242->getProductDefinitions())
		//	{
		//		for (auto pProductInstance : itProductDefinition.second->productInstances())
		//		{
		//			OwlInstance	owInstance = 0;
		//			owlBuildInstance(m_sdaiModel, itProductDefinition.second->getSdaiInstance(), &owInstance);

		//			if (owInstance != 0)
		//			{
		//				auto itInstanceTransformation = m_mapInstanceTransformations.find(owInstance);
		//				if (itInstanceTransformation != m_mapInstanceTransformations.end())
		//				{
		//					itInstanceTransformation->second.push_back(pProductInstance->getTransformationMatrix());
		//				}
		//				else
		//				{
		//					m_mapInstanceTransformations[owInstance] = vector<_matrix*>{ pProductInstance->getTransformationMatrix() };
		//				}
		//			} // if (owInstance != 0)
		//		} // for (auto pProductInstance ...
		//	} // for (auto& itProductDefinition ...
		//} // if (m_pAP242 != nullptr)
	}
};