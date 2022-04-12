#pragma once
#include <MelLibType.h>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#else
#include <iostream>
#endif
#include <string>

/**
 * @file
 * @brief The only sense of this file is as a wrapper to spdlog in order t avoid its use if no installed in any concrete platform
 */
namespace mel
{  
    /**
     * @brief Basic text functionalities
     * 
     */
    namespace text
    {
        namespace level
        {
            enum ELevel {debug = 0,info = 1,err = 2,warn=3,critical=4};
            #ifndef USE_SPDLOG                
            namespace _private
            {
                extern ELevel MEL_API sLevel;
            }
            #endif
        }
        void MEL_API set_level(mel::text::level::ELevel level);    
        level::ELevel MEL_API get_level();
        template<class ...Args> void debug(std::string s,Args&&... args)
        {
            #ifdef USE_SPDLOG
            spdlog::debug(std::move(s),std::forward<Args>(args)...);
            #else
            if ( mel::text::level::_private::sLevel <= level::ELevel::debug )
            {
                std::cout <<"[debug] "  << s;
                (std::cout <<...<< args) << std::endl;
            }
            #endif
            //@todo use format for C++20
        }
        template<class ...Args> void info(std::string s,Args&&... args)
        {
            #ifdef USE_SPDLOG
            spdlog::info(std::move(s),std::forward<Args>(args)...);
            #else
            if ( mel::text::level::_private::sLevel <= level::ELevel::info )
            {
                //por poner algo por ahora
                //std::cout <<"[info] "  << s << std::endl;
                std::cout <<"[info] "  << s;
                (std::cout <<...<< args) << std::endl;
            }
            #endif
            //@todo use format for C++20
        }
        template<class ...Args> void error(std::string s,Args&&... args)
        {
            #ifdef USE_SPDLOG
            spdlog::error(std::move(s),std::forward<Args>(args)...);
            #else
            if ( mel::text::level::_private::sLevel <= level::ELevel::err )
            {
                std::cout <<"[error] "  << s;
                (std::cout <<...<< args) << std::endl;
            }
            #endif
            //@todo use format for C++20
        }
        template<class ...Args> void warn(std::string s,Args&&... args)
        {
            #ifdef USE_SPDLOG
            spdlog::warn(std::move(s),std::forward<Args>(args)...);
            #else
            if ( mel::text::level::_private::sLevel <= level::ELevel::warn )
            {
                std::cout <<"[warn] "  << s;
                (std::cout <<...<< args) << std::endl;
            }
            #endif

            //@todo use format for C++20
        }
        template<class ...Args> void critical(std::string s,Args&&... args)
        {
            #ifdef USE_SPDLOG
            spdlog::critical(std::move(s),std::forward<Args>(args)...);
            #else
            if ( mel::text::level::_private::sLevel <= level::ELevel::critical )
            {
                std::cout <<"[critical] "  << s;
                (std::cout <<...<< args) << std::endl;
            }
            #endif
            //@todo use format for C++20
        }
    }
}