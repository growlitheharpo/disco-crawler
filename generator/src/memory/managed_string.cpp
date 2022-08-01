/* Copyright (C) 2022 James Keats
 *
 * You may use, distribute, and modify this code under the terms of its modified
 * BSD-3-Clause license. Use for any commercial purposes is prohibited.
 * You should have received a copy of the license with this file. If not, please visit:
 * https://github.com/growlitheharpo/heart-engine-playground
 *
 */

#include "memory/managed_string.h"

#include <heart/hash/string_hash.h>

ManagedString::ManagedString() :
	m_initialized(false),
	m_index(0),
	m_size(0)
{
}

ManagedString::ManagedString(const char* value)
{
	uint32 index;
	ManagedStringPool::Get().AddString(index, m_size, value);

	HEART_ASSERT(index < (UINT_MAX >> 1));
	m_index = index;
	m_initialized = true;
}

void ManagedString::InitializeLookback(ObjectType type, size_t index)
{
	if (!m_initialized)
		return;

	HEART_ASSERT(index < UINT_MAX);
	ManagedStringPool::Get().InitializeLookback(m_index, LookbackHelper {type, uint32(index)});
}

const char* ManagedString::CStr() const
{
	if (!m_initialized)
		return "";

	return ManagedStringPool::Get().GetString(m_index);
}

ObjectType ManagedString::GetLookbackType() const
{
	if (!m_initialized)
		return ObjectType::Unknown;

	return ManagedStringPool::Get().GetLookback(m_index).type;
}

uint32 ManagedString::GetLookbackIndex() const
{
	if (!m_initialized)
		return -1;

	return ManagedStringPool::Get().GetLookback(m_index).index;
}

ManagedStringPool::Builder::IndexIntoBlob ManagedStringPool::Builder::Push(const char* entry)
{
	StringHash hash = HeartMurmurHash3(entry);
	IndexIntoBlob blobIndexIfInserted = runningSize;

	// !! INCOMPATIBLE WITH LOOKBACK !!
	// Detect duplicates
	// auto&& [previousIter, wasInserted] = previous.emplace(hash, blobIndexIfInserted);
	// if (!wasInserted)
	// {
	// 	return previousIter->second;
	// }

	auto storageIndex = storage.size();
	storage.emplace_back(entry);

	reverse[blobIndexIfInserted] = storageIndex;

	size_t requiredSize = sizeof(LookbackHelper) + storage.back().value.size() + 1;
	runningSize += IndexIntoBlob(requiredSize);

	return blobIndexIfInserted;
}

void ManagedStringPool::Builder::Finalize(uint8*& outBlob, size_t& outSize)
{
	outSize = runningSize;
	outBlob = (uint8*)malloc(outSize);
	HEART_ASSERT(outBlob != nullptr);

	uint8* writer = outBlob;
	uint8* end = outBlob + outSize;
	for (auto& entry : storage)
	{
		StringLayout* layout = (StringLayout*)writer;
		layout->lookback = entry.lookback;
		
		uint8* strStart = (uint8*)&layout->firstCharacter;
		memcpy_s(strStart, end - strStart, entry.value.data(), entry.value.size());

		writer = strStart + entry.value.size();
		*(writer++) = '\0';
	}

	HEART_ASSERT(writer == end);

	decltype(reverse) tmp1 {};
	reverse.swap(tmp1);

	decltype(previous) tmp2 {};
	previous.swap(tmp2);

	decltype(storage) tmp3 {};
	storage.swap(tmp3);
}

ManagedStringPool& ManagedStringPool::Get()
{
	static ManagedStringPool s_globalStringPool;
	return s_globalStringPool;
}

ManagedStringPool::~ManagedStringPool()
{
	if (m_blob)
		free(m_blob);

	m_blob = nullptr;
	m_size = 0;
}

void ManagedStringPool::AddString(uint32& index, uint16& size, const char* str)
{
	HEART_ASSERT(!m_blob);

	index = m_builder.Push(str);
	size = uint16(strlen(str));
}

const char* ManagedStringPool::GetString(uint32 index) const
{
	if (m_blob)
	{
		const StringLayout* str = (const StringLayout*)(m_blob + index);
		return &str->firstCharacter;
	}

	if (auto reverseIter = m_builder.reverse.find(index); reverseIter != m_builder.reverse.end())
	{
		Builder::IndexIntoStorage storageIndex = reverseIter->second;
		const Builder::TemporaryString& temp = m_builder.storage[storageIndex];
		return temp.value.c_str();
	}

	HEART_ASSERT(false);
	return nullptr;
}

void ManagedStringPool::InitializeLookback(uint32 index, LookbackHelper lookback)
{
	auto reverseIter = m_builder.reverse.find(index);
	if (reverseIter != m_builder.reverse.end())
	{
		Builder::IndexIntoStorage storageIndex = reverseIter->second;
		Builder::TemporaryString& temp = m_builder.storage[storageIndex];
		temp.lookback = lookback;
	}
}

LookbackHelper ManagedStringPool::GetLookback(uint32 index) const
{
	if (m_blob)
	{
		const StringLayout* str = (const StringLayout*)(m_blob + index);
		return str->lookback;
	}

	if (auto reverseIter = m_builder.reverse.find(index); reverseIter != m_builder.reverse.end())
	{
		Builder::IndexIntoStorage storageIndex = reverseIter->second;
		const Builder::TemporaryString& temp = m_builder.storage[storageIndex];
		return temp.lookback;
	}

	HEART_ASSERT(false);
	return {};
}

uint32 ManagedStringPool::FinalizeBuilder()
{
	uint32 stringCount = uint32(m_builder.storage.size());
	m_builder.Finalize(m_blob, m_size);
	return stringCount;
}
