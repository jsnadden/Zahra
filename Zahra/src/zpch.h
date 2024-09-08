#pragma once

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <memory>
#include <functional>
#include <algorithm>
#include <utility>
#include <filesystem>

#include <array>
#include <string>
#include <vector>
#include <map>
#include <unordered_map>
#include <unordered_set>

#include "Zahra/Core/Assert.h"
#include "Zahra/Core/Log.h"
#include "Zahra/Debug/Profiling.h"
#include "Zahra/Core/Ref.h"
#include "Zahra/Core/Scope.h"


#ifdef Z_PLATFORM_WINDOWS
	#include <Windows.h>
#endif
