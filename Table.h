#pragma once
#include <string>
#include <string_view>
#include <unordered_map>
#include <cstdint>
#include "Value.h"

struct Entry
{
    struct StrObject *key;
    Value value;
};

class Table
{
public:
    Table();
    ~Table();

    bool Set(struct StrObject *key,const Value& value);
    bool Get(struct StrObject *key,Value& value);
    bool Find(struct StrObject *key);
    bool Delete(struct StrObject *key);
    void Mark();
    void UnMark();

    uint32_t GetCount() const;
    uint32_t GetCapacity() const;
    const Entry* GetEntries() const;

    bool IsValid(uint32_t idx);
private:
    Entry *FindEntry(uint32_t capacity, struct StrObject *key);
    void AdjustCapacity(uint32_t capacity);

    uint32_t m_Count;
    uint32_t m_Capacity;
    Entry *m_Entries;
};

bool operator==(const Table& left,const Table& right);