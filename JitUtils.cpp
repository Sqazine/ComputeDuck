#include "JitUtils.h"
#include "Utils.h"
#include "Value.h"
#include "Object.h"
#include "Allocator.h"
#include "Table.h"

extern "C" COMPUTE_DUCK_API StrObject *CREATE_STR_OBJECT_FN_NAME(const char *v)
{
    return Allocator::GetInstance()->CreateObject<StrObject>(v);
}

extern "C" COMPUTE_DUCK_API ArrayObject *CREATE_ARRAY_OBJECT_FN_NAME(Value *elements, uint32_t size)
{
    return Allocator::GetInstance()->CreateObject<ArrayObject>(elements, size);
}

extern "C" COMPUTE_DUCK_API RefObject *CREATE_REF_OBJECT_FN_NAME(Value *pointer)
{
    return Allocator::GetInstance()->CreateObject<RefObject>(pointer);
}

extern "C" COMPUTE_DUCK_API StructObject *CREATE_STRUCT_OBJECT_FN_NAME(Table *table)
{
    return Allocator::GetInstance()->CreateObject<StructObject>(table);
}

extern "C" COMPUTE_DUCK_API Value *GET_LOCAL_VARIABLE_SLOT_FN_NAME(int16_t scopeDepth, int16_t index, bool isUpValue)
{
    return Allocator::GetInstance()->GetLocalVariableSlot(scopeDepth, index, isUpValue);
}

extern "C" COMPUTE_DUCK_API Table *CREATE_TABLE_FN_NAME()
{
    return new Table();
}

extern "C" COMPUTE_DUCK_API bool TABLE_SET_FN_NAME(Table *table, StrObject *key, const Value &value)
{
    return table->Set(key, value);
}

extern "C" COMPUTE_DUCK_API bool TABLE_GET_FN_NAME(Table *table, StrObject *key, Value &value)
{
    return table->Get(key, value);
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

std::string GenerateLocalVarName(int16_t scopeDepth, int16_t index, int16_t isUpValue)
{
    auto name = "localVar_" + std::to_string(scopeDepth) + "_" + std::to_string(index) + "_" + std::to_string(isUpValue);
    return name;
}
