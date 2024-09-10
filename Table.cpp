#include "Table.h"
#include "Object.h"
#define TABLE_MAX_LOAD 0.75

#define GROW_CAPACITY(capacity) ((capacity) < 8 ? 8 : (capacity)*2)

Table::Table()
    :m_Count(0), m_Capacity(0), m_Entries(nullptr)
{
}

Table::~Table()
{
    SAFE_DELETE_ARRAY(m_Entries);
    m_Count = 0;
    m_Capacity = 0;
}

bool Table::Set(StrObject *key, const Value &value)
{
    if (this->m_Count + 1 > this->m_Capacity * TABLE_MAX_LOAD)
    {
        int capacity = GROW_CAPACITY(this->m_Capacity);
        this->AdjustCapacity(capacity);
    }

    Entry *entry = FindEntry(this->m_Capacity, key);
    bool isNewKey = entry->key == nullptr;
    if (isNewKey && IS_NIL_VALUE(entry->value))
        this->m_Count++;

    entry->key = key;
    entry->value = value;
    return isNewKey;
}

bool Table::Get(StrObject *key, Value& value)
{
    if (this->m_Count == 0)
        return false;
    Entry *entry = FindEntry(this->m_Capacity, key);
    if (entry->key == nullptr)
        return false;

    value = entry->value;
    return true;
}

bool Table::Find(StrObject *key)
{
    if (this->m_Count == 0)
        return false;
    Entry *entry = FindEntry(this->m_Capacity, key);
    if (entry->key == nullptr)
        return false;
    return true;
}

bool Table::Delete(StrObject *key)
{
    if (this->m_Count == 0)
        return false;
    Entry *entry = FindEntry(this->m_Capacity, key);
    if (entry->key == nullptr)
        return false;
    entry->key = nullptr;
    entry->value = Value(true);
    return true;
}

void Table::Mark()
{
    for (int i = 0; i < this->m_Capacity; ++i)
    {
        Entry *entry = &this->m_Entries[i];
        if (entry->key)
            ObjectMark(entry->key);
        entry->value.Mark();
    }
}

void Table::UnMark()
{
    for (int i = 0; i < this->m_Capacity; ++i)
    {
        Entry *entry = &this->m_Entries[i];
        if (entry->key)
            ObjectUnMark(entry->key);
        entry->value.UnMark();
    }
}

uint32_t Table::GetCount() const
{
    return m_Count;
}

uint32_t Table::GetCapacity() const
{
    return m_Capacity;
}

const Entry *Table::GetEntries() const
{
    return m_Entries;
}

bool Table::IsValid(uint32_t idx)
{
    return idx >= 0 && idx < m_Capacity && m_Entries[idx].key != nullptr;
}

Entry *Table::FindEntry(uint32_t capacity, StrObject *key)
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

void Table::AdjustCapacity(uint32_t capacity)
{
    Entry *entries = new Entry[capacity];
    this->m_Count = 0;
    for (int i = 0; i < capacity; ++i)
    {
        entries[i].key = nullptr;
        entries[i].value = Value();
    }

    for (int i = 0; i < this->m_Capacity; ++i)
    {
        Entry *entry = &this->m_Entries[i];
        if (entry->key == nullptr)
            continue;
        Entry *dest = FindEntry(capacity, entry->key);
        dest->key = entry->key;
        dest->value = entry->value;
        this->m_Count++;
    }

    SAFE_DELETE_ARRAY(m_Entries);

    this->m_Entries = entries;
    this->m_Capacity = capacity;
}

bool operator==(const Table &left, const Table &right)
{
    return left.GetCount() == right.GetCount() && left.GetEntries() == right.GetEntries();
}
