--[[ Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
--]]

function get_root_location()
	return "%{wks.location}/../"
end

function get_output_location(prj_name)
	return get_root_location() .. "build/bin/" .. prj_name .. "/%{cfg.longname}/"
end

function include_self()
	includedirs {
		"include/",
		"src/",
	}
	files {
		"include/**",
		"src/**",
	}
end

function include_heart(also_link)
	includedirs {
		get_root_location() .. "external/heart/heart/heart-core/include/",
		get_root_location() .. "external/heart/heart/heart-stl/include/",
	}
	if also_link then
		dependson('heart-stl')
		links {
			'heart-core',
			-- 'heart-stl', -- does not actually "build", so no need to link
		}
	end
end

function set_location()
	location "%{wks.location}/proj/%{prj.name}/"
end

workspace "disco-crawler"
	location "build/"
	language "C++"
	cppdialect "c++20"
	startproject "generator"

	architecture "x86_64"
	configurations { "Debug", "Release" }

	filter { "configurations:Debug" }
		defines { "DEBUG", "_DEBUG" }
		symbols "On"

	filter { "configurations:Release" }
		defines { "NDEBUG" }
		optimize "On"
	
	filter {"system:windows", "action:vs*"}
		systemversion "latest"

	filter {}

	flags {
		"FatalWarnings"
	}

	defines {
		"_ITERATOR_DEBUG_LEVEL=0",
	}

	targetdir ("build/bin/%{prj.name}/%{cfg.longname}")
	objdir ("build/obj/%{prj.name}/%{cfg.longname}")

include "external/"

include "generator/"

include "visualizer/"
