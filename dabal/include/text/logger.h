#pragma once
#ifdef USE_SPDLOG
#include <spdlog/spdlog.h>
#endif
#include <string>
/**
 * @brief the only sense of this file is as a wrapper to spdlog in order t oavoid its use if no used in any concrete platform
 * 
 */
namespace text
{
    template<class ...Args> void debug(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::debug(std::move(s),std::forward<Args>(args)...);
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void info(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::info(std::move(s),std::forward<Args>(args)...);
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void error(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::error(std::move(s),std::forward<Args>(args)...);
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void warn(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::warn(std::move(s),std::forward<Args>(args)...);
        #endif
        //@todo use format for C++20
    }
    template<class ...Args> void critical(std::string s,Args&&... args)
    {
        #ifdef USE_SPDLOG
        spdlog::critical(std::move(s),std::forward<Args>(args)...);
        #endif
        //@todo use format for C++20
    }
}