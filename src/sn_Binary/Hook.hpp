#ifndef SN_BINARY_HOOK_H
#define SN_BINARY_HOOK_H

#include "../sn_CommonHeader.h"

namespace sn_Binary {
#ifdef SN_ENABLE_WINDOWS_API && defined(_MSC_VER)
    namespace hook {

        namespace detail {
            __declspec(naked, noinline) volatile void hook_pre(
                DWORD callback,
                DWORD argOffset
            ) {
                // [ebp - 0x08] argOffset
                // [ebp - 0x0C] callback
                __asm {
                    // Save regs
                    pushad;
                    // Remember offset for the first argument
                    mov		eax, dword ptr ss:[ebp - 0x08]
                    mov		[eax], ebp;
                    add		[eax], 0x08;
                    /*	
                        Save actual return address on stack, not really needed for
                        hook_pre but needs to have the same side effects as hook_post
                        which needs this
                    */
                    push	dword ptr ss:[ebp + 0x04];
                    // Call callback function
                    call	dword ptr ss:[ebp - 0x0C];
                    // Set esp to point back to our return point
                    add     esp, 0x4;
                    // estore all registers from stack
                    popad;
                    ret;
                }
            }

             __declspec(naked, noinline) volatile void hook_post(
                DWORD callback,
                DWORD argOffset
            ) {
                // [ebp - 0x08] argOffset
                // [ebp - 0x0C] callback
                __asm {
                    // Save regs
                    pushad;
                    // Remember offset for the first argument
                    mov		eax, dword ptr ss:[ebp - 0x08]
                    mov		[eax], ebp;
                    add		[eax], 0x08;
                    // Save actual return address on stack
                    push	dword ptr ss:[ebp + 0x04];
                    // Move callback pointer to eax
                    mov     eax, dword ptr ss:[ebp - 0x0C];
                    // Set new return point
                    mov     dword ptr ss:[ebp + 0x04], eax;
                    // Set esp to point back to our return point
                    add     esp, 0x4;
                    // Restore old eax
                    pop     eax;
                    ret;
                }
            }
        }

        class Hook {
        public:
            using callorder_t = int;
            using callback_t = void(*)();
            static const callorder_t POSTCALL = 0;
            static const callorder_t PRECALL = 1;
            static const std::size_t m_insSize = 4;
        private:
            DWORD m_originRetAddress;
            std::size_t m_numOfArgsBytes;
            void(*)() m_hook;
            callorder_t m_currentCallOrder;
            DWORD m_dwAddress; // address of hooked function
            DWORD m_argOffset;
            
            static const BYTE JMP         = 0xE9,
            static const BYTE RET         = 0xC3,
            static const BYTE PUSH        = 0x68,
            static const BYTE CALL        = 0xE8,
            static const BYTE ADD_ESP_8[3]                = {0x83, 0xC4, 0x8};
            static const BYTE MOVE_EAX_EBP[3]             = {0x8b, 0x45, 0xE8};
            static const BYTE SUB_ESP_NUMOFARGS_BYTES[2]  = {0x2B, 0x25};
            static const BYTE ADD_ESP_NUMOFARGS_BYTES[2]  = {0x03, 0x25};
            static const BYTE PARTIAL_MOV = 0xA3;
            static const BYTE PUSH_EAX    = 0x50;
            static const BYTE POP_EAX     = 0x58;
            
            BYTE* get_func_instructions(
                const HANDLE& hProcess,
                DWORD lpFunction,
                DWORD& numOfInstructions,
                BYTE eof_inst
            ) {
                BYTE inst;
                DWORD size = 0;
                while (true) {
                    // Read bytes
                    ReadProcessMemory(
                        hProcess,
                        reinterpret_cast<const void*>(lpFunction + size),
                        &inst,
                        sizeof(BYTE),
                        NULL
                    );
                    if (inst == eof_inst) {
                        break;
                    }
                    size += sizeof(BYTE);
                }
                BYTE* code = new BYTE[size];
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<const void*>(lpFunction + size),
                    code,
                    size,
                    &numOfInstructions
                );
                return code;
            }

            void hook_function(
                LPCSTR lpModule,
                LPCSTR lpFuncName,
                DWORD lpFunction
            ) {
                // Get address of function
                m_dwAddress = reinterpret_cast<DWORD>(
                    GetProcAddress(
                        GetModuleHandleA(lpModule),
                        lpFuncName
                    )
                );

                BYTE jmp[5] = {
                    JMP,
                    0x00,0x00,0x00,0x00
                };

                // calculate the opcode for the jumps (to - from - 5)
                DWORD dwCalc = reinterpret_cast<DWORD>(lpFunction) - m_dwAddress - (sizeof(DWORD) + 1);

                std::memcpy(&jmp[1], &dwCalc, sizeof(DWORD));

                HANDLE currentProcess = GetCurrentProcess();
                DWORD currentProcessId = GetProcessId(currentProcess);

                BYTE oldCode[sizeof(DWORD) + 1];

                HANDLE hProcess = OpenProcess(
                    PROCESS_VM_READ,
                    FALSE,
                    currentProcessId
                );

                // Get instruction of the call function
                DWORD sizeof_callback;
                BYTE* callback_instructions = get_func_instructions(
                    hProcess,
                    lpFunction,
                    sizeof_callback,
                    RET
                );

                // Read code from hookee (moved to our hooker function)
                ReadProcessMemory(
                    hProcess,
                    reinterpret_cast<const void*>(m_dwAddress),
                    oldCode,
                    sizeof(DWORD) + 1,
                    NULL
                );

                DWORD writeAddr = m_dwAddress;
                // write jump
                WriteProcessMemory(
                    currentProcess,
                    reinterpret_cast<void*>(writeAddr),
                    jmp,
                    sizeof(DWORD) + 1,
                    NULL
                );

                writeAddr = lpFunction;

                // calculate the jump opcode for the jump back to hooked function
                DWORD dwCalcBack = -dwCalc - (sizeof(DWORD) + 1) - (sizeof(DWORD) + 1) - sizeof_callback;

                // write old code into new function
                WriteProcessMemory(
                    currentProcess,
                    reinterpret_cast<void*>(writeAddr),
                    oldCode,
                    sizeof(DWORD) + 1,
                    NULL
                );

                writeAddr += sizeof(DWORD) + 1;
	            
                //Write old callback function data at the new position
	            WriteProcessMemory(
                    currentProcess, 
                    reinterpret_cast<void*>(writeAddr), 
                    callback_instructions, 
                    sizeof_callback, 
                    NULL
                );

                writeAddr += sizeof_callback;
                
                BYTE jmp_back[5] = {
                    JMP,
                    0x00,0x00,0x00,0x00
                };

                // write opcide into buffer
                std::memcpy(&jmp_back[1], &dwCalcBack, sizeof(DWORD));
                
                // write the jump back to the original function
                WriteProcessMemory(
                    currentProcess,
                    reinterpret_cast<void*>(writeAddr),
                    jmp_back,
                    sizeof(DWORD) + 1,
                    NULL
                );

                CloseHandle(hProcess);
            }
            
            callback_t create_callback_wrapper(void (*hook)()) {

                /*
                    Create callback function like so:
                    1.	push eax						1						0
                    2.	push (DWORD)&arg_offset			1 + sizeof(DWORD) = 5	1
                    3.	push (DWORD)hook				1 + sizeof(DWORD) = 5	6
                    4.	call detail::hook_(pre/post)	1 + sizeof(DWORD) = 5	11
                    5.	mov eax, [ebp-0x18]				3						16
                    6.	mov orig_ret_addr, eax			1 + sizeof(DWORD) = 5	19
                    7.	add esp, 0x08					3						24
                    8.	pop eax							1						27
                    9.	ret								1						28
                    RET is never actually reached, but is an end-of-function-marker for getFuncInstructions 
                */

                const DWORD size = 1 + (1+sizeof(DWORD)) + (1 + sizeof(DWORD)) + (1 + sizeof(DWORD)) + 3 + (1 + sizeof(DWORD)) + 1 + 3 + 1;
                
                BYTE* callback_wrapper = new BYTE[size];

                //1. push eax
                callback_wrapper[0] = PUSH_EAX;

                //2. push (DWORD)&arg_offset
                callback_wrapper[1] = PUSH;
                DWORD addr = reinterpret_cast<DWORD>(&m_argOffset);
                std::memcpy(&callback_wrapper[2], &addr, sizeof(DWORD));

                //3. push (DWORD)hook
                callback_wrapper[6] = PUSH;
                std::memcpy(&callback_wrapper[7], &hook, sizeof(DWORD));

                //4. call detail::hook
                callback_wrapper[11] = CALL;
                
                if(m_currentCallOrder == PRECALL) {
                    addr = reinterpret_cast<DWORD>(&detail::hook_pre) - reinterpret_cast<DWORD>(callback_wrapper) - size + 8;
                }
                else {
                    addr = reinterpret_cast<DWORD>(&detail::hook_post) - reinterpret_cast<DWORD>(callback_wrapper) - size + 8;
                }
                std::memcpy(&callback_wrapper[12], &addr, sizeof(DWORD));

                // 5. mov eax, [ebp-0x18]
                std::memcpy(&callback_wrapper[16], MOV_EAX_EBP, sizeof(MOV_EAX_EBP));

                // 6. mov orig_ret_addr, eax
                addr = reinterpret_cast<DWORD>(&m_originRetAddress);
                callback_wrapper[19] = PARTIAL_MOV;
                std::memcpy(&callback_wrapper[20], &addr, sizeof(DWORD));

                // 7. add esp, 0x08
                std::memcpy(&callback_wrapper[24], ADD_ESP_8, sizeof(ADD_ESP_8));

                // 8. pop eax
                callback_wrapper[27] = POP_EAX;

                // 9. Ret
                callback_wrapper[28] = RET;

                // Set permissions to memory area so we can execute our code
                DWORD OldProtect;
                VirtualProtect(
                    callback_wrapper, 
                    size, 
                    PAGE_EXECUTE_READWRITE, 
                    &OldProtect
                );

                return reinterpret_cast<callback_t>(callback_wrapper);

            }

            callback_t create_post_callback_wrapper(void (*hook)()) {
                /*
                    1. sub		esp, numOfArgs_bytes	(So we don't remove the arguments from the stack when we push old eip when we call the hook)
                    2. call		hook;					(Using absoloute call through a pointer)
                    3. add		esp, numOfArgs_bytes	(Restore old state of esp)
                    4. push		orig_ret_addr
                    5. ret
                */

                const DWORD size = (2 + sizeof(DWORD)) + (2 + sizeof(DWORD)) + (2 + sizeof(DWORD)) + (2 + sizeof(DWORD)) + 1;
               
                BYTE* func = new BYTE[size];
                DWORD* hook_ptr = new DWORD;
                *hook_ptr = reinterpret_cast<DWORD>(hook);

                DWORD addr;

                // 1. sub esp, numOfArgs_bytes
                std::memcpy(&func[0], SUB_ESP_NUMOFARGS_BYTES, sizeof(SUB_ESP_NUMOFARGS_BYTES));
                addr = reinterpret_cast<DWORD>(&m_numOfArgsBytes);
                std::memcpy(&func[2], &addr, sizeof(DWORD));

                // 2. call hook;
                func[6] = 0xFF; // Absolute indirect call
                func[7] = 0x15;
                addr = reinterpret_cast<DWORD>(hook_ptr);
                std::memcpy(&func[8], &addr, sizeof(DWORD));

                //3. add esp, numOfArgs_bytes
                std::memcpy(&func[12], ADD_ESP_NUMOFARGS_BYTES, sizeof(ADD_ESP_NUMOFARGS_BYTES));
                addr = reinterpret_cast<DWORD>(&m_numOfArgsBytes);
                std::memcpy(&func[14], &addr, sizeof(DWORD));

                // 4. push orig_ret_addr
                func[18] = 0xFF; // Push value of absolute address on stack
                func[19] = 0x35;
                addr = reinterpret_cast<DWORD>(&m_originRetAddress);
                std::memcpy(&func[20], &addr, sizeof(DWORD));

                //5. ret
                func[24] = RET;

                //Allow execution of the newly created asm
                DWORD OldProtect;
                VirtualProtect(
                    func, 
                    size, 
                    PAGE_EXECUTE_READWRITE, 
                    &OldProtect
                );

                return reinterpret_cast<callback_t>(func);
            }

        public:
            // Get nth argument
            template <typename T>
            __declspec(noinline) volatile void get_arg(int n) {
                // or use inline asm
                return *reinterpret_cast<T*>(m_argOffset + m_insSize * n);
            }

            template <typename T>
            __declspec(noinline) volatile void set_arg(int n, T arg) {
                *reinterpret_cast<T*>(m_argOffset + m_insSize * n)
            }

            template <typename T>
            __declspec(noinline) volatile void set_return_value(T val) {
                __asm {
                    mov eax, val
                }
            }

            void set_hook(
                const char* func, 
                const char* module, 
                const std::size_t numOfArgs,
                const callorder_t callOrder,
                void (*hook)()
            ) {
                m_numOfArgsBytes = numofArgs * m_insSize;
                m_currentCallOrder = callOrder;

                callback_t callback;
                if (m_currentCallOrder == PRECALL) {
                    callback = create_callback_wrapper(hook);
                } else {
                    callback_t midfunc = create_post_callback_wrapper(hook);
                    callback = create_callback_wrapper(midfunc);
                }
                m_hook = callback;

                hook_function(module, func, reinterpret_cast<DWORD>(m_hook));
            }

            void unset_hook() {
                //Optain process and process id of this process
                HANDLE currentProcess = GetCurrentProcess();
                DWORD currentProcessId = GetProcessId(currentProcess);

                // Buffer for the original code to override
                BYTE oldCode[sizeof(DWORD) + 1];

                // Open the process for reading
                HANDLE hProcess = OpenProcess(
                    PROCESS_VM_READ, 
                    FALSE, 
                    currentProcessId);

                // Read legit moved code that was overriden for the jump
                ReadProcessMemory(
                    hProcess, 
                    reinterpret_cast<void*>(m_hook), 
                    oldCode, 
                    sizeof(oldCode), 
                    NULL
                );
                
                // Write legit moved code back into the hooked function instead of jump
                WriteProcessMemory(
                    currentProcess, 
                    reinterpret_cast<void*>(m_dwAddress), 
                    oldCode, 
                    sizeof(oldCode), 
                    NULL
                );

                CloseHandle(hProcess);

                //Clear variables
                m_dwAddress = NULL;
                m_hook = nullptr;
            }

        };
    }
#endif
}