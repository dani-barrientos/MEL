#pragma once
#include <DabalLibType.h>
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#else
#include <iostream>
#endif
#include <string>
/**
 * @brief the only sense of this file is as a wrapper to spdlog in order t oavoid its use if no used in any concrete platform
 */
namespace text
{
    namespace level
    {
        enum ELevel {debug = 0,info = 1,err = 2,warn=3,critical=4};
        #ifndef USE_SPDLOG                
        namespace _private
        {
            extern ELevel DABAL_API sLevel;
        }
        #endif
    }
    void DABAL_API set_level(::text::level::ELevel level);    
    level::ELevel DABAL_API get_level();
    template<class ...Args> void debug(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::debug(std::move(s),std::forward<Args>(args)...);
        #else
        if ( text::level::_private::sLevel <= level::ELevel::debug )
            //por poner algo por ahora
            std::cout <<"[debug] " << s << std::endl;
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void info(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::info(std::move(s),std::forward<Args>(args)...);
        #else
        if ( text::level::_private::sLevel <= level::ELevel::info )
            //por poner algo por ahora
            std::cout <<"[info] "  << s << std::endl;
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void error(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::error(std::move(s),std::forward<Args>(args)...);
        #else
        if ( text::level::_private::sLevel <= level::ELevel::err )
            //por poner algo por ahora
            std::cout <<"[error] "  << s << std::endl;
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void warn(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::warn(std::move(s),std::forward<Args>(args)...);
        #else
        if ( text::level::_private::sLevel <= level::ELevel::warn )
            //por poner algo por ahora
            std::cout <<"[warn] "  << s << std::endl;
        #endif

        //@todo use format for C++20
    }
    template<class ...Args> void critical(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::critical(std::move(s),std::forward<Args>(args)...);
        #else
        if ( text::level::_private::sLevel <= level::ELevel::critical )
            //por poner algo por ahora
            std::cout <<"[critical] "  << s << std::endl;
        #endif
        //@todo use format for C++20
    }
}