
#define APPREGATE(arg, actualArg, line)                 \
    if (IS_NUM_VALUE(arg))                              \
    {                                                   \
        double actualArg = TO_NUM_VALUE(arg);           \
        line;                                           \
    }                                                   \
    else if (IS_BOOL_VALUE(arg))                        \
    {                                                   \
        bool actualArg = TO_BOOL_VALUE(arg);            \
        line;                                           \
    }                                                   \
    else if (IS_STR_VALUE(arg))                         \
    {                                                   \
        StrObject *actualArg = TO_STR_VALUE(arg);       \
        line;                                           \
    }                                                   \
    else if (IS_ARRAY_VALUE(arg))                       \
    {                                                   \
        ArrayObject *actualArg = TO_ARRAY_VALUE(arg);   \
        line;                                           \
    }                                                   \
    else if (IS_STRUCT_VALUE(arg))                      \
    {                                                   \
        StructObject *actualArg = TO_STRUCT_VALUE(arg); \
        line;                                           \
    }

#define APPREGATE2(arg0, arg1, actualArg0, actualArg1, line) \
    if (IS_NUM_VALUE(arg0))                                  \
    {                                                        \
        double actualArg0 = TO_NUM_VALUE(arg0);              \
        APPREGATE(arg1, actualArg1, line)                    \
    }                                                        \
    else if (IS_BOOL_VALUE(arg0))                            \
    {                                                        \
        bool actualArg0 = TO_BOOL_VALUE(arg0);               \
        APPREGATE(arg1, actualArg1, line)                    \
    }                                                        \
    else if (IS_STR_VALUE(arg0))                             \
    {                                                        \
        StrObject *actualArg0 = TO_STR_VALUE(arg0);          \
        APPREGATE(arg1, actualArg1, line)                    \
    }                                                        \
    else if (IS_ARRAY_VALUE(arg0))                           \
    {                                                        \
        ArrayObject *actualArg0 = TO_ARRAY_VALUE(arg0);      \
        APPREGATE(arg1, actualArg1, line)                    \
    }                                                        \
    else if (IS_STRUCT_VALUE(arg0))                          \
    {                                                        \
        StructObject *actualArg0 = TO_STRUCT_VALUE(arg0);    \
        APPREGATE(arg1, actualArg1, line)                    \
    }

#define APPREGATE3(arg0, arg1, arg2, actualArg0, actualArg1, actualArg2, line) \
    if (IS_NUM_VALUE(arg0))                                                    \
    {                                                                          \
        double actualArg0 = TO_NUM_VALUE(arg0);                                \
        APPREGATE2(arg1, arg2, actualArg1, actualArg2, line)                   \
    }                                                                          \
    else if (IS_BOOL_VALUE(arg0))                                              \
    {                                                                          \
        bool actualArg0 = TO_BOOL_VALUE(arg0);                                 \
        APPREGATE2(arg1, arg2, actualArg1, actualArg2, line)                   \
    }                                                                          \
    else if (IS_STR_VALUE(arg0))                                               \
    {                                                                          \
        StrObject *actualArg0 = TO_STR_VALUE(arg0);                            \
        APPREGATE2(arg1, arg2, actualArg1, actualArg2, line)                   \
    }                                                                          \
    else if (IS_ARRAY_VALUE(arg0))                                             \
    {                                                                          \
        ArrayObject *actualArg0 = TO_ARRAY_VALUE(arg0);                        \
        APPREGATE2(arg1, arg2, actualArg1, actualArg2, line)                   \
    }                                                                          \
    else if (IS_STRUCT_VALUE(arg0))                                            \
    {                                                                          \
        StructObject *actualArg0 = TO_STRUCT_VALUE(arg0);                      \
        APPREGATE2(arg1, arg2, actualArg1, actualArg2, line)                   \
    }

#define APPREGATE4(arg0, arg1, arg2, arg3, actualArg0, actualArg1, actualArg2, actualArg3, line) \
    if (IS_NUM_VALUE(arg0))                                                                      \
    {                                                                                            \
        double actualArg0 = TO_NUM_VALUE(arg0);                                                  \
        APPREGATE3(arg1, arg2, arg3, actualArg1, actualArg2, actualArg3, line)                   \
    }                                                                                            \
    else if (IS_BOOL_VALUE(arg0))                                                                \
    {                                                                                            \
        bool actualArg0 = TO_BOOL_VALUE(arg0);                                                   \
        APPREGATE3(arg1, arg2, arg3, actualArg1, actualArg2, actualArg3, line)                   \
    }                                                                                            \
    else if (IS_STR_VALUE(arg0))                                                                 \
    {                                                                                            \
        StrObject *actualArg0 = TO_STR_VALUE(arg0);                                              \
        APPREGATE3(arg1, arg2, arg3, actualArg1, actualArg2, actualArg3, line)                   \
    }                                                                                            \
    else if (IS_ARRAY_VALUE(arg0))                                                               \
    {                                                                                            \
        ArrayObject *actualArg0 = TO_ARRAY_VALUE(arg0);                                          \
        APPREGATE3(arg1, arg2, arg3, actualArg1, actualArg2, actualArg3, line)                   \
    }                                                                                            \
    else if (IS_STRUCT_VALUE(arg0))                                                              \
    {                                                                                            \
        StructObject *actualArg0 = TO_STRUCT_VALUE(arg0);                                        \
        APPREGATE3(arg1, arg2, arg3, actualArg1, actualArg2, actualArg3, line)                   \
    }

#define APPREGATE5(arg0, arg1, arg2, arg3, arg4, actualArg0, actualArg1, actualArg2, actualArg3, actualArg4, line) \
    if (IS_NUM_VALUE(arg0))                                                                                        \
    {                                                                                                              \
        double actualArg0 = TO_NUM_VALUE(arg0);                                                                    \
        APPREGATE4(arg1, arg2, arg3, arg4, actualArg1, actualArg2, actualArg3, actualArg4, line)                   \
    }                                                                                                              \
    else if (IS_BOOL_VALUE(arg0))                                                                                  \
    {                                                                                                              \
        bool actualArg0 = TO_BOOL_VALUE(arg0);                                                                     \
        APPREGATE4(arg1, arg2, arg3, arg4, actualArg1, actualArg2, actualArg3, actualArg4, line)                   \
    }                                                                                                              \
    else if (IS_STR_VALUE(arg0))                                                                                   \
    {                                                                                                              \
        StrObject *actualArg0 = TO_STR_VALUE(arg0);                                                                \
        APPREGATE4(arg1, arg2, arg3, arg4, actualArg1, actualArg2, actualArg3, actualArg4, line)                   \
    }                                                                                                              \
    else if (IS_ARRAY_VALUE(arg0))                                                                                 \
    {                                                                                                              \
        ArrayObject *actualArg0 = TO_ARRAY_VALUE(arg0);                                                            \
        APPREGATE4(arg1, arg2, arg3, arg4, actualArg1, actualArg2, actualArg3, actualArg4, line)                   \
    }                                                                                                              \
    else if (IS_STRUCT_VALUE(arg0))                                                                                \
    {                                                                                                              \
        StructObject *actualArg0 = TO_STRUCT_VALUE(arg0);                                                          \
        APPREGATE4(arg1, arg2, arg3, arg4, actualArg1, actualArg2, actualArg3, actualArg4, line)                   \
    }

//#define APPREGATE6(arg0, arg1, arg2, arg3, arg4, arg5, actualArg0, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line) \
//    if (IS_NUM_VALUE(arg0))                                                                                                          \
//    {                                                                                                                                \
//        double actualArg0 = TO_NUM_VALUE(arg0);                                                                                      \
//        APPREGATE5(arg1, arg2, arg3, arg4, arg5, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line)                   \
//    }                                                                                                                                \
//    else if (IS_BOOL_VALUE(arg0))                                                                                                    \
//    {                                                                                                                                \
//        bool actualArg0 = TO_BOOL_VALUE(arg0);                                                                                       \
//        APPREGATE5(arg1, arg2, arg3, arg4, arg5, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line)                   \
//    }                                                                                                                                \
//    else if (IS_STR_VALUE(arg0))                                                                                                     \
//    {                                                                                                                                \
//        StrObject *actualArg0 = TO_STR_VALUE(arg0);                                                                                  \
//        APPREGATE5(arg1, arg2, arg3, arg4, arg5, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line)                   \
//    }                                                                                                                                \
//    else if (IS_ARRAY_VALUE(arg0))                                                                                                   \
//    {                                                                                                                                \
//        ArrayObject *actualArg0 = TO_ARRAY_VALUE(arg0);                                                                              \
//        APPREGATE5(arg1, arg2, arg3, arg4, arg5, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line)                   \
//    }                                                                                                                                \
//    else if (IS_STRUCT_VALUE(arg0))                                                                                                  \
//    {                                                                                                                                \
//        StructObject *actualArg0 = TO_STRUCT_VALUE(arg0);                                                                            \
//        APPREGATE5(arg1, arg2, arg3, arg4, arg5, actualArg1, actualArg2, actualArg3, actualArg4, actualArg5, line)                   \
//    }

#define RUN(type, ret, initializer)                                                                                                                                                                                                                                             \
    type ret initializer;                                                                                                                                                                                                                                                       \
    if (frame.fn->parameterCount == 1)                                                                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                                           \
        auto arg0 = *(frame.slot + 0);                                                                                                                                                                                                                                          \
        APPREGATE(arg0, actualArg0, ret m_Jit->Run<type>(fnName, std::move(actualArg0)))                                                                                                                                                                                        \
    }                                                                                                                                                                                                                                                                           \
    else if (frame.fn->parameterCount == 2)                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                           \
        auto arg0 = *(frame.slot + 0);                                                                                                                                                                                                                                          \
        auto arg1 = *(frame.slot + 1);                                                                                                                                                                                                                                          \
        APPREGATE2(arg0, arg1, actualArg0, actualArg1, ret m_Jit->Run<type>(fnName, std::move(actualArg0), std::move(actualArg1)))                                                                                                                                              \
    }                                                                                                                                                                                                                                                                           \
    else if (frame.fn->parameterCount == 3)                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                           \
        auto arg0 = *(frame.slot + 0);                                                                                                                                                                                                                                          \
        auto arg1 = *(frame.slot + 1);                                                                                                                                                                                                                                          \
        auto arg2 = *(frame.slot + 2);                                                                                                                                                                                                                                          \
        APPREGATE3(arg0, arg1, arg2, actualArg0, actualArg1, actualArg2, ret m_Jit->Run<type>(fnName, std::move(actualArg0), std::move(actualArg1), std::move(actualArg2)))                                                                                                     \
    }                                                                                                                                                                                                                                                                           \
    else if (frame.fn->parameterCount == 4)                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                           \
        auto arg0 = *(frame.slot + 0);                                                                                                                                                                                                                                          \
        auto arg1 = *(frame.slot + 1);                                                                                                                                                                                                                                          \
        auto arg2 = *(frame.slot + 2);                                                                                                                                                                                                                                          \
        auto arg3 = *(frame.slot + 3);                                                                                                                                                                                                                                          \
        APPREGATE4(arg0, arg1, arg2, arg3, actualArg0, actualArg1, actualArg2, actualArg3, ret m_Jit->Run<type>(fnName, std::move(actualArg0), std::move(actualArg1), std::move(actualArg2), std::move(actualArg3)))                                                            \
    }                                                                                                                                                                                                                                                                           \
    else if (frame.fn->parameterCount == 5)                                                                                                                                                                                                                                     \
    {                                                                                                                                                                                                                                                                           \
        auto arg0 = *(frame.slot + 0);                                                                                                                                                                                                                                          \
        auto arg1 = *(frame.slot + 1);                                                                                                                                                                                                                                          \
        auto arg2 = *(frame.slot + 2);                                                                                                                                                                                                                                          \
        auto arg3 = *(frame.slot + 3);                                                                                                                                                                                                                                          \
        auto arg4 = *(frame.slot + 4);                                                                                                                                                                                                                                          \
        APPREGATE5(arg0, arg1, arg2, arg3, arg4, actualArg0, actualArg1, actualArg2, actualArg3, actualArg4, ret m_Jit->Run<type>(fnName, std::move(actualArg0), std::move(actualArg1), std::move(actualArg2), std::move(actualArg3), std::move(actualArg4)))                   \
    }                                                                                                                                                                                                                                                                           \
    else                                                                                                                                                                                                                                                                        \
        ret m_Jit->Run<type>(fnName);

template <typename T>
inline void VM::ExecuteJitFunction(const CallFrame &frame, const std::string &fnName)
{
    RUN(T, ret =, 0);
    auto prevCallFrame = POP_CALL_FRAME();
    SET_STACK_TOP(prevCallFrame->slot - 1);
    PUSH(ret);
}

template <>
inline void VM::ExecuteJitFunction<void>(const CallFrame &frame, const std::string &fnName)
{
    RUN(void, , );
    auto prevCallFrame = POP_CALL_FRAME();
    SET_STACK_TOP(prevCallFrame->slot - 1);
}