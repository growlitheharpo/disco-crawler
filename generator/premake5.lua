--[[ Copyright (C) 2022 James Keats
*
* You may use, distribute, and modify this code under the terms of its modified
* BSD-3-Clause license. Use for any commercial purposes is prohibited.

* You should have received a copy of the license with this file. If not, please visit:
* https://github.com/growlitheharpo/heart-engine-playground
*
--]]

project "generator"
	kind "ConsoleApp"
	set_location()

	include_self()
	include_heart(true)
	include_entt()
	debugdir "%{wks.location}/"

	defines {
		"RAPIDJSON_ASSERT=HEART_ASSERT",
	}
	includedirs {
		get_root_location() .. "external/rapidjson/include",
		"src/"
	}
