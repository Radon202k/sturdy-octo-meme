#ifndef WIN32_CORE_H
#define WIN32_CORE_H

#include "../core.h"

#include <windows.h>

#include "win32_utils.h"
#include "win32_window.h"

#ifdef VULKAN_RENDERER
#include "vulkan/win32_vulkan.h"
#endif

#ifdef OPENGL_RENDERER
#include "opengl/win32_opengl.h"
#endif

#include "win32_types.h"

global os_globals os;

#include "win32_mouse.h"
#include "win32_events.h"
#include "win32_time.h"

#endif //WIN32_CORE_H