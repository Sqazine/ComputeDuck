#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include "Value.h"

struct StrObject;

struct Entry
{
    StrObject *key{nullptr};
    Value value;
};

class COMPUTEDUCK_API HashTable
{
public:
    HashTable();
    ~HashTable();

    bool Set(StrObject *key,const Value& value);
    Value* Get(StrObject *key);
    bool Find(StrObject *key);
    bool Delete(StrObject *key);
    void Mark();
    void UnMark();

    uint32_t GetCount() const;
    uint32_t GetCapacity() const;
    const Entry* GetEntries() const;

    bool IsValid(uint32_t idx);
private:
    Entry *FindEntry(Entry *entries, uint32_t capacity, StrObject *key);
    Entry *FindEntry(StrObject *key);
    void AdjustCapacity(uint32_t capacity);

    uint32_t m_Count;
    uint32_t m_Capacity;
    Entry *m_Entries;
};

bool operator==(const HashTable& left,const HashTable& right);