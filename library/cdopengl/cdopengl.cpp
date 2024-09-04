#include "cdopengl.h"
#include "glad/khrplatform.h"
#include "glad/glad.h"
#include "BuiltinManager.h"
#include "Value.h"
#include <cassert>

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(gladLoadGL)(Value *args, uint8_t argCount, Value &result)
{
    result = Value(gladLoadGL());
    return true;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glGenVertexArrays)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && !IS_REF_VALUE(args[1]))
        ASSERT("Invalid value of glGenVertexArrays(args[0],args[1]).");

    auto count = (uint32_t)TO_NUM_VALUE(args[0]);
    auto ref = (TO_REF_VALUE(args[1])->pointer);

    if (IS_ARRAY_VALUE((*ref)))
    {
        auto array = TO_ARRAY_VALUE((*ref));
        std::vector<GLuint> vaos(count);
        glGenVertexArrays(count, vaos.data());
        array->elements = new Value[vaos.size()];
        array->len = vaos.size();
        for (int32_t i = 0; i < array->len; ++i)
            array->elements[i] = (double)vaos[i];
        assert(glGetError() == 0);
    }
    else if (IS_NUM_VALUE((*ref)))
    {
        GLuint id = -1;
        glGenVertexArrays(count, &id);
        ref->stored = id;
        assert(glGetError() == 0);
    }
    else
        ASSERT("Invalid value of glGenVertexArrays:args[1].only number array or number is available");
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glBindVertexArray)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid value of glBindVertexArray(args[0],args[1]).");

    auto vao = (GLuint)TO_NUM_VALUE(args[0]);
    glBindVertexArray(vao);
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glVertexAttribPointer)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_BUILTIN_VALUE(args[2]) && !IS_BUILTIN_VALUE(args[3]) && !IS_NUM_VALUE(args[4]) && !(IS_REF_VALUE(args[5]) || IS_NIL_VALUE(args[5])))
        ASSERT("Invalid value of glVertexAttribPointer(args[0],args[1],args[2],args[3],args[4],args[5]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
    auto arg2 = TO_BUILTIN_VALUE(args[2])->Get<Value>();
    auto arg3 = TO_BUILTIN_VALUE(args[3])->Get<Value>();
    auto arg4 = (GLuint)TO_NUM_VALUE(args[4]);

    if (IS_NIL_VALUE(args[5]))
        glVertexAttribPointer(arg0, arg1, (GLenum)arg2.stored, (GLboolean)arg3.stored, arg4, (void *)0);
    else
    {
        if (arg2.stored == GL_FLOAT)
        {
            auto arg5 = TO_REF_VALUE(args[5])->pointer;
            auto arrArg5 = TO_ARRAY_VALUE((*arg5));

            std::vector<float> rawArg5(arrArg5->len);
            for (int32_t i = 0; i < rawArg5.size(); ++i)
                rawArg5[i] = (float)arrArg5->elements[i].stored;

            glVertexAttribPointer(arg0, arg1, (GLenum)arg2.stored, (GLboolean)arg3.stored, arg4, (void *)rawArg5.data());
        }
        else if (arg2.stored == GL_UNSIGNED_INT)
        {
            auto arg5 = TO_REF_VALUE(args[5])->pointer;
            auto arrArg5 = TO_ARRAY_VALUE((*arg5));

            std::vector<uint32_t> rawArg5(arrArg5->len);
            for (int32_t i = 0; i < rawArg5.size(); ++i)
                rawArg5[i] = (uint32_t)arrArg5->elements[i].stored;

            glVertexAttribPointer(arg0, arg1, (GLenum)arg2.stored, (GLboolean)arg3.stored, arg4, (void *)rawArg5.data());
        }
        else if (arg2.stored == GL_INT)
        {
            auto arg5 = TO_REF_VALUE(args[5])->pointer;
            auto arrArg5 = TO_ARRAY_VALUE((*arg5));

            std::vector<int32_t> rawArg5(arrArg5->len);
            for (int32_t i = 0; i < rawArg5.size(); ++i)
                rawArg5[i] = (uint32_t)arrArg5->elements[i].stored;

            glVertexAttribPointer(arg0, arg1, (GLenum)arg2.stored, (GLboolean)arg3.stored, arg4, (void *)rawArg5.data());
        }
    }
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glEnableVertexAttribArray)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid value of glEnableVertexAttribArray(args[0]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    glEnableVertexAttribArray(arg0);
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glGenBuffers)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && !IS_REF_VALUE(args[1]))
        ASSERT("Invalid value of glGenBuffers(args[0],args[1]).");

    auto count = (uint32_t)TO_NUM_VALUE(args[0]);
    auto ref = (TO_REF_VALUE(args[1])->pointer);

    if (IS_ARRAY_VALUE((*ref)))
    {
        auto array = TO_ARRAY_VALUE((*ref));
        std::vector<GLuint> vaos(count);
        glGenBuffers(count, vaos.data());
        array->elements = new Value[vaos.size()];
        for (int32_t i = 0; i < array->len; ++i)
            array->elements[i] = (double)vaos[i];
        assert(glGetError() == 0);
    }
    else if (IS_NUM_VALUE((*ref)))
    {
        GLuint id = -1;
        glGenBuffers(count, &id);
        ref->stored = id;
        assert(glGetError() == 0);
    }
    else
        ASSERT("Invalid value of glGenBuffers:args[1].only number array or number is available");

    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glBindBuffer)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]))
        ASSERT("Invalid value of glBindBuffer(args[0],args[1]).");

    auto flag = (GLuint)(TO_BUILTIN_VALUE(args[0])->Get<Value>()).stored;
    auto obj = (GLuint)TO_NUM_VALUE(args[1]);
    glBindBuffer(flag, obj);
    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glBufferData)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_ARRAY_VALUE(args[2]) && !IS_BUILTIN_VALUE(args[3]))
        ASSERT("Invalid value of glBufferData(args[0],args[1],args[2],args[3]).");

    auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->Get<Value>()).stored;
    auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
    auto arg2 = TO_ARRAY_VALUE(args[2]);
    auto arg3 = (GLuint)(TO_BUILTIN_VALUE(args[3])->Get<Value>()).stored;

    if (arg0 == GL_ELEMENT_ARRAY_BUFFER)
    {
        std::vector<uint32_t> rawArg2(arg2->len);
        for (int32_t i = 0; i < rawArg2.size(); ++i)
            rawArg2[i] = (uint32_t)arg2->elements[i].stored;

        glBufferData(arg0, arg1, (const void *)rawArg2.data(), arg3);
    }
    else
    {
        std::vector<float> rawArg2(arg2->len);
        for (int32_t i = 0; i < rawArg2.size(); ++i)
            rawArg2[i] = (float)arg2->elements[i].stored;

        glBufferData(arg0, arg1, (const void *)rawArg2.data(), arg3);
    }
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glCreateShader)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]))
        ASSERT("Invalid value of glCreateShader(args[0]).");

    auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->Get<Value>()).stored;
    result = (double)glCreateShader(arg0);
    assert(glGetError() == 0);
    return true;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glShaderSource)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && IS_NUM_VALUE(args[1]) && !IS_REF_VALUE(args[2]) && !(IS_REF_VALUE(args[3]) || IS_NIL_VALUE(args[3])))
        ASSERT("Invalid value of glShaderSource(args[0],args[1],args[2],args[3]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
    auto arg2 = TO_STR_VALUE(*(TO_REF_VALUE(args[2])->pointer))->value;

    glShaderSource(arg0, 1, &arg2, nullptr);
    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glCompileShader)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid value of glCompileShader(args[0]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    glCompileShader(arg0);

    int isSuccess;
    char infoLog[512];
    glGetShaderiv(arg0, GL_COMPILE_STATUS, &isSuccess);
    if (!isSuccess)
    {
        glGetShaderInfoLog(arg0, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }

    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glCreateProgram)(Value *args, uint8_t argCount, Value &result)
{
    result = (double)glCreateProgram();
    assert(glGetError() == 0);
    return true;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glAttachShader)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]))
        ASSERT("Invalid value of glAttachShader(args[0],args[1]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);

    glAttachShader(arg0, arg1);
    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glLinkProgram)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid value of glLinkProgram(args[0]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);

    glLinkProgram(arg0);

    int success;
    char infoLog[512];
    glGetProgramiv(arg0, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(arg0, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
                  << infoLog << std::endl;
    }

    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glClearColor)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_NUM_VALUE(args[2]) && !IS_NUM_VALUE(args[3]))
        ASSERT("Invalid value of glClearColor(args[0],arg[1],arg[2],arg[3]).");

    auto arg0 = (float)TO_NUM_VALUE(args[0]);
    auto arg1 = (float)TO_NUM_VALUE(args[1]);
    auto arg2 = (float)TO_NUM_VALUE(args[2]);
    auto arg3 = (float)TO_NUM_VALUE(args[3]);

    glClearColor(arg0, arg1, arg2, arg3);
    assert(glGetError() == 0);
    return false;
}
extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glClear)(Value *args, uint8_t argCount, Value &result)
{
    if (IS_BUILTIN_VALUE(args[0]))
    {
        auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->Get<Value>().stored);
        glClear(arg0);
    }
    else if (IS_NUM_VALUE(args[0]))
    {
        auto arg0 = (GLuint)(TO_NUM_VALUE(args[0]));
        glClear(arg0);
    }
    else
        ASSERT("Invalid value of glClear(args[0]).");
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glUseProgram)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_NUM_VALUE(args[0]))
        ASSERT("Invalid value of glUseProgram(args[0]).");

    auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
    glUseProgram(arg0);
    assert(glGetError() == 0);
    return false;
}

extern "C" COMPUTE_DUCK_API bool BUILTIN_FN(glDrawElements)(Value *args, uint8_t argCount, Value &result)
{
    if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_BUILTIN_VALUE(args[2]) && !(IS_REF_VALUE(args[3]) || IS_NIL_VALUE(args[3])))
        ASSERT("Invalid value of glDrawElements(args[0],arg[1],arg[2],arg[3]).");

    auto arg0 = (GLenum)TO_BUILTIN_VALUE(args[0])->Get<Value>().stored;
    auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
    auto arg2 = (GLenum)TO_BUILTIN_VALUE(args[2])->Get<Value>().stored;

    if (IS_NIL_VALUE(args[3]))
        glDrawElements(arg0, arg1, arg2, nullptr);
    else
    {
        auto arg3 = TO_REF_VALUE(args[3])->pointer;
        auto arrArg3 = TO_ARRAY_VALUE((*arg3));

        std::vector<uint32_t> rawArg3(arrArg3->len);
        for (int32_t i = 0; i < rawArg3.size(); ++i)
            rawArg3[i] = (uint32_t)arrArg3->elements[i].stored;

        glDrawElements(arg0, arg1, arg2, rawArg3.data());
    }

    assert(glGetError() == 0);
    return false;
}

void RegisterBuiltins()
{
    REGISTER_BUILTIN_VALUE(GL_ARRAY_BUFFER);
    REGISTER_BUILTIN_VALUE(GL_STATIC_DRAW);
    REGISTER_BUILTIN_VALUE(GL_VERTEX_SHADER);
    REGISTER_BUILTIN_VALUE(GL_FRAGMENT_SHADER);
    REGISTER_BUILTIN_VALUE(GL_COLOR_BUFFER_BIT);
    REGISTER_BUILTIN_VALUE(GL_DEPTH_BUFFER_BIT);
    REGISTER_BUILTIN_VALUE(GL_ELEMENT_ARRAY_BUFFER);
    REGISTER_BUILTIN_VALUE(GL_FLOAT);
    REGISTER_BUILTIN_VALUE(GL_FALSE);
    REGISTER_BUILTIN_VALUE(GL_TRUE);
    REGISTER_BUILTIN_VALUE(GL_TRIANGLES);
    REGISTER_BUILTIN_VALUE(GL_UNSIGNED_INT);

    REGISTER_BUILTIN_FN(gladLoadGL);
    REGISTER_BUILTIN_FN(glGenVertexArrays);
    REGISTER_BUILTIN_FN(glBindVertexArray);
    REGISTER_BUILTIN_FN(glVertexAttribPointer);
    REGISTER_BUILTIN_FN(glEnableVertexAttribArray);
    REGISTER_BUILTIN_FN(glGenBuffers);
    REGISTER_BUILTIN_FN(glBindBuffer);
    REGISTER_BUILTIN_FN(glBufferData);
    REGISTER_BUILTIN_FN(glCreateShader);
    REGISTER_BUILTIN_FN(glShaderSource);
    REGISTER_BUILTIN_FN(glCompileShader);
    REGISTER_BUILTIN_FN(glCreateProgram);
    REGISTER_BUILTIN_FN(glAttachShader);
    REGISTER_BUILTIN_FN(glLinkProgram);
    REGISTER_BUILTIN_FN(glClearColor);
    REGISTER_BUILTIN_FN(glClear);
    REGISTER_BUILTIN_FN(glUseProgram);
    REGISTER_BUILTIN_FN(glDrawElements);
}