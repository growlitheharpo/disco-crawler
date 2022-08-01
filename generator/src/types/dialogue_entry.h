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

struct DialogEntry
{
	static constexpr ObjectType Type = ObjectType::DialogEntry;

	int32 id = 0;
	
	int32 conversationId = 0;

	ManagedString title;

	ManagedString dialogText;

	ManagedString sequence;

	int32 actor = 0;

	int32 conversant = 0;

	uint64 articyId = 0;

	uint8 isRoot = 0;

	uint8 isGroup = 0;

	int16 conditionPriority = 0;

	// TODO: outgoing links

	uint64 outputId = 0;

	uint64 inputId = 0;

	ManagedString conditionsString;

	std::array<ManagedString*, 4> GetStrings()
	{
		std::array<ManagedString*, 4> r = {
			&title,
			&dialogText,
			&sequence,
			&conditionsString,
		};

		return r;
	}
};

template <typename RapidjsonT>
DialogEntry ParseDialogEntry(RapidjsonT&& jsonObj)
{
	DialogEntry result;

	auto fieldsArrayIter = jsonObj.FindMember("fields");
	if (fieldsArrayIter == jsonObj.MemberEnd() || !fieldsArrayIter->value.IsArray())
		return result;

	auto fieldsArray = fieldsArrayIter->value.GetArray();

	READ_NAMED_SINGLE_FIELD(result, id, jsonObj);

	READ_NAMED_SINGLE_FIELD(result, conversationId, jsonObj);

	READ_NAMED_SINGLE_FIELD(result, isRoot, jsonObj);

	READ_NAMED_SINGLE_FIELD(result, isGroup, jsonObj);

	READ_NAMED_SINGLE_FIELD(result, conditionPriority, jsonObj);

	READ_NAMED_SINGLE_FIELD(result, conditionsString, jsonObj);

	ReadFromFieldsArray(result.title, fieldsArray, "Title");

	ReadFromFieldsArray(result.dialogText, fieldsArray, "Dialogue Text");

	ReadFromFieldsArray(result.articyId, fieldsArray, "Articy Id");

	ReadFromFieldsArray(result.actor, fieldsArray, "Actor");

	READ_NAMED_FROM_FIELDS_ARRAY(result, conversant, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, outputId, fieldsArray);

	READ_NAMED_FROM_FIELDS_ARRAY(result, inputId, fieldsArray);

	return result;
}
