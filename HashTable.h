#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include "Value.h"

struct StrObject;

struct Entry
{
    StrObject *key;
    Value value;
};

class HashTable
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
    Entry *FindEntry(uint32_t capacity, StrObject *key);
    void AdjustCapacity(uint32_t capacity);

    uint32_t m_Count;
    uint32_t m_Capacity;
    Entry *m_Entries;
};

bool operator==(const HashTable& left,const HashTable& right);