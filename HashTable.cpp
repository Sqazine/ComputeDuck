#include "HashTable.h"
#include "Object.h"
#define TABLE_MAX_LOAD 0.75

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

HashTable::HashTable()
    :m_Count(0), m_Capacity(0), m_Entries(nullptr)
{
}

HashTable::~HashTable()
{
    m_Entries = nullptr;
    m_Count = 0;
    m_Capacity = 0;
}

bool HashTable::Set(StrObject *key, const Value &value)
{
    if (m_Count + 1 > m_Capacity * TABLE_MAX_LOAD)
    {
        size_t capacity = GROW_CAPACITY(m_Capacity);
        AdjustCapacity(capacity);
    }

    Entry *entry = FindEntry(m_Capacity, key);
    bool isNewKey = entry->key == nullptr;
    if (isNewKey && IS_NIL_VALUE(entry->value))
        m_Count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

Value* HashTable::Get(StrObject *key)
{
    if (m_Count == 0)
        return nullptr;
    Entry *entry = FindEntry(m_Capacity, key);
    if (entry->key == nullptr)
        return nullptr;

    return &entry->value;
}

bool HashTable::Find(StrObject *key)
{
    if (m_Count == 0)
        return false;
    Entry *entry = FindEntry(m_Capacity, key);
    if (entry->key == nullptr)
        return false;
    return true;
}

bool HashTable::Delete(StrObject *key)
{
    if (m_Count == 0)
        return false;
    Entry *entry = FindEntry(m_Capacity, key);
    if (entry->key == nullptr)
        return false;
    entry->key = nullptr;
    entry->value = Value(true);
    return true;
}

void HashTable::Mark()
{
    for (size_t i = 0; i < m_Capacity; ++i)
    {
        Entry *entry = &m_Entries[i];
        if (entry->key)
        {
            MarkObject(entry->key);
            entry->value.Mark();
        }
    }
}

void HashTable::UnMark()
{
    for (size_t i = 0; i < m_Capacity; ++i)
    {
        Entry *entry = &m_Entries[i];
        if (entry->key)
            UnMarkObject(entry->key);
        entry->value.UnMark();
    }
}

uint32_t HashTable::GetCount() const
{
    return m_Count;
}

uint32_t HashTable::GetCapacity() const
{
    return m_Capacity;
}

const Entry *HashTable::GetEntries() const
{
    return m_Entries;
}

bool HashTable::IsValid(uint32_t idx)
{
    return idx >= 0 && idx < m_Capacity && m_Entries[idx].key != nullptr;
}

Entry *HashTable::FindEntry(uint32_t capacity, StrObject *key)
{
    uint32_t index = key->hash & (capacity - 1);//equal to a%b;
    Entry *tombstone = nullptr;
    while (1)
    {
        Entry *entry = &m_Entries[index];
        if (entry->key == nullptr)
        {
            if (IS_NIL_VALUE(entry->value))
                return tombstone != nullptr ? tombstone : entry;
            else if (tombstone == nullptr)
                tombstone = entry;
        }
        else if (entry->key->hash == key->hash)
            return entry;

        index = (index + 1) & (capacity - 1);
    }
}

void HashTable::AdjustCapacity(uint32_t capacity)
{
    Entry *entries = new Entry[capacity];
    m_Count = 0;
    for (size_t i = 0; i < capacity; ++i)
    {
        entries[i].key = nullptr;
        entries[i].value = Value();
    }

    for (size_t i = 0; i < m_Capacity; ++i)
    {
        Entry *entry = &m_Entries[i];
        if (entry->key == nullptr)
            continue;
        Entry *dest = FindEntry(capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        m_Count++;
    }

    m_Entries = entries;
    m_Capacity = capacity;
}

bool operator==(const HashTable &left, const HashTable &right)
{
    return left.GetCount() == right.GetCount() && left.GetEntries() == right.GetEntries();
}
