--[[ Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
--]]

local export_include_root = "%{wks.location}/../external/"

group "external"

project "rapidjson"
	kind "None"
	set_location()
	files { "rapidjson/include/**" }
	includedirs { "rapidjson/include/" }

function include_entt(should_link)
	includedirs {
		export_include_root .. "entt/src/",
	}

	filter { "configurations:Release" }
		defines { "ENTT_DISABLE_ASSERT=1" }
	filter {}
end

project "entt"
	kind "None" -- entt is header-only
	set_location()
	warnings "Off"
	include_entt()
	files {
		"entt/src/**",
		"entt/README.md",
	}

group "heart"
	include "heart/heart/heart-core"
	include "heart/heart/heart-stl"
group "*"
