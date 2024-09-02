
#include "stdafx.h"

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(void) XmlAddAttr(TiXmlElement *hXml, const char *name, const char *value)
{
	if (hXml && value)
		hXml->SetAttribute(name, value);
}

MIR_CORE_DLL(const char*) XmlGetAttr(const TiXmlElement *hXml, const char *pszName)
{
	return (hXml == nullptr) ? nullptr : hXml->Attribute(pszName);
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(TiXmlElement*) XmlAddChild(TiXmlElement *hXml, const char *name)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	hXml->InsertEndChild(res);
	return res;
}

MIR_CORE_DLL(TiXmlElement*) XmlAddChildA(TiXmlElement *hXml, const char *name, const char *value)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	if (value)
		res->SetText(value);
	hXml->InsertEndChild(res);
	return res;
}

MIR_CORE_DLL(TiXmlElement*) XmlAddChildI(TiXmlElement *hXml, const char *name, int value)
{
	if (hXml == nullptr)
		return nullptr;

	auto *res = hXml->GetDocument()->NewElement(name);
	if (value)
		res->SetText(value);
	hXml->InsertEndChild(res);
	return res;
}

/////////////////////////////////////////////////////////////////////////////////////////

MIR_CORE_DLL(const TiXmlElement*) XmlFirstChild(const TiXmlElement *hXml, const char *key)
{
	return (hXml) ? hXml->FirstChildElement(key) : nullptr;
}

MIR_CORE_DLL(const char*) XmlGetChildText(const TiXmlElement *hXml, const char *key)
{
	auto *pChild = XmlFirstChild(hXml, key);
	return (pChild == nullptr) ? nullptr : pChild->GetText();
}

MIR_CORE_DLL(int) XmlGetChildInt(const TiXmlElement *hXml, const char *key)
{
	auto *pChild = XmlFirstChild(hXml, key);
	return (pChild == nullptr) ? 0 : atoi(pChild->GetText());
}

MIR_CORE_DLL(const TiXmlElement*) XmlGetChildByTag(const TiXmlElement *hXml, const char *key, const char *attrName, const char *attrValue)
{
	if (hXml)
		for (auto *pChild : TiXmlFilter(hXml, key))
			if (pChild->Attribute(attrName, attrValue))
				return pChild;

	return nullptr;
}

MIR_CORE_DLL(int) XmlGetChildCount(const TiXmlElement *hXml)
{
	int iCount = 0;
	if (hXml)
		for (auto *it : TiXmlEnum(hXml)) {
			UNREFERENCED_PARAMETER(it);
			iCount++;
		}

	return iCount;
}
