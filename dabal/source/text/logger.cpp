#include <text/logger.h>

::text::level::ELevel text::level::_private::sLevel = ::text::level::ELevel::critical;
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
    return spdlog::get_level();
    #else
    return text::level::_private::sLevel;
    #endif
}