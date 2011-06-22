#ifndef INCLUDED_SRCEXFUN_H
#define INCLUDED_SRCEXFUN_H

#include <libxml/xpath.h>
#include "SAX2Utilities.hpp"

void setPosition(int n);

void setRootAttributes(PROPERTIES_TYPE&);

void xsltsrcMLRegister();

void xpathsrcMLRegister(xmlXPathContextPtr context);

void xpathRegisterDefaultExtensionFunctions();

void xpathRegisterExtensionFunction(const std::string & name, const std::string & xpath);

#endif
