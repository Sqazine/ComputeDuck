#include "cdopengl.h"
#include "glad/khrplatform.h"
#include "glad/glad.h"
#include "BuiltinManager.h"
#include "Value.h"
#include <cassert>
void RegisterBuiltins()
{
#define REGISTER_VALUE(x) BuiltinManager::GetInstance()->Register(#x, Value((double)(x)));

	REGISTER_VALUE(GL_ARRAY_BUFFER)
	REGISTER_VALUE(GL_STATIC_DRAW)
	REGISTER_VALUE(GL_VERTEX_SHADER)
	REGISTER_VALUE(GL_FRAGMENT_SHADER)
	REGISTER_VALUE(GL_COLOR_BUFFER_BIT)
	REGISTER_VALUE(GL_DEPTH_BUFFER_BIT)
	REGISTER_VALUE(GL_ELEMENT_ARRAY_BUFFER)
	REGISTER_VALUE(GL_FLOAT)
	REGISTER_VALUE(GL_FALSE)
	REGISTER_VALUE(GL_TRUE)
	REGISTER_VALUE(GL_TRIANGLES)
	REGISTER_VALUE(GL_UNSIGNED_INT)

	BuiltinManager::GetInstance()->Register("gladLoadGL", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
				auto ret = gladLoadGL();
				result = Value((float)ret);
				return true; });

	BuiltinManager::GetInstance()->Register("glGenVertexArrays", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && !IS_REF_VALUE(args[1]))
				ASSERT("Not a valid value of glGenVertexArrays(args[0],args[1]).");

			auto count = (uint32_t)TO_NUM_VALUE(args[0]);
			auto ref = (TO_REF_VALUE(args[1])->pointer);

			if(IS_ARRAY_VALUE((*ref)))
			{
				auto& array=TO_ARRAY_VALUE((*ref))->elements;
				std::vector<GLuint> vaos(count);
				glGenVertexArrays(count, vaos.data());
				array.resize(vaos.size());
				for(int32_t i=0;i<array.size();++i)
					array[i]=(double)vaos[i];
				assert(glGetError() == 0);
			}
			else if(IS_NUM_VALUE((*ref)))
			{
				GLuint id = -1;
				glGenVertexArrays(count, &id);
				ref->number = id;
				assert(glGetError() == 0);
			}
			else
					ASSERT("Not a valid value of glGenVertexArrays:args[1].only number array or number is available");
			return false; });

	BuiltinManager::GetInstance()->Register("glBindVertexArray", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]))
				ASSERT("Not a valid value of glGenVertexArrays(args[0],args[1]).");

			auto vao = (GLuint)TO_NUM_VALUE(args[0]);
			glBindVertexArray(vao);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glVertexAttribPointer", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_BUILTIN_VALUE(args[2]) && !IS_BUILTIN_VALUE(args[3]) && !IS_NUM_VALUE(args[4]) && !(IS_REF_VALUE(args[5]) || IS_NIL_VALUE(args[5])))
				ASSERT("Not a valid value of glVertexAttribPointer(args[0],args[1],args[2],args[3],args[4],args[5]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
			auto arg2 = TO_BUILTIN_VALUE(args[2])->value;
			auto arg3 = TO_BUILTIN_VALUE(args[3])->value;
			auto arg4 = (GLuint)TO_NUM_VALUE(args[4]);

			if (IS_NIL_VALUE(args[5])) {
				glVertexAttribPointer(arg0, arg1, (GLenum)arg2.number, (GLboolean)arg3.number, arg4, (void*)0);
			}
			else {
				if (arg2.number == GL_FLOAT)
				{
					auto arg5 = TO_REF_VALUE(args[5])->pointer;
					auto arrArg5 = TO_ARRAY_VALUE((*arg5));


					std::vector<float> rawArg5(arrArg5->elements.size());
					for (int32_t i = 0; i < rawArg5.size(); ++i)
						rawArg5[i] = (float)arrArg5->elements[i].number;

					glVertexAttribPointer(arg0, arg1, (GLenum)arg2.number, (GLboolean)arg3.number, arg4, (void*)rawArg5.data());
				}
				else if (arg2.number == GL_UNSIGNED_INT)
				{
					auto arg5 = TO_REF_VALUE(args[5])->pointer;
					auto arrArg5 = TO_ARRAY_VALUE((*arg5));

					std::vector<uint32_t> rawArg5(arrArg5->elements.size());
					for (int32_t i = 0; i < rawArg5.size(); ++i)
						rawArg5[i] = (uint32_t)arrArg5->elements[i].number;

					glVertexAttribPointer(arg0, arg1, (GLenum)arg2.number, (GLboolean)arg3.number, arg4, (void*)rawArg5.data());
				}
				else if (arg2.number == GL_INT)
				{
					auto arg5 = TO_REF_VALUE(args[5])->pointer;
					auto arrArg5 = TO_ARRAY_VALUE((*arg5));

					std::vector<int32_t> rawArg5(arrArg5->elements.size());
					for (int32_t i = 0; i < rawArg5.size(); ++i)
						rawArg5[i] = (uint32_t)arrArg5->elements[i].number;

					glVertexAttribPointer(arg0, arg1, (GLenum)arg2.number, (GLboolean)arg3.number, arg4, (void*)rawArg5.data());
				}
			}
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glEnableVertexAttribArray", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]))
				ASSERT("Not a valid value of glEnableVertexAttribArray(args[0]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			glEnableVertexAttribArray(arg0);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glGenBuffers", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && !IS_REF_VALUE(args[1]))
				ASSERT("Not a valid value of glGenBuffers(args[0],args[1]).");

			auto count = (uint32_t)TO_NUM_VALUE(args[0]);
			auto ref = (TO_REF_VALUE(args[1])->pointer);

			if (IS_ARRAY_VALUE((*ref)))
			{
				auto& array = TO_ARRAY_VALUE((*ref))->elements;
				std::vector<GLuint> vaos(count);
				glGenBuffers(count, vaos.data());
				array.resize(vaos.size());
				for (int32_t i = 0; i < array.size(); ++i)
					array[i] = (double)vaos[i];
				assert(glGetError() == 0);
			}
			else if (IS_NUM_VALUE((*ref)))
			{
				GLuint id = -1;
				glGenBuffers(count, &id);
				ref->number = id;
				assert(glGetError() == 0);
			}
			else
				ASSERT("Not a valid value of glGenBuffers:args[1].only number array or number is available");

			return false; });

	BuiltinManager::GetInstance()->Register("glBindBuffer", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]))
				ASSERT("Not a valid value of glBindBuffer(args[0],args[1]).");

			auto flag = (GLuint)(TO_BUILTIN_VALUE(args[0])->value).number;
			auto obj = (GLuint)TO_NUM_VALUE(args[1]);
			glBindBuffer(flag, obj);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glBufferData", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_ARRAY_VALUE(args[2]) && !IS_BUILTIN_VALUE(args[3]))
				ASSERT("Not a valid value of glBufferData(args[0],args[1],args[2],args[3]).");

			auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->value).number;
			auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
			auto arg2 = TO_ARRAY_VALUE(args[2]);
			auto arg3 = (GLuint)(TO_BUILTIN_VALUE(args[3])->value).number;

			if (arg0 == GL_ELEMENT_ARRAY_BUFFER)
			{
				std::vector<uint32_t> rawArg2(arg2->elements.size());
				for (int32_t i = 0; i < rawArg2.size(); ++i)
					rawArg2[i] = (uint32_t)arg2->elements[i].number;

				glBufferData(arg0, arg1, (const void*)rawArg2.data(), arg3);
			}
			else
			{
				std::vector<float> rawArg2(arg2->elements.size());
				for (int32_t i = 0; i < rawArg2.size(); ++i)
					rawArg2[i] = (float)arg2->elements[i].number;

				glBufferData(arg0, arg1, (const void*)rawArg2.data(), arg3);
			}
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glCreateShader", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_BUILTIN_VALUE(args[0]))
				ASSERT("Not a valid value of glCreateShader(args[0]).");

			auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->value).number;
			result = (double)glCreateShader(arg0);
			assert(glGetError() == 0);
			return true; });

	BuiltinManager::GetInstance()->Register("glShaderSource", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && IS_NUM_VALUE(args[1]) && !IS_REF_VALUE(args[2]) && !(IS_REF_VALUE(args[3]) || IS_NIL_VALUE(args[3])))
				ASSERT("Not a valid value of glShaderSource(args[0],args[1],args[2],args[3]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
			auto arg2 = TO_STR_VALUE((*(TO_REF_VALUE(args[2])->pointer)))->value.c_str();

			const GLint* arg3 = nullptr;
			if (IS_REF_VALUE(args[3]))
				arg3 = (const GLint*)TO_REF_VALUE(args[3])->pointer;

			glShaderSource(arg0, 1, &arg2, nullptr);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glCompileShader", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]))
				ASSERT("Not a valid value of glCompileShader(args[0]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			glCompileShader(arg0);

			int isSuccess;
			char infoLog[512];
			glGetShaderiv(arg0, GL_COMPILE_STATUS, &isSuccess);
			if (!isSuccess)
			{
				glGetShaderInfoLog(arg0, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << infoLog << std::endl;
			}

			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glCreateProgram", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			result = (double)glCreateProgram();
			assert(glGetError() == 0);
			return true; });

	BuiltinManager::GetInstance()->Register("glAttachShader", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]))
				ASSERT("Not a valid value of glAttachShader(args[0],args[1]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);

			glAttachShader(arg0, arg1);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glLinkProgram", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]))
				ASSERT("Not a valid value of glLinkProgram(args[0]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);

			glLinkProgram(arg0);

			int success;
			char infoLog[512];
			glGetProgramiv(arg0, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(arg0, 512, NULL, infoLog);
				std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
			}

			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glClearColor", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_NUM_VALUE(args[2]) && !IS_NUM_VALUE(args[3]))
				ASSERT("Not a valid value of glCreateShader(args[0],arg[1],arg[2],arg[3]).");

			auto arg0 = (float)TO_NUM_VALUE(args[0]);
			auto arg1 = (float)TO_NUM_VALUE(args[1]);
			auto arg2 = (float)TO_NUM_VALUE(args[2]);
			auto arg3 = (float)TO_NUM_VALUE(args[3]);

			glClearColor(arg0, arg1, arg2, arg3);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glClear", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (IS_BUILTIN_VALUE(args[0]))
			{
				auto arg0 = (GLuint)(TO_BUILTIN_VALUE(args[0])->value.number);
				glClear(arg0);
			}
			else if (IS_NUM_VALUE(args[0]))
			{
				auto arg0 = (GLuint)(TO_NUM_VALUE(args[0]));
				glClear(arg0);
			}
			else
				ASSERT("Not a valid value of glClear(args[0]).");
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glUseProgram", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_NUM_VALUE(args[0]))
				ASSERT("Not a valid value of glUseProgram(args[0]).");

			auto arg0 = (GLuint)TO_NUM_VALUE(args[0]);
			glUseProgram(arg0);
			assert(glGetError() == 0);
			return false; });

	BuiltinManager::GetInstance()->Register("glDrawElements", [&](Value *args, uint8_t argCount, Value &result) -> bool
													{
			if (!IS_BUILTIN_VALUE(args[0]) && !IS_NUM_VALUE(args[1]) && !IS_BUILTIN_VALUE(args[2]) && !(IS_REF_VALUE(args[3]) || IS_NIL_VALUE(args[3])))
				ASSERT("Not a valid value of glDrawElements(args[0],arg[1],arg[2],arg[3]).");

			auto arg0 = (GLenum)TO_BUILTIN_VALUE(args[0])->value.number;
			auto arg1 = (GLuint)TO_NUM_VALUE(args[1]);
			auto arg2 = (GLenum)TO_BUILTIN_VALUE(args[2])->value.number;

			if (IS_NIL_VALUE(args[3]))
			{
				glDrawElements(arg0, arg1, arg2, nullptr);
			}
			else
			{
				auto arg3 = TO_REF_VALUE(args[3])->pointer;
				auto arrArg3 = TO_ARRAY_VALUE((*arg3));

				std::vector<uint32_t> rawArg3(arrArg3->elements.size());
				for (int32_t i = 0; i < rawArg3.size(); ++i)
					rawArg3[i] = (uint32_t)arrArg3->elements[i].number;

				glDrawElements(arg0, arg1, arg2, rawArg3.data());
			}

			assert(glGetError() == 0);
			return false; });
}