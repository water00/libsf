#pragma once
#include "osRelated.h"

#ifdef USE_SELECT
#include "sfThread_select.h"
#else
#include "sfThread_epoll.h"
#endif