#pragma once

#include "../libsrc/libthecore/stdafx.h"

#ifndef _WIN32
#include <semaphore.h>
#else
#define isdigit iswdigit
#define isspace iswspace
#endif

#include "../common/length.h"
#include "../common/tables.h"
#include "../common/singleton.h"
#include "../common/utils.h"
#include "../common/stl.h"
#include "../common/CommonDefines.h"