/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#pragma once

#include "memory/managed_string.h"
#include "types/object_type.h"
#include "json/read_helpers.h"

#include <heart/types.h>

#include <array>

struct Conversation
{
	static constexpr ObjectType Type = ObjectType::Conversation;

	int32 id = 0;

	ManagedString title;

	uint64 articyId = 0;

	hrt::vector<size_t> dialogEntries;

	std::array<ManagedString*, 1> GetStrings()
	{
		std::array<ManagedString*, 1> r = {
			&title,
		};

		return r;
	}
};

template <typename RapidJsonT>
Conversation ParseConversation(RapidJsonT&& jsonObj)
{
	Conversation result;

	auto fieldsArrayIter = jsonObj.FindMember("fields");
	if (fieldsArrayIter == jsonObj.MemberEnd() || !fieldsArrayIter->value.IsArray())
		return result;

	auto fieldsArray = fieldsArrayIter->value.GetArray();

	READ_NAMED_SINGLE_FIELD(result, id, jsonObj);

	READ_NAMED_FROM_FIELDS_ARRAY(result, title, fieldsArray);

	ReadFromFieldsArray(result.articyId, fieldsArray, "Articy Id");

	return result;
}
