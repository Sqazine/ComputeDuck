#pragma once
#include "Utils.h"
#include "Object.h"
class COMPUTEDUCK_API VM
{
public:
    VM() = default;
    ~VM() = default;

    // ++ 修改内容
    // void Run(const Chunk &chunk);
    void Run(FunctionObject *fn);
    // -- 修改内容

private:
    void Execute();

    // ++ 删除内容
    // Chunk m_Chunk;
    // -- 删除内容
};