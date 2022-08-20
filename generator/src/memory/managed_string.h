/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#pragma once

#include "types/object_type.h"

#include <heart/copy_move_semantics.h>
#include <heart/debug/assert.h>
#include <heart/types.h>

#include <heart/stl/string.h>
#include <heart/stl/unordered_map.h>
#include <heart/stl/vector.h>

class ManagedStringPool;

struct LookbackHelper
{
	ObjectType type = {};
	uint32 index = 0;
};

class ManagedString
{
private:
	friend class HashLookup;

	uint32 m_initialized : 1;
	uint32 m_index : 31;
	uint16 m_size = 0;

public:
	ManagedString();
	~ManagedString() = default;

	ManagedString(const char* value);

	void InitializeLookback(ObjectType type, size_t index);

	const char* CStr() const;

	ObjectType GetLookbackType() const;

	uint32 GetLookbackIndex() const;

	uint32 GetSize() const
	{
		return m_size;
	}
};

class ManagedStringPool
{
public:
	static ManagedStringPool& Get();

private:
	friend class HashLookup;

	struct Builder
	{
		typedef uint32 StringHash;
		typedef uint32 IndexIntoBlob;
		typedef size_t IndexIntoStorage;

		struct TemporaryString
		{
			hrt::string value;
			LookbackHelper lookback;
		};

		hrt::vector<TemporaryString> storage;
		hrt::unordered_map<StringHash, IndexIntoBlob> previous;
		hrt::unordered_map<IndexIntoBlob, IndexIntoStorage> reverse;

		IndexIntoBlob runningSize = 0;

		IndexIntoBlob Push(const char* entry);
		void Finalize(uint8*& outBlob, size_t& outSize);
	};

#pragma pack(push)
#pragma pack(1)
	struct StringLayout
	{
		LookbackHelper lookback;
		char firstCharacter;
	};
#pragma pack(pop)

	uint8* m_blob = nullptr;
	size_t m_size = 0;

	Builder m_builder;

public:
	ManagedStringPool() = default;
	DISABLE_COPY_AND_MOVE_SEMANTICS(ManagedStringPool);
	~ManagedStringPool();

	void AddString(uint32& index, uint16& size, const char* str);
	const char* GetString(uint32 index) const;

	void InitializeLookback(uint32 index, LookbackHelper lookback);
	LookbackHelper GetLookback(uint32 index) const;

	uint32 FinalizeBuilder();
};
