/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#include "types/actor.h"
#include "types/conversation.h"
#include "types/dialogue_entry.h"
#include "types/variable.h"

#include "memory/hash_lookup.h"
#include "os/slim_win32.h"
#include "json/rapidjson_wrapper.h"

#include <heart/countof.h>
#include <heart/stl/vector.h>
#include <heart/types.h>

#include <iostream>

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
	::SetConsoleOutputCP(CP_UTF8);
	setvbuf(stdout, nullptr, _IOFBF, 1000);

	rapidjson::Document doc;
	hrt::vector<Actor> actors;
	hrt::vector<Variable> variables;
	hrt::vector<Conversation> conversations;
	hrt::vector<DialogEntry> dialogEntries;

	std::cout << "Reading json... ";
	std::cout.flush();
	{
		doc = ParseDocumentAsStream("C:\\Users\\James\\Desktop\\DiscoDump\\Disco Elysium.json");
		if (!doc.IsObject())
			return 1;
	}
	std::cout << "Done!" << std::endl;

	std::cout << "Parsing entries... ";
	std::cout.flush();
	{
		auto rootObj = doc.GetObject();
		if (auto actorsIter = rootObj.FindMember("actors"); actorsIter != rootObj.MemberEnd() && actorsIter->value.IsArray())
		{
			auto actorsArray = actorsIter->value.GetArray();
			for (auto& entry : actorsArray)
			{
				actors.push_back(ParseActor(entry.GetObject()));
				InitializeLookback(actors.back(), actors.size() - 1);
			}
		}

		if (auto variablesIter = rootObj.FindMember("variables"); variablesIter != rootObj.MemberEnd() && variablesIter->value.IsArray())
		{
			auto variablesArray = variablesIter->value.GetArray();
			for (auto& entry : variablesArray)
			{
				variables.push_back(ParseVariable(entry.GetObject()));
				InitializeLookback(variables.back(), variables.size() - 1);
			}
		}

		if (auto conversationsIter = rootObj.FindMember("conversations"); conversationsIter != rootObj.MemberEnd() && conversationsIter->value.IsArray())
		{
			auto conversationsArray = conversationsIter->value.GetArray();
			for (auto& conversationJson : conversationsArray)
			{
				Conversation& conversation = conversations.emplace_back(ParseConversation(conversationJson));
				InitializeLookback(conversation, conversations.size() - 1);

				auto dialogIter = conversationJson.FindMember("dialogueEntries");
				if (dialogIter != conversationJson.MemberEnd() && dialogIter->value.IsArray())
				{
					auto dialogArray = dialogIter->value.GetArray();
					for (auto& dialogJson : dialogArray)
					{
						DialogEntry& dialogEntry = dialogEntries.emplace_back(ParseDialogEntry(dialogJson));
						InitializeLookback(dialogEntry, dialogEntries.size() - 1);

						HEART_ASSERT(conversation.dialogEntryCount < HeartCountOf(conversation.dialogEntries));
						conversation.dialogEntries[conversation.dialogEntryCount++] = uint32(dialogEntries.size() - 1);
					}
				}
			}
		}

		doc = {};
	}
	std::cout << "Done! Found " << actors.size() << " actors, " << conversations.size() << " conversations and " << dialogEntries.size() << " dialog nodes." << std::endl;

	std::cout << "Finalizing string pool... ";
	std::cout.flush();
	uint32 stringCount = ManagedStringPool::Get().FinalizeBuilder();
	std::cout << "Done! " << stringCount << " strings pooled." << std::endl;

	std::cout << "Compiling index... ";
	std::cout.flush();
	HashLookup hasher;
	uint32 hashCount = hasher.Compile();
	std::cout << "Done! " << hashCount << " words indexed." << std::endl;

	std::string input;
	std::cout << "Ready to search:" << std::endl;
	while (input != "exitnow")
	{
		std::getline(std::cin, input);

		auto matches = hasher.LookupWord(input.c_str());
		for (ManagedString& match : matches)
		{
			if (match.GetLookbackType() == ObjectType::DialogEntry)
			{
				std::cout << match.CStr() << std::endl;
			}
		}

		std::cout << std::endl;
	}

	return 0;
}
