/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#include "types/actor.h"
#include "types/variable.h"

#include "json/rapidjson_wrapper.h"

#include <heart/types.h>

#include <heart/stl/vector.h>

template <typename T>
void InitializeLookback(T& target, size_t index)
{
	for (ManagedString* str : target.GetStrings())
	{
		str->InitializeLookback(T::Type, index);
	}
}

int main()
{
	// Takes ~10 seconds in debug, ~1 second in release
	rapidjson::Document doc = ParseDocumentAsStream("C:\\Users\\James\\Desktop\\DiscoDump\\Disco Elysium.json");
	if (!doc.IsObject())
		return 1;

	auto rootObj = doc.GetObject();

	for (auto field = rootObj.MemberBegin(); field != rootObj.MemberEnd(); ++field)
	{
		printf("%s\n", field->name.GetString());
	}

	hrt::vector<Actor> actors;
	if (auto actorsIter = rootObj.FindMember("actors"); actorsIter != rootObj.MemberEnd() && actorsIter->value.IsArray())
	{
		auto actorsArray = actorsIter->value.GetArray();
		for (auto& entry : actorsArray)
		{
			actors.push_back(ParseActor(entry.GetObject()));
			InitializeLookback(actors.back(), actors.size() - 1);
		}
	}

	hrt::vector<Variable> variables;
	if (auto variablesIter = rootObj.FindMember("variables"); variablesIter != rootObj.MemberEnd() && variablesIter->value.IsArray())
	{
		auto variablesArray = variablesIter->value.GetArray();
		for (auto& entry : variablesArray)
		{
			variables.push_back(ParseVariable(entry.GetObject()));
			InitializeLookback(variables.back(), variables.size() - 1);
		}
	}

	ManagedStringPool::Get().FinalizeBuilder();

	for (auto& actor : actors)
	{
		printf("Actor %u is %s - %s\n", actor.id, actor.name.CStr(), actor.description.CStr());
	}

	return 0;
}
