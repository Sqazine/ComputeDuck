#include "Chunk.h"
#include "VM.h"
#include "Allocator.h"
#include <format>
#include <iostream>
int32_t main(int argc, const char **argv)
{
    Chunk chunk;
    VM vm;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD};
    chunk.constants = {Value(1.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{0:f}", POP().stored) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_SUB};
    chunk.constants = {Value(1.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{0:f}", POP().stored) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_MUL};
    chunk.constants = {Value(1.0f), Value(2.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{0:f}", POP().stored) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_DIV};
    chunk.constants = {Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{0:f}", POP().stored) << std::endl;

    return 0;
}