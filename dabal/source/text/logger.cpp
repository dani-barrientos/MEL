#include <text/logger.h>

#ifndef USE_SPDLOG
::text::level::ELevel text::level::_private::sLevel = ::text::level::ELevel::critical;
#endif
void text::set_level(::text::level::ELevel level)
{
#ifdef USE_SPDLOG
    spdlog::level::level_enum lvl;
    switch (level)
    {
    case ::text::level::debug: 
        lvl = spdlog::level::debug;
        break;
    case ::text::level::info: 
        lvl = spdlog::level::info;
        break;
    case ::text::level::warn: 
        lvl = spdlog::level::warn;
        break;
    case ::text::level::err: 
        lvl = spdlog::level::err;
        break;
    case ::text::level::critical: 
        lvl = spdlog::level::critical;
        break;
    }
    spdlog::set_level(lvl);
#else
    level::_private::sLevel = level;
#endif
}
::text::level::ELevel text::get_level()
{
    #ifdef USE_SPDLOG
    ::text::level::ELevel result;
    auto lvl = spdlog::get_level();
    switch (lvl)
    {
    case spdlog::level::debug:
        result = ::text::level::ELevel::debug;
        break;
    case spdlog::level::info:
        result = ::text::level::ELevel::info;
        break;
    case spdlog::level::warn:
        result = ::text::level::ELevel::warn;
        break;        
    case spdlog::level::err:
        result = ::text::level::ELevel::err;
        break;
    case spdlog::level::critical:
        result = ::text::level::ELevel::critical;
        break;
    default:
        result = ::text::level::ELevel::debug; //@todo por poner algo, ya que me falta algun nivel que no me importa ahora
    }
    return result;
    #else
    return text::level::_private::sLevel;
    #endif
}