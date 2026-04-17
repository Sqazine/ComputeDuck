#pragma once
#include "Utils.h"
#include "Object.h"
class COMPUTEDUCK_API VM
{
public:
    VM() = default;
    ~VM() = default;

    void Run(FunctionObject *fn);

private:
    void Execute();

};