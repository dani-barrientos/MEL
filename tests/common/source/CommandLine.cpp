#include <CommandLine.h>
using tests::CommandLine;
#include <iostream>

void CommandLine::createSingleton( int argc, const char* argv[] )
{
    CommandLine* cl = new CommandLine( argc, argv );
    setObject( cl );
}
CommandLine::CommandLine( int argc, const char* argv[] )
{
    _parse( argc, argv );
}
void CommandLine::_parse( int argc, const char* argv[] )
{
    string lastOption;
    for ( int i = 1; i < argc; ++i )
    {
        const char* val = argv[i];
        if ( val[0] == '-' )
        {
            lastOption = &val[1];
            mOptions[lastOption] = "";
            // std::cout << "Option: "<<lastOption<<'\n';
        }
        else
        {
            if ( !lastOption.empty() )
            {
                mOptions[lastOption] = val;
                //  std::cout << "Option: "<<mOptions[lastOption]<<'\n';
            }
        }
    }
}
std::optional<std::string> CommandLine::getOption( const string& option ) const
{
    auto it = mOptions.find( option );
    if ( it != mOptions.end() )
        return it->second;
    else
        return std::nullopt;
}