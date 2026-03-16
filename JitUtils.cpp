#ifdef COMPUTEDUCK_BUILD_WITH_LLVM

#include "JitUtils.h"
#include "Utils.h"
#include "Value.h"
#include "Object.h"
#include "Allocator.h"
#include "HashTable.h"
#include <random>

extern "C" COMPUTEDUCK_API StrObject *AllocateStrObject(const char *v)
{
    return ALLOCATE_OBJECT(StrObject, v);
}

extern "C" COMPUTEDUCK_API ArrayObject *AllocateArrayObject(Value *elements, uint32_t size)
{
    return ALLOCATE_OBJECT(ArrayObject, elements, size);
}

extern "C" COMPUTEDUCK_API RefObject *AllocateRefObject(Value *pointer)
{
    return ALLOCATE_OBJECT(RefObject, pointer);
}

extern "C" COMPUTEDUCK_API StructObject *AllocateStructObject(HashTable *table)
{
    return ALLOCATE_OBJECT(StructObject, table);
}

extern "C" COMPUTEDUCK_API Value *GetLocalVariableSlot(int16_t index)
{
    return GET_LOCAL_VARIABLE_SLOT(index);
}

extern "C" COMPUTEDUCK_API HashTable *AllocateHashTable()
{
    return new HashTable();
}

extern "C" COMPUTEDUCK_API bool HashTableSet(HashTable *table, StrObject *key, const Value &value)
{
    return table->Set(key, value);
}

extern "C" COMPUTEDUCK_API Value* HashTableGet(HashTable *table, StrObject *key)
{
    return table->Get(key);
}

extern "C" COMPUTEDUCK_API bool HashTableSetIfFound(HashTable *table, StrObject *key, const Value &value)
{
    bool isSuccess = table->Find(key);
    if (!isSuccess)
        ASSERT("no member named:(%s)", ObjectStringify(key).c_str());
    return table->Set(key, value);
}

extern "C" COMPUTEDUCK_API RefObject *AllocateIndexRefObject(Value *ptr, const Value &v)
{
    return ALLOCATE_INDEX_REF_OBJECT(ptr, v);
}

extern "C" COMPUTEDUCK_API Value* GetUpvalue(uint8_t index)
{
    auto frame = PEEK_CALL_FRAME(1);
    return frame->closure->upvalues[index]->location;
}

extern "C" COMPUTEDUCK_API void SetUpvalue(const Value& value,uint8_t index)
{
    auto frame = PEEK_CALL_FRAME(1);
    auto slot = frame->closure->upvalues[index]->location;
    slot = GetEndOfRefValuePtr(slot);
    *slot = value;
}

extern "C" COMPUTEDUCK_API RefObject *RefUpvalue(uint8_t index)
{
    auto frame = PEEK_CALL_FRAME(1);
   return ALLOCATE_OBJECT(RefObject, frame->closure->upvalues[index]->location);
}

extern "C" COMPUTEDUCK_API RefObject *RefIndexUpvalue(Value* idxValue,uint8_t index)
{
     auto frame = PEEK_CALL_FRAME(1);
    Value *slot = GetEndOfRefValuePtr(frame->closure->upvalues[index]->location);
    return ALLOCATE_INDEX_REF_OBJECT(slot, *idxValue);
}
void TypeSet::Insert(uint8_t type)
{
    m_ValueTypeSet.insert(type);
}

void TypeSet::Insert(const TypeSet *other)
{
    if (other && !other->m_ValueTypeSet.empty())
        m_ValueTypeSet.insert(other->m_ValueTypeSet.begin(), other->m_ValueTypeSet.end());
}

bool TypeSet::IsOnlyTypeOf(uint8_t t)
{
    return m_ValueTypeSet.size() == 1 && m_ValueTypeSet.contains(t);
}

uint8_t TypeSet::GetOnlyType()
{
    return m_ValueTypeSet.begin().operator*();
}

bool TypeSet::IsNotMultiType()
{
    return m_ValueTypeSet.size() == 1;
}

bool TypeSet::IsNone()
{
    return m_ValueTypeSet.size() == 0;
}

size_t TypeSet::Hash()
{
    size_t value = 0;
    for (auto iter : m_ValueTypeSet)
        value ^= std::hash<uint8_t>()(iter);
    return value;
}

std::string GenerateUUID()
{
    std::random_device rd;
    std::mt19937_64 generator(rd());
    std::uniform_int_distribution<uint64_t> dis;

    uint64_t part1 = dis(generator);
    uint64_t part2 = dis(generator);

    std::ostringstream oss;
    oss << std::hex << part1 << part2;
    return oss.str();
}

size_t HashValueList(Value *start, Value *end)
{
    size_t value = 0;
    for (Value *slot = start; slot < end; ++slot)
    {
        if (IS_OBJECT_VALUE(*slot))
            value ^= std::hash<uint8_t>()(TO_OBJECT_VALUE(*slot)->type);
        else
            value ^= std::hash<uint8_t>()(slot->type);
    }
    return value;
}

std::string GenerateFunctionName(const std::string &uuid, size_t returnHash, size_t paramHash)
{
    auto fnName = "function_" + uuid + "_" + std::to_string(returnHash) + "_" + std::to_string(paramHash);
    return fnName;
}

std::string GenerateLocalVarName(int16_t index)
{
    return "localVar_" + std::to_string(index);
}

#endif