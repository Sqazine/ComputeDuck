#include "Chunk.h"
#include "VM.h"
#include "Allocator.h"
#include <format>
#include <iostream>
int32_t main(int argc, const char **argv)
{
    Allocator::GetInstance()->Init();

    Chunk chunk;
    VM vm;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_ADD}; // 1.0 + 2.0 = 3.0
    chunk.constants = {Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{}", POP().Stringify()) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_SUB}; // 1.0 - 2.0 = -1.0
    chunk.constants = {Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{}", POP().Stringify()) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_MUL}; // 1.0 * 2.0 = 2.0
    chunk.constants = {Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{}", POP().Stringify()) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_DIV}; // 1.0 / 2.0 = 0.5
    chunk.constants = {Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{}", POP().Stringify()) << std::endl;

    chunk.opCodes = {OP_CONSTANT, 0, OP_CONSTANT, 1, OP_CONSTANT, 2, OP_ADD, OP_MUL}; // 1.0 + 2.0 * 3.0
    chunk.constants = {Value(3.0f), Value(2.0f), Value(1.0f)};
    std::cout << chunk.Stringify() << std::endl;
    vm.Run(chunk);

    std::cout << std::format("Result:{}", POP().Stringify()) << std::endl;

    Allocator::GetInstance()->Destroy();

    return 0;
}