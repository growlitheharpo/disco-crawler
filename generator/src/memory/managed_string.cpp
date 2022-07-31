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

ManagedString::ManagedString(const char* value)
{
	uint16 size;
	
	ManagedStringPool::Get().AddString(m_index, size, value);
	SetSize(size);
}

void ManagedString::InitializeLookback(ObjectType type, size_t index)
{
	SetLookbackType(type);
	SetLookbackIndex(index);
}

const char* ManagedString::CStr() const
{
	return ManagedStringPool::Get().GetString(m_index);
}

ManagedStringPool::Builder::IndexIntoBlob ManagedStringPool::Builder::Push(const char* entry)
{
	StringHash hash = HeartMurmurHash3(entry);
	IndexIntoBlob blobIndexIfInserted = runningSize;

	// Detect duplicates
	auto&& [previousIter, wasInserted] = previous.emplace(hash, blobIndexIfInserted);
	if (!wasInserted)
	{
		return previousIter->second;
	}

	auto storageIndex = storage.size();
	storage.emplace_back(entry);
	
	reverse[blobIndexIfInserted] = storageIndex;
	runningSize += IndexIntoBlob(storage.back().size() + 1);

	return blobIndexIfInserted;
}

const char* ManagedStringPool::Builder::Lookup(IndexIntoBlob index) const
{
	if (auto iter = reverse.find(index); iter != reverse.end())
		return storage[iter->second].c_str();

	HEART_ASSERT(false);
	return nullptr;
}

void ManagedStringPool::Builder::Finalize(uint8*& outBlob, size_t& outSize)
{
	outSize = runningSize;
	outBlob = (uint8*)malloc(outSize);

	uint8* writer = outBlob;
	uint8* end = outBlob + outSize;
	for (auto& entry : storage)
	{
		memcpy_s(writer, end - writer, entry.data(), entry.size());
		writer += entry.size();
		*writer = '\0';
		++writer;
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

	size_t previousSize = m_builder.runningSize;
	index = m_builder.Push(str);
	size_t newSize = m_builder.runningSize;

	size_t deltaSize = newSize - previousSize;
	size_t len = strlen(str);

	HEART_ASSERT((deltaSize - 1) == len || deltaSize == 0);
	size = uint16(len);
}

const char* ManagedStringPool::GetString(uint32 index) const
{
	if (m_blob)
		return (const char*)(m_blob + index);
	
	return m_builder.Lookup(index);
}

void ManagedStringPool::FinalizeBuilder()
{
	m_builder.Finalize(m_blob, m_size);
}
