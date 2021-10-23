#pragma once

#include <mono/jit/jit.h>
#include <spdlog/fmt/ostr.h>
#include <string_view>

namespace Hitman::BloodMoney::Scripting {
    class MonoException {
    public:
        explicit MonoException(MonoObject* pExceptionObject);

        explicit operator bool() noexcept;

        [[nodiscard]] std::string_view GetMessage()    const;
        [[nodiscard]] std::string_view GetTypeName()   const;
        [[nodiscard]] std::string_view GetSource()     const;
        [[nodiscard]] std::string_view GetStackTrace() const;

        template <typename OStream>
        friend OStream& operator<<(OStream& os, const MonoException& monoException) {
            return os   <<   "Exception   : " << monoException.GetTypeName()
                        << "\nMessage     : " << monoException.GetMessage()
                        << "\nSource      : " << monoException.GetSource()
                        << "\nStack trace :\n" << monoException.GetStackTrace();
        }
    private:
        MonoObject* m_pExceptionObject { nullptr };
        MonoClass* m_pExceptionClass { nullptr };
        MonoType* m_pExceptionType { nullptr };
    };
}