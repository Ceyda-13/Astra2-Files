#pragma once

#define M2_NEW new
#define M2_DELETE(p) delete (p)
#define M2_DELETE_ARRAY(p) delete[] (p)

#define THR_INFO __FUNCTION__, __LINE__

template<typename T>
T* get_pointer (T* p)
{
	return p;
}

#include "../libsrc/libthecore/stdafx.h"

#include "../common/stl.h"
#include "../common/singleton.h"
#include "../common/utils.h"
#include "../common/CommonDefines.h"

#include <cmath>
#include <cfloat>

#include "typedef.h"
#include "locale.hpp"
#include "event.h"

#define PASSES_PER_SEC(sec) ((sec) * passes_per_sec)

#ifndef M_PI
#define M_PI    3.14159265358979323846
#endif
#ifndef M_PI_2
#define M_PI_2  1.57079632679489661923
#endif

#define IN
#define OUT


