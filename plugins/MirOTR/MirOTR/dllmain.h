#pragma once

#include "stdafx.h"
#include "version.h"

#define DLLFUNC extern "C" __declspec(dllexport)

extern HINSTANCE hInst;
extern PLUGINLINK *pluginLink;