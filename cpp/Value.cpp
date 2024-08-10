#include "Value.h"
#include "Object.h"

Value::Value()
	: type(ValueType::NIL), object(nullptr)
{
}

Value::Value(bool boolean)
	: stored(boolean), type(ValueType::BOOL)
{
}

Value::Value(StrObject*object)
	: object(object), type(ValueType::STR)
{
}

Value::Value(ArrayObject* object)
    : object(object), type(ValueType::ARRAY)
{
}

Value::Value(RefObject* object)
    : object(object), type(ValueType::REF)
{
}

Value::Value(FunctionObject* object)
    : object(object), type(ValueType::FUNCTION)
{
}

Value::Value(StructObject* object)
    : object(object), type(ValueType::STRUCT)
{
}

Value::Value(BuiltinObject* object)
    : object(object), type(ValueType::BUILTIN)
{
}

Value::Value(ValueType type)
	: type(type), object(nullptr)
{
}
Value ::~Value()
{
}

std::string Value::Stringify(
#ifndef NDEBUG
    bool printChunkIfIsFunction
#endif
) const
{
	switch (type)
	{
	case ValueType::NIL:
		return "nil";
	case ValueType::NUM:
		return std::to_string(stored);
	case ValueType::BOOL:
		return stored == 1.0 ? "true" : "false";
    case ValueType::STR:
	{
        auto strObj = TO_STR_VALUE(*this);
        return TO_STR_VALUE(*this)->value;
	}
    case ValueType::ARRAY:
	{
        auto arrObj = TO_ARRAY_VALUE(*this);
        std::string result = "[";
        if (arrObj->len != 0)
        {
            for (int32_t i = 0; i < arrObj->len; ++i)
                result += arrObj->elements[i].Stringify() + ",";
            result = result.substr(0, result.size() - 1);
        }
        result += "]";
        return result;
	}
	case ValueType::STRUCT:
	{
        std::string result = "struct instance(0x" + PointerAddressToString(object) + "):\n{\n";
        for (const auto& [k, v] : TO_STRUCT_VALUE(*this)->members)
            result += k + ":" + v.Stringify() + "\n";
        result = result.substr(0, result.size() - 1);
        result += "\n}\n";
        return result;
	}
    case ValueType::REF:
		return TO_REF_VALUE(*this)->pointer->Stringify();
    case ValueType::FUNCTION:
	{
        std::string result = "function(0x" + PointerAddressToString(object) + ")";
#ifndef NDEBUG
        if (printChunkIfIsFunction)
        {
            result += ":\n";
            result += TO_FUNCTION_VALUE(*this)->chunk.Stringify();
        }
#endif
        return result;
	}
	case ValueType::BUILTIN:
	{
        std::string vStr;
        if (TO_BUILTIN_VALUE(*this)->Is<BuiltinFn>())
            vStr = "(0x" + PointerAddressToString(object) + ")";
        else if (TO_BUILTIN_VALUE(*this)->Is<NativeData>())
            vStr = "(0x" + PointerAddressToString(TO_BUILTIN_VALUE(*this)->Get<NativeData>().nativeData) + ")";
        else
            vStr = TO_BUILTIN_VALUE(*this)->Get<Value>().Stringify();

        return "Builtin :" + vStr;
	}
	default:
		return "nil";
	}
	return "nil";
}

void Value::Mark() const
{
	if (IS_OBJECT_VALUE(*this))
	{
		this->object->marked=true;
        switch (this->type)
        {
        case ValueType::STR:
        {
            break;
        }
        case ValueType::ARRAY:
        {
            for (int32_t i = 0; i < TO_ARRAY_VALUE(*this)->capacity; ++i)
                TO_ARRAY_VALUE(*this)->elements[i].Mark();
            break;
        }
        case ValueType::STRUCT:
        {
            for (auto& [k, v] : TO_STRUCT_VALUE(*this)->members)
                v.Mark();
            break;
        }
        case ValueType::REF:
        {
            TO_REF_VALUE(*this)->pointer->Mark();
            break;
        }
        case ValueType::FUNCTION:
        {
            for (auto& v : TO_FUNCTION_VALUE(*this)->chunk.constants)
                v.Mark();
            break;
        }
        case ValueType::BUILTIN:
        {
            if (TO_BUILTIN_VALUE(*this)->Is<Value>())
                TO_BUILTIN_VALUE(*this)->Get<Value>().Mark();
            break;
        }
        default:
            break;
        }
	}
	
}
void Value::UnMark() const
{
    if (IS_OBJECT_VALUE(*this))
    {
        this->object->marked = false;
        switch (this->type)
        {
        case ValueType::STR:
        {
            break;
        }
        case ValueType::ARRAY:
        {
            for (int32_t i = 0; i < TO_ARRAY_VALUE(*this)->capacity; ++i)
                TO_ARRAY_VALUE(*this)->elements[i].UnMark();
            break;
        }
        case ValueType::STRUCT:
        {
            for (const auto& [k, v] : TO_STRUCT_VALUE(*this)->members)
                v.UnMark();
            break;
        }
        case ValueType::REF:
        {
            TO_REF_VALUE(*this)->pointer->UnMark();
            break;
        }
        case ValueType::FUNCTION:
        {
            for (auto& v : TO_FUNCTION_VALUE(*this)->chunk.constants)
                v.UnMark();
            break;
        }
        case ValueType::BUILTIN:
        {
            if (TO_BUILTIN_VALUE(*this)->Is<Value>())
                TO_BUILTIN_VALUE(*this)->Get<Value>().UnMark();
            break;
        }
        default:
            break;
        }
    }
}

bool operator==(const Value &left, const Value &right)
{
	if (left.type != right.type)
		return false;

	switch (left.type)
	{
	case ValueType::NIL:
		return IS_NIL_VALUE(right);
	case ValueType::NUM:
	{
		return left.stored == TO_NUM_VALUE(right);
	}
	case ValueType::BOOL:
	{
		return left.stored == TO_BOOL_VALUE(right);
	}
	case ValueType::STR:
	{
        return strcmp(((StrObject* )((left).object))->value , TO_STR_VALUE(right)->value)==0;
	}
    case ValueType::ARRAY:
    {
        if (TO_ARRAY_VALUE(left)->len != TO_ARRAY_VALUE(right)->len)
            return false;
        for (size_t i = 0; i < TO_ARRAY_VALUE(left)->len; ++i)
            if (TO_ARRAY_VALUE(left)->elements[i] != TO_ARRAY_VALUE(right)->elements[i])
                return false;
        return true;
    }
    case ValueType::STRUCT:
    {
        for (const auto& [k1, v1] : TO_STRUCT_VALUE(left)->members)
        {
            auto iter = TO_STRUCT_VALUE(right)->members.find(k1);
            if (iter == TO_STRUCT_VALUE(right)->members.end())
                return false;
        }
        return true;
    }
    case ValueType::REF:
    {
        return *TO_REF_VALUE(left)->pointer == *TO_REF_VALUE(right)->pointer;
    }
    case ValueType::FUNCTION:
    {
            if (TO_FUNCTION_VALUE(left)->chunk.opCodes.size() != TO_FUNCTION_VALUE(right)->chunk.opCodes.size())
                return false;
            if (TO_FUNCTION_VALUE(left)->parameterCount != TO_FUNCTION_VALUE(right)->parameterCount)
                return false;
            if (TO_FUNCTION_VALUE(left)->localVarCount != TO_FUNCTION_VALUE(right)->localVarCount)
                return false;
            for (int32_t i = 0; i < TO_FUNCTION_VALUE(left)->chunk.opCodes.size(); ++i)
                if (TO_FUNCTION_VALUE(left)->chunk.opCodes[i] != TO_FUNCTION_VALUE(right)->chunk.opCodes[i])
                    return false;
            return true;
    }
    case ValueType::BUILTIN:
    {
        if (TO_BUILTIN_VALUE(left)->Is<NativeData>())
        {
            auto thisNd = TO_BUILTIN_VALUE(left)->Get<NativeData>();
            auto otherNd = TO_BUILTIN_VALUE(right)->Get<NativeData>();
            return PointerAddressToString(thisNd.nativeData) == PointerAddressToString(otherNd.nativeData);
        }
        else
            return TO_BUILTIN_VALUE(left)->data.index() == TO_BUILTIN_VALUE(right)->data.index();
    }
	default:
		return false;
	}
}

bool operator!=(const Value &left, const Value &right)
{
	return !(left == right);
}