#ifndef WIN32_CORE_H
#define WIN32_CORE_H

#include "../core.h"

#include <windows.h>

#include "win32_utils.h"

#include "vulkan/win32_vulkan.h"

#include "win32_types.h"

global os_globals os;

#include "win32_window.h"
#include "win32_mouse.h"
#include "win32_events.h"

#endif //WIN32_CORE_H