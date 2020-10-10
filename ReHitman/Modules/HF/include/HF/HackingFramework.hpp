/**
 * HF - Hacking Framework (or Happy, who knows)
 * @author DronCode (https://github.com/DronCode)
 * @credits Based on https://github.com/DronCode/ReHitman/blob/master/HM3CoreKill/HM3CoreKill/ck/HM3Function.h
 */
#pragma once

#include <unordered_map>
#include <system_error>
#include <functional>
#include <utility>
#include <memory>
#include <vector>

#include <Windows.h>
#include <Psapi.h>
#include <TlHelp32.h>

#ifndef GET_ADDR_FROM_MEMBER_OR_ENTITY
#define GET_ADDR_FROM_MEMBER_OR_ENTITY(f) (reinterpret_cast<uint32_t>(reinterpret_cast<uint32_t*&>(f)))
#endif

#define LAMBDA(x) (+x)

/**
 * @brief this is my hacker framework
 */
namespace HF
{
    using Byte  = uint8_t;
    using Addr  = uint32_t;
    using Patch = std::vector<Byte>;

    struct X86 final
    {
        static constexpr uint8_t  NOP       = 0x90;
        static constexpr uint8_t  INT3      = 0xCC;
        static constexpr uint8_t  CALL_NEAR = 0xE8;
        static constexpr uint8_t  JMP_NEAR  = 0xE9;
        static constexpr uint16_t CALL_ABS  = 0xFF15;
        static constexpr uint16_t JMP_ABS   = 0xFF25;
        static constexpr uint8_t  PUSH_AD   = 0x60;
        static constexpr uint8_t  PUSH_FD   = 0x9C;
        static constexpr uint8_t  POP_AD    = 0x61;
        static constexpr uint8_t  POP_FD    = 0x9D;
        static constexpr uint8_t  PUSH_EAX  = 0x50;
        static constexpr uint8_t  PUSH_ECX  = 0x51;
        static constexpr uint8_t  PUSH_EDX  = 0x52;
        static constexpr uint8_t  PUSH_EBP  = 0x55;
        static constexpr uint8_t  PUSH_ESI  = 0x56;
        static constexpr uint8_t  PUSH_EDI  = 0x57;
        static constexpr uint8_t  POP_EAX   = 0x58;
        static constexpr uint8_t  POP_ECX   = 0x59;
        static constexpr uint8_t  POP_ESX   = 0x5E;

        static constexpr auto DEFAULT_JMP_SIZE = 0x5;

        using AssemblyPayload = std::vector<uint8_t>;

#pragma pack(push, 1)
        struct NearJump {
            const uint8_t opcode = X86::JMP_NEAR;
            Addr endpoint { 0 };
        };
#pragma pack(pop)

#pragma pack(push, 1)
        struct NearCall {
            const uint8_t opcode = X86::CALL_NEAR;
            Addr endpoint { 0 };
        };
#pragma pack(pop)

        static Addr calculateNearOffset(Addr jumpTo, Addr jumpFrom, uint32_t jumpInstructionOffset = DEFAULT_JMP_SIZE)
        {
            uint32_t res = 0x0;
            res = jumpTo - jumpFrom - jumpInstructionOffset;
            return res;
        }
    };

    namespace Win32
    {
        /**
         * @class VProtect
         * @brief RAII VirtualProtect implementation
         */
        class VProtect final
        {
            uint32_t m_ptr { 0x0 };
            uint32_t m_size { 0x0 };
            uint32_t m_oldMask { 0x0 };
            uint32_t m_newMask { 0x0 };
            bool     m_inited { false };
        public:
            VProtect() = default;

            [[maybe_unused]] VProtect(uint32_t at, uint32_t size, uint32_t newMask)
                    : m_ptr(at)
                    , m_size(size)
                    , m_oldMask(0x0)
                    , m_newMask(newMask)
            {
                VirtualProtect(
                        (LPVOID)m_ptr,
                        (SIZE_T)m_size,
                        (DWORD)m_newMask,
                        (PDWORD)&m_oldMask
                );
                m_inited = true;
            }

            [[maybe_unused]] ~VProtect()
            {
                if (m_inited)
                {
                    VirtualProtect(
                            (LPVOID)m_ptr,
                            (SIZE_T)m_size,
                            (DWORD)m_oldMask,
                            nullptr
                    );
                }
            }

            explicit operator bool() const
            {
                return m_inited;
            }
        };

        class Module
        {
            friend class Process;

            std::string m_name;
            std::string m_path;
            std::uintptr_t m_baseAddr { 0x0 };
            std::uintptr_t m_size { 0x0 };
        public:
            Module(std::string&& name, std::string&& path,  std::uintptr_t base, std::uintptr_t size)
                    : m_name(std::move(name))
                    , m_path(std::move(path))
                    , m_baseAddr(base)
                    , m_size(size)
            {}

            using Ptr = std::shared_ptr<Module>;
            using Ref = std::weak_ptr<Module>;

            [[nodiscard]] const std::string& getName() const { return m_name; }
            [[nodiscard]] std::uintptr_t getBaseAddress() const { return m_baseAddr; }
            [[nodiscard]] std::uintptr_t getSize() const { return m_size; }
        };

        class Process
        {
            std::string m_name;
            DWORD m_pid { 0x0 };
            HANDLE m_handle { nullptr };
            bool m_valid { false };
            std::unordered_map<std::string, Module::Ptr> m_modules;

        public:
            using Ref = std::weak_ptr<Process>;
            using Ptr = std::shared_ptr<Process>;

            Process() = default;

            explicit Process(std::string name)
                    : m_name(std::move(name))
            {
                HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
                PROCESSENTRY32 entry;

                entry.dwSize = sizeof(entry);

                do
                {
                    if (!strcmp(entry.szExeFile, m_name.c_str()))
                    {
                        m_pid = entry.th32ProcessID;
                        m_handle = OpenProcess(PROCESS_ALL_ACCESS, false, m_pid);
                        m_valid = true;
                        break;
                    }
                }
                while (Process32Next(handle, &entry));

                CloseHandle(handle);
                FindModules();
            }

            explicit Process(std::string_view name) : Process(std::string(name)) {}

            ~Process()
            {
                if (m_handle)
                {
                    CloseHandle(m_handle);
                    m_handle = nullptr;
                }
            }

            explicit operator bool() const
            {
                return m_valid;
            }

            [[nodiscard]] bool isValid() const
            {
                return m_valid;
            }

            [[nodiscard]] Module::Ptr getModule(const std::string& name) const
            {
                auto it = m_modules.find(name);
                if (it != std::end(m_modules))
                {
                    return it->second;
                }
                return nullptr;
            }

            [[nodiscard]] Module::Ptr getModule(std::string_view name) const
            {
                //TODO: Rewrite to std::string_view?
                return getModule(std::string(name));
            }

            [[nodiscard]] Module::Ptr getSelfModule() const
            {
                if (!m_valid)
                    return nullptr;

                return m_modules.at(m_name);
            }

            void forEachModule(const std::function<void(const Module::Ptr&)>& predicate)
            {
                if (!predicate)
                {
                    return;
                }

                for (const auto& [_name, mod] : m_modules)
                {
                    predicate(mod);
                }
            }

            size_t readMemory(std::uintptr_t addr, size_t size, uint8_t* buffer) const
            {
                SIZE_T readBytes = 0x0;
                ReadProcessMemory(m_handle, (LPCVOID)addr, (LPVOID)buffer, (SIZE_T)size, &readBytes);
                return readBytes;
            }

            size_t writeMemory(std::uintptr_t addr, size_t size, const uint8_t* buffer)
            {
                SIZE_T writtenBytes = 0x0;
                WriteProcessMemory(m_handle, (LPVOID)addr, (LPCVOID)buffer, (SIZE_T)size, &writtenBytes);
                return writtenBytes;
            }

            bool fillMemory(std::uintptr_t addr, uint8_t byte, size_t count)
            {
                if (!count)
                    return false;

                const size_t kAwaitBytesCount = count * sizeof(uint8_t);

                auto buffer = std::make_unique<char[]>(kAwaitBytesCount);
                for (size_t i = 0; i < count; i++)
                    buffer[i] = byte;

                SIZE_T readyBytes = 0;
                if (!WriteProcessMemory(m_handle, (LPVOID)addr, (LPCVOID)buffer.get(), count * sizeof(uint8_t), &readyBytes))
                {
                    return false;
                }

                return readyBytes == kAwaitBytesCount;
            }

            enum class MemoryPolicy
            {
                Read             = PAGE_READONLY,
                Write            = PAGE_READWRITE,
                Execute          = PAGE_EXECUTE,
                ReadWrite        = PAGE_READWRITE,
                ReadWriteExecute = PAGE_EXECUTE_READWRITE
            };

            template <typename TData = void>
            [[nodiscard]] TData* allocateMemory(size_t size, MemoryPolicy memPolicy = MemoryPolicy::ReadWrite) const
            {
                auto ptr = VirtualAlloc(nullptr, size, MEM_COMMIT | MEM_RESERVE, static_cast<DWORD>(memPolicy));
                if (!ptr)
                    return nullptr;

                return (TData*)ptr;
            }

            template <typename TData = void>
            void freeMemory(TData* ptr)
            {
                if (ptr)
                {
                    VirtualFree((LPVOID)ptr, 0, MEM_RELEASE);
                }
            }

            [[nodiscard]] DWORD getPid() const { return m_pid; }

            static Process::Ptr getSelf()
            {
                static Process::Ptr self = nullptr;
                if (!self)
                {
                    HANDLE handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, GetCurrentProcessId());
                    if (!handle)
                    {
                        return nullptr;
                    }

                    TCHAR processNameBuffer[MAX_PATH];
                    if (!GetModuleFileNameEx(handle, nullptr, processNameBuffer, MAX_PATH))
                    {
                        return nullptr; //TODO: Describe about error via GetLastError()
                    }

                    CloseHandle(handle);

                    auto selfName = std::string(processNameBuffer);
                    self = std::make_shared<Process>(std::move(selfName));
                }

                return self;
            }

        private:
            void FindModules()
            {
                if (!m_valid)
                    return;

                HANDLE handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, m_pid);
                MODULEENTRY32 entry;
                entry.dwSize = sizeof(entry);

                do
                {
                    std::string key { entry.szModule };
                    std::string modName { entry.szModule };
                    std::string modPath { entry.szExePath };
                    m_modules[key] = std::make_shared<Module>(
                            std::move(modName),
                            std::move(modPath),
                            reinterpret_cast<DWORD>(entry.modBaseAddr),
                            static_cast<DWORD>(entry.modBaseSize)
                    );
                }
                while (Module32Next(handle, &entry));
            }
        };
    };

    namespace Memory
    {
        class MaskComparator final
        {
            static constexpr uint8_t kMustBeEqualToken = 'x';
            static constexpr uint8_t kNullTerminator = 0x0;

        public:
            bool operator()(const uint8_t* data, const uint8_t* mask, const char* szmask)
            {
                for (; *szmask; ++szmask, ++data) {
                    if (*szmask == kMustBeEqualToken && *data != *mask)
                    {
                        return false;
                    }
                }

                return *szmask == kNullTerminator;
            }
        };

        class SignatureSearcher
        {
        public:
            template <typename TComparator = MaskComparator>
            static std::uintptr_t find(const Win32::Process& process, std::uintptr_t from, size_t size, const char* signature, const char* mask)
            {
                std::uintptr_t result = 0x0;
                auto buffer = new uint8_t[size];
                /*size_t readBytes = */process.readMemory(from, size, buffer);

                TComparator comparator;

                for (auto i = 0; i < size; i++)
                {
                    if (comparator((const uint8_t*)(buffer + i), (const uint8_t*)signature, mask)) {
                        result = from + i;
                        break;
                    }
                }

                delete[] buffer;
                return result;
            }

            template <typename Functor, typename TComparator = MaskComparator>
            static Functor findFunction(const Win32::Process& process, const Win32::Module::Ptr& module, const char* signature, const char* mask)
            {
                return find(process, module->getBaseAddress(), module->getSize(), signature, mask);
            }
        };
    }

    namespace Hook
    {
        template <typename ClassT>
        class VFHook final
        {
            std::intptr_t m_originalFunction { 0x0 };
            ClassT*  m_instance { nullptr };
            size_t   m_index { 0x0 };
        public:
            VFHook(ClassT* instance, size_t index, std::intptr_t newAddr)
                    : m_instance(instance)
                    , m_index(index)
            {
                setup(newAddr);
            }

            template <typename FinClass, typename Ret, typename... Args>
            VFHook(ClassT* instance, size_t index, Ret(__thiscall FinClass::* member)(Args...))
                : VFHook(instance, index, GET_ADDR_FROM_MEMBER_OR_ENTITY(member))
            {
            }

            template <typename FinClass, typename Ret, typename... Args>
            VFHook(ClassT* instance, size_t index, Ret(__thiscall FinClass::* member)(Args...) const)
                : VFHook(instance, index, GET_ADDR_FROM_MEMBER_OR_ENTITY(member))
            {
            }

            ~VFHook()
            {
                remove();
            }

            [[nodiscard]] std::intptr_t getOriginalPtr() const { return m_originalFunction; }

            void reset(std::intptr_t newAddr)
            {
                setup(newAddr);
            }

            template <typename Functor>
            void reset(Functor target)
            {
                setup(GET_ADDR_FROM_MEMBER_OR_ENTITY(target));
            }

            template <typename Ret, typename... Args>
            auto invoke(Args&&... args)
            {
                typedef Ret(__thiscall* Function_t)(ClassT*, Args&&...);
                auto func = (Function_t)m_originalFunction;
                return func(m_instance, std::forward<Args>(args)...);
            }

        private:
            static std::intptr_t getMethodAddrByIndex(ClassT* instance, size_t index)
            {
                std::intptr_t vftblptr = *reinterpret_cast<std::intptr_t*>(instance);
                std::intptr_t entity = vftblptr + sizeof(std::intptr_t) * index;
                return *reinterpret_cast<std::intptr_t*>(entity);
            }

            void setup(uint32_t newAddr)
            {
                std::intptr_t vftblptr = *reinterpret_cast<std::intptr_t*>(m_instance);
                std::intptr_t entity = vftblptr + sizeof(std::intptr_t) * m_index;
                m_originalFunction = *reinterpret_cast<std::intptr_t*>(entity);

                {
                    MEMORY_BASIC_INFORMATION mbi;
                    VirtualQuery((LPCVOID)entity, &mbi, sizeof(mbi));

                    Win32::VProtect protect(reinterpret_cast<uint32_t>(mbi.BaseAddress),
                                            mbi.RegionSize,
                                            PAGE_READWRITE);
                    *reinterpret_cast<std::intptr_t*>(entity) = static_cast<std::intptr_t>(newAddr);

//                    auto unprotect = [](void* region) -> int {
//                        MEMORY_BASIC_INFORMATION mbi;
//                        VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
//                        VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);
//                        return mbi.Protect;
//                    };
//
//                    auto protect = [](void* region, int protection) {
//                        MEMORY_BASIC_INFORMATION mbi;
//                        VirtualQuery((LPCVOID)region, &mbi, sizeof(mbi));
//                        VirtualProtect(mbi.BaseAddress, mbi.RegionSize, protection, &mbi.Protect);
//                    };
//
//                    int original_protection = unprotect((void*)entity);
//                    *reinterpret_cast<std::intptr_t*>(entity) = static_cast<std::intptr_t>(newAddr);
//                    protect((void*)entity, original_protection);
                }
            }

            void remove()
            {
                setup(m_originalFunction);
            }
        };

        template <size_t PayloadSize = X86::DEFAULT_JMP_SIZE>
        class Trampoline
        {
            static_assert(PayloadSize >= X86::DEFAULT_JMP_SIZE, "PayloadSize must be grater or equal than 5 (x86 near jmp)");

        private:
            Win32::Process::Ref m_targetProcess;
            Addr m_targetAddr;
            Addr m_trampolineEndpointAddr;
            X86::AssemblyPayload m_payloadBefore;
            X86::AssemblyPayload m_payloadAfter;
            bool m_inited { false };
            size_t m_trampolineAllocatedSpaceSize { 0 };
            uint8_t* m_trampolineBuffer { nullptr };
            uint8_t* m_originalBuffer { nullptr };

        public:
            using Type = Trampoline<PayloadSize>;
            using Ptr = std::shared_ptr<Type>;
            using Ref = std::weak_ptr<Type>;

            Trampoline(
                    const Win32::Process::Ptr& process,
                    std::uintptr_t addr,
                    std::uintptr_t endpoint,
                    X86::AssemblyPayload  payloadBeforeCallEndpoint,
                    X86::AssemblyPayload  payloadAfterEndpointCalled
            )
                : m_targetProcess(process)
                , m_targetAddr(addr)
                , m_trampolineEndpointAddr(endpoint)
                , m_payloadBefore(std::move(payloadBeforeCallEndpoint))
                , m_payloadAfter(std::move(payloadAfterEndpointCalled))
            {
            }

            bool setup()
            {
                if (m_inited)
                    return false;

                auto process = m_targetProcess.lock();
                if (!process)
                    return false;

                m_trampolineAllocatedSpaceSize = PayloadSize + m_payloadBefore.size() + sizeof(X86::NearCall) + m_payloadAfter.size() + sizeof(X86::NearJump);
                m_trampolineBuffer = process->allocateMemory<uint8_t>(m_trampolineAllocatedSpaceSize, Win32::Process::MemoryPolicy::ReadWriteExecute);

                if (!m_trampolineBuffer)
                {
                    return false;
                }

                // So, here we will generate your inlined trampoline
                // First of all we should copy original bytes (opcodes) from place where will be located our JMP trampoline
                m_originalBuffer = reinterpret_cast<uint8_t*>(malloc(PayloadSize));

                if (process->readMemory(m_targetAddr, PayloadSize, &m_originalBuffer[0]) != PayloadSize)
                {
                    // We have not enough bytes. Something goes wrong
                    return false;
                }

                auto pTrampolineHead = reinterpret_cast<std::intptr_t>(&m_trampolineBuffer[0]); /// Pointer to actual position in the buffer

                // So, we have an original bytes here. After that we should allocate the body of trampoline
                // First: place original code
                {
                    if (process->writeMemory(pTrampolineHead, PayloadSize, (uint8_t*)&m_originalBuffer[0]) != PayloadSize)
                    {
                        return false;
                    }
                    pTrampolineHead += PayloadSize;
                }

                // Second: place user pre-payload
                if (!m_payloadBefore.empty())
                {
                    if (process->writeMemory(pTrampolineHead, m_payloadBefore.size(), m_payloadBefore.data()) != m_payloadBefore.size())
                    {
                        return false;
                    }
                    pTrampolineHead += m_payloadBefore.size();
                }

                // Third: place CALL instruction
                {
                    X86::NearCall nearCall;
                    const Addr sourceAddr = ((Addr)&m_trampolineBuffer[0]) + PayloadSize + m_payloadBefore.size();
                    nearCall.endpoint = X86::calculateNearOffset(m_trampolineEndpointAddr, sourceAddr);
                    if (process->writeMemory(pTrampolineHead, sizeof(X86::NearCall), (uint8_t*)&nearCall) != sizeof(X86::NearCall))
                    {
                        return false;
                    }
                    pTrampolineHead += sizeof(X86::NearCall);
                }

                // Fourth: place user post-payload
                if (!m_payloadAfter.empty())
                {
                    if (process->writeMemory(pTrampolineHead, m_payloadAfter.size(), m_payloadAfter.data()) != m_payloadAfter.size())
                    {
                        return false;
                    }
                    pTrampolineHead += m_payloadAfter.size();
                }

                // Fifth: place JMP back to original code
                {
                    X86::NearJump jumpBack;
                    jumpBack.endpoint = X86::calculateNearOffset(m_targetAddr + sizeof(X86::NearJump), (Addr)pTrampolineHead);
                    if (process->writeMemory(pTrampolineHead, sizeof(X86::NearJump), (uint8_t*)&jumpBack) != sizeof(X86::NearJump))
                    {
                        return false;
                    }
                }

                // Sixth: fill original entry point by NOPs
                if (!process->fillMemory(m_targetAddr, X86::NOP, PayloadSize))
                {
                    return false;
                }

                // Eighth: place jump instructions to our trampoline
                {
                    X86::NearJump jumpInto;
                    jumpInto.endpoint = X86::calculateNearOffset((Addr)&m_trampolineBuffer[0], (Addr)m_targetAddr);
                    if (process->writeMemory(m_targetAddr, sizeof(X86::NearJump), (uint8_t*)&jumpInto) != sizeof(X86::NearJump))
                    {
                        return false;
                    }
                }

                m_inited = true;
                return true;
            }

            void remove()
            {
                if (m_inited && !m_targetProcess.expired())
                {
                    auto process = m_targetProcess.lock();

                    if (m_originalBuffer)
                    {
                        std::memcpy((void*)m_targetAddr, &m_originalBuffer[0], PayloadSize);
                        std::free(m_originalBuffer);
                        m_originalBuffer = nullptr;
                    }

                    process->freeMemory(m_trampolineBuffer);
                    m_trampolineBuffer = nullptr;
                }

                m_inited = false;
            }

            [[nodiscard]] std::intptr_t at() const { return m_targetAddr; }

            ~Trampoline()
            {
                /**
                 * @note Be carefully here! If your memory was allocated by other way you should take the pointer via get() and free allocated space manually!
                 */
                if (m_inited)
                {
                    remove();
                }
            }

            explicit operator bool() const
            {
                return m_inited && m_trampolineBuffer != nullptr;
            }

            void* get()
            {
                return (void*)m_trampolineBuffer;
            };

            size_t getSize() const
            {
                return m_trampolineAllocatedSpaceSize;
            }
        };

        using TrampolineBasic = Trampoline<X86::DEFAULT_JMP_SIZE>;
        using TrampolineBasicPtr = std::shared_ptr<TrampolineBasic>;
        template <size_t PatchSize> using TrampolinePtr = std::shared_ptr<Trampoline<PatchSize>>;

        /**
         * @fn HookFunction
         * @brief Override control flow to user's function via patching original function
         * @tparam Functor type of function who will receive control from original code
         * @tparam PatchSize size of patch (default 5 bytes but if original assembly doesn't allow that you can specify your custom size in bytes)
         * @param process reference to the process
         * @param addr address of place where base patch will be placed
         * @param to pointer to the function who will receive control flow
         * @param patchBeforeJump custom payload (compiled assembly) will be placed before call final function (@arg to)
         * @param patchAfterReturnFromTrampoline custom payload (compiled assembly) will be placed after final function will be called (@arg to)
         */
        template <typename Functor, size_t PatchSize = X86::DEFAULT_JMP_SIZE>
        static std::shared_ptr<Trampoline<PatchSize>> HookFunction(
                std::shared_ptr<Win32::Process> process,
                std::uintptr_t addr,
                Functor to,
                const X86::AssemblyPayload& patchBeforeJump,
                const X86::AssemblyPayload& patchAfterReturnFromTrampoline
        ) requires (PatchSize >= X86::DEFAULT_JMP_SIZE) {
            return std::make_shared<Trampoline<PatchSize>>(
                    process,
                    addr,
                    GET_ADDR_FROM_MEMBER_OR_ENTITY(to),
                    patchBeforeJump,
                    patchAfterReturnFromTrampoline);
        }

        template <typename TargetClass, size_t MethodIndex, typename FinalClass, typename Ret, typename... Args>
        static std::unique_ptr<VFHook<TargetClass>> HookVirtualFunction(TargetClass* instance, Ret(FinalClass::*target)(Args&&...))
        {
            return std::make_unique<VFHook<TargetClass>>(instance, MethodIndex, GET_ADDR_FROM_MEMBER_OR_ENTITY(target));
        }

        template <typename TargetClass, size_t MethodIndex, typename Functor>
        static std::unique_ptr<VFHook<TargetClass>> HookVirtualFunction(TargetClass* instance, Functor target)
        {
            return std::make_unique<VFHook<TargetClass>>(instance, MethodIndex, GET_ADDR_FROM_MEMBER_OR_ENTITY(target));
        }

        static bool FillMemoryByNOPs(const std::shared_ptr<Win32::Process>& process, std::intptr_t addr, size_t size)
        {
            if (!process || !size)
            {
                return false;
            }

            return process->fillMemory(addr, X86::NOP, size);
        }

        template <size_t OpCodesCount>
        static bool MoveInstructions(const std::shared_ptr<Win32::Process>& process, const Addr& from, const Addr& to) requires (OpCodesCount > 0)
        {
            auto buffer = std::make_unique<uint8_t[]>(OpCodesCount);
            if (process->readMemory(from, OpCodesCount, buffer.get()) != OpCodesCount)
                return false;

            if (process->writeMemory(to, OpCodesCount, buffer.get()) != OpCodesCount)
                return false;

            return true;
        }
    }
}