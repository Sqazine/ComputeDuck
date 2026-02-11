#ifdef COMPUTEDUCK_BUILD_WITH_LLVM

#include "JitUtils.h"
#include "Utils.h"
#include "Value.h"
#include "Object.h"
#include "Allocator.h"
#include "Table.h"
#include <random>

extern "C" COMPUTE_DUCK_API StrObject *CreateStrObject(const char *v)
{
    return Allocator::GetInstance()->CreateObject<StrObject>(v);
}

extern "C" COMPUTE_DUCK_API ArrayObject *CreateArrayObject(Value *elements, uint32_t size)
{
    return Allocator::GetInstance()->CreateObject<ArrayObject>(elements, size);
}

extern "C" COMPUTE_DUCK_API RefObject *CreateRefObject(Value *pointer)
{
    return Allocator::GetInstance()->CreateObject<RefObject>(pointer);
}

extern "C" COMPUTE_DUCK_API StructObject *CreateStructObject(Table *table)
{
    return Allocator::GetInstance()->CreateObject<StructObject>(table);
}

extern "C" COMPUTE_DUCK_API Value *GetLocalVariableSlot(int16_t index)
{
    return Allocator::GetInstance()->GetLocalVariableSlot(index);
}

extern "C" COMPUTE_DUCK_API Table *CreateTable()
{
    return new Table();
}

extern "C" COMPUTE_DUCK_API bool TableSet(Table *table, StrObject *key, const Value &value)
{
    return table->Set(key, value);
}

extern "C" COMPUTE_DUCK_API bool TableGet(Table *table, StrObject *key, Value &value)
{
    return table->Get(key, value);
}

extern "C" COMPUTE_DUCK_API bool TableSetIfFound(Table *table, StrObject *key, const Value &value)
{
    bool isSuccess = table->Find(key);
    if (!isSuccess)
        ASSERT("no member named:(%s)", ObjectStringify(key).c_str());
    return table->Set(key, value);
}

extern "C" COMPUTE_DUCK_API RefObject *CreateIndexRefObject(Value *ptr, const Value &v)
{
    return Allocator::GetInstance()->CreateIndexRefObject(ptr, v);
}

void TypeSet::Insert(uint8_t type)
{
    m_ValueTypeSet.insert(type);
}

void TypeSet::Insert(const TypeSet *other)
{
    if (other != nullptr && !other->m_ValueTypeSet.empty())
        m_ValueTypeSet.insert(other->m_ValueTypeSet.begin(), other->m_ValueTypeSet.end());
}

bool TypeSet::IsOnly(uint8_t t)
{
    return m_ValueTypeSet.size() == 1 && m_ValueTypeSet.contains(t);
}

uint8_t TypeSet::GetOnly()
{
    if (m_ValueTypeSet.size() == 1)
    {
        for (auto iter : m_ValueTypeSet)
            return iter;
    }

    ASSERT("Unreachedable");
}

bool TypeSet::IsMultiplyType()
{
    return m_ValueTypeSet.size() >= 2;
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