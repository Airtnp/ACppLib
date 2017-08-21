#ifndef SN_BUILTIN_CRTDBG_H
#define SN_BUILTIN_CRTDBG_H

#include "../sn_Config.hpp"

#if defined(SN_CONFIG_COMPILER_MSVC)
#include <stdlib.h>
#include <crtdbg.h>

#define _CRTDBG_MAP_ALLOC

#ifdef _DEBUG
    #define SN_BUILTIN_CRTDBG_NEW new(_NORMAL_BLOCK, __FILE__, __LINE__)
    #define new SN_BUILTIN_CRTDBG_NEW
#endif

#define SN_BUILTIN_CRTDBG_ALLOC(size) _CrtSetBreakAlloc(size)
#define SN_BUILTIN_CRTDBG_BEGIN _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG | _CRTDBG_LEAK_CHECK_DF))
#define SN_BUILTIN_CRTDBG_END _CrtDumpMemoryLeaks()

namespace sn_builtin {
    namespace crtdbg {
        class DbgMemLeak {  
            _CrtMemState m_checkpoint;  
        public:  
            explicit DbgMemLeak() {     
                _CrtMemCheckpoint(&m_checkpoint);   
            };  
           
            ~DbgMemLeak() {  
                _CrtMemState checkpoint;  
                _CrtMemCheckpoint(&checkpoint);  
                _CrtMemState diff;  
                _CrtMemDifference(&diff, &m_checkpoint, &checkpoint);  
                _CrtMemDumpStatistics(&diff);  
                _CrtMemDumpAllObjectsSince(&diff);  
            };  
        };
        
        void crtdbg_exit() {
            (void)_CrtDumpMemoryLeaks();            
        }
    }
}

#define SN_BUILTIN_CRTDBG_REG atexit(&sn_Builtin::crtdbg::crtdbg_exit)

#endif




#endif