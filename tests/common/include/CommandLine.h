#include <core/Singleton.h>
#include <unordered_map>
using std::unordered_map;
#include <string>
using std::string;
#include <optional>
namespace tests
{
    class CommandLine : public ::mel::core::Singleton<CommandLine,true,false>
    {
        friend class ::mel::core::Singleton<CommandLine,true,false>;
        public:
            typedef unordered_map<std::string,std::string> OptionsMap;
            inline const OptionsMap& getOptions() const{ return mOptions;}
            std::optional<std::string> getOption(const string& option) const;
            static void createSingleton(int argc,const char* argv[]);
        private:
            CommandLine(int argc,const char* argv[]);
            OptionsMap mOptions;

            void _parse(int argc,const char* argv[]);
    };
}