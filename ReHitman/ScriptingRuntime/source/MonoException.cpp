#include <MonoException.h>

namespace Hitman::BloodMoney::Scripting {
    static const char* mono_get_string_property(std::string_view psPropertyName, MonoClass* pClass, MonoObject* pInstance) {
        MonoProperty* pMessageProperty = nullptr;
        MonoMethod* pMessageGetter = nullptr;
        MonoString* pMessageStringInstance = nullptr;

        pMessageProperty = mono_class_get_property_from_name(pClass, psPropertyName.data());
        pMessageGetter = mono_property_get_get_method(pMessageProperty);
        pMessageStringInstance = (MonoString*)mono_runtime_invoke(pMessageGetter, pInstance, nullptr, nullptr);
        return mono_string_to_utf8(pMessageStringInstance);
    };

    constexpr std::string_view kEmptyString = "";
    constexpr std::string_view kMessageProperty = "Message";
    constexpr std::string_view kSourceProperty = "Source";
    constexpr std::string_view kStackTraceProperty = "StackTrace";

    MonoException::MonoException(MonoObject *pExceptionObject) : m_pExceptionObject(pExceptionObject) {
        if (pExceptionObject) {
            m_pExceptionClass = mono_object_get_class(pExceptionObject);
        }

        if (m_pExceptionClass) {
            m_pExceptionType = mono_class_get_type(m_pExceptionClass);
        }
    }

    MonoException::operator bool() noexcept {
        return m_pExceptionObject != nullptr && m_pExceptionClass != nullptr && m_pExceptionType != nullptr;
    }

    std::string_view MonoException::GetMessage() const {
        if (!m_pExceptionObject || !m_pExceptionClass) return kEmptyString;
        return mono_get_string_property(kMessageProperty, m_pExceptionClass, m_pExceptionObject);
    }

    std::string_view MonoException::GetTypeName() const {
        if (!m_pExceptionObject || !m_pExceptionType) return kEmptyString;
        return mono_type_get_name(m_pExceptionType);
    }

    std::string_view MonoException::GetSource() const {
        if (!m_pExceptionObject) return kEmptyString;
        return mono_get_string_property(kSourceProperty, m_pExceptionClass, m_pExceptionObject);
    }

    std::string_view MonoException::GetStackTrace() const {
        if (!m_pExceptionObject) return kEmptyString;
        return mono_get_string_property(kStackTraceProperty, m_pExceptionClass, m_pExceptionObject);
    }
}