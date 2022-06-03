#include <text/logger.h>

#ifndef USE_SPDLOG
::mel::text::level::ELevel mel::text::level::_private::sLevel =
    ::mel::text::level::ELevel::critical;
#endif
void mel::text::set_level( mel::text::level::ELevel level )
{
#ifdef USE_SPDLOG
    spdlog::level::level_enum lvl;
    switch ( level )
    {
    case ::mel::text::level::debug:
        lvl = spdlog::level::debug;
        break;
    case ::mel::text::level::info:
        lvl = spdlog::level::info;
        break;
    case ::mel::text::level::warn:
        lvl = spdlog::level::warn;
        break;
    case ::mel::text::level::err:
        lvl = spdlog::level::err;
        break;
    case ::mel::text::level::critical:
        lvl = spdlog::level::critical;
        break;
    }
    spdlog::set_level( lvl );
#else
    level::_private::sLevel = level;
#endif
}
::mel::text::level::ELevel mel::text::get_level()
{
#ifdef USE_SPDLOG
    ::mel::text::level::ELevel result;
    auto lvl = spdlog::get_level();
    switch ( lvl )
    {
    case spdlog::level::debug:
        result = ::mel::text::level::ELevel::debug;
        break;
    case spdlog::level::info:
        result = ::mel::text::level::ELevel::info;
        break;
    case spdlog::level::warn:
        result = ::mel::text::level::ELevel::warn;
        break;
    case spdlog::level::err:
        result = ::mel::text::level::ELevel::err;
        break;
    case spdlog::level::critical:
        result = ::mel::text::level::ELevel::critical;
        break;
    default:
        result = ::mel::text::level::ELevel::debug; //@todo por poner algo, ya
                                                    // que me falta algun nivel
                                                    // que no me importa ahora
    }
    return result;
#else
    return mel::text::level::_private::sLevel;
#endif
}