#ifndef CRYPTOPP_PCH_H
#define CRYPTOPP_PCH_H

#ifdef CRYPTOPP_GENERATE_X64_MASM

	#include "cpu.h"

#else

	#include "config.h"

	#include "simple.h"
	#include "secblock.h"
	#include "misc.h"
	#include "smartptr.h"

#endif

#endif
