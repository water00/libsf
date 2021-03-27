#pragma once

//#define USE_SELECT

#ifdef USE_SELECT
#include "sfThread_select.h"
#else
#include "sfThread_epoll.h"
#endif