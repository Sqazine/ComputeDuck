#pragma once
#include <string>
#include <type_traits>
#include <cfloat>
#include "Utils.h"

#define IS_NIL_VALUE(v) ((v).type == ValueType::NIL)
#define IS_NUM_VALUE(v) ((v).type == ValueType::NUM)
#define IS_BOOL_VALUE(v) ((v).type == ValueType::BOOL)
#define IS_STR_VALUE(v) ((v).type == ValueType::STR)
#define IS_ARRAY_VALUE(v) ((v).type == ValueType::ARRAY)
#define IS_STRUCT(v) ((v).type == ValueType::STRUCT)
#define IS_REF_VALUE(v) ((v).type == ValueType::REF)
#define IS_FUNCTION_VALUE(v) ((v).type == ValueType::FUNCTION)
#define IS_STRUCT_VALUE(v) ((v).type == ValueType::STRUCT)
#define IS_BUILTIN_VALUE(v) ((v).type == ValueType::BUILTIN)

#define IS_OBJECT_VALUE(v) ((v).type >= ValueType::STR)

#define TO_NUM_VALUE(v) ((v).stored)
#define TO_BOOL_VALUE(v) (((v).stored >= DBL_EPSILON) ? true : false)
#define TO_STR_VALUE(v) ((StrObject *)((v).object))
#define TO_ARRAY_VALUE(v) ((ArrayObject *)((v).object))
#define TO_REF_VALUE(v) ((RefObject *)((v).object))
#define TO_FUNCTION_VALUE(v) ((FunctionObject *)((v).object))
#define TO_STRUCT_VALUE(v) ((StructObject *)((v).object))
#define TO_BUILTIN_VALUE(v) ((BuiltinObject *)((v).object))

#define TO_OBJECT_VALUE(v) ((Object *)((v).object))

enum class ValueType : uint8_t
{
	NIL,
	NUM,
	BOOL,
	STR,
	ARRAY,
	STRUCT,
	REF,
	FUNCTION,
	BUILTIN,
};

struct COMPUTE_DUCK_API Value
{
	template <typename T>
	requires(std::is_integral_v<T> || std::is_floating_point_v<T>)
		Value(T number)
		: stored(static_cast<double>(number)), type(ValueType::NUM)
	{
	}
	Value();
	Value(bool boolean);
	Value(struct StrObject *object);
	Value(struct ArrayObject *object);
	Value(struct RefObject *object);
	Value(struct FunctionObject *object);
	Value(struct StructObject *object);
	Value(struct BuiltinObject *object);
	Value(ValueType type);
	~Value();

	std::string Stringify(
#ifndef NDEBUG
		bool printChunkIfIsFunctionObject = false
#endif
	) const;

	void Mark() const;
	void UnMark() const;

	ValueType type;

	union
	{
		double stored;
		struct Object *object{nullptr};
	};
};

bool operator==(const Value &left, const Value &right);
bool operator!=(const Value &left, const Value &right);