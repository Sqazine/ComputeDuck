#pragma once
#include <string_view>
#include <unordered_map>

    class Context
    {
    public:
        Context();
        Context(Context *upContext);
        ~Context();

        void DefineVariableByName(std::string_view name, struct Object *value);

        void AssignVariableByName(std::string_view name, struct Object *value);
        struct Object *GetVariableByName(std::string_view name);

        void AssignVariableByAddress(std::string_view address, struct Object *value);
        struct Object *GetVariableByAddress(std::string_view address);

    private:
        friend class VM;

        std::unordered_map<std::string, struct Object *> m_Values;
        Context *m_UpContext;
    };