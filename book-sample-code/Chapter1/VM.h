#pragma once
#include "Utils.h"
#include "Chunk.h"
class COMPUTEDUCK_API VM
{
public:
    VM() = default;
    ~VM() = default;

    void Run(const Chunk &chunk);

private:
    void Execute();

    Chunk m_Chunk;
};