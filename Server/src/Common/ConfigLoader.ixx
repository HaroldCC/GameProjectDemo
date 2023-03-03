module;

#include <string>
#include <string_view>
#include "toml++/toml.h"

export module common.config;

export namespace Config
{
    class ConfigLoader final
    {
    public:
        ConfigLoader()  = default;
        ~ConfigLoader() = default;

        bool Load(std::string_view fileName)
        {
            auto config = toml::parse_file(fileName);
            if (config.empty())
            {
                return false;
            }

            for (auto &&[k, v] : config)
            {
                _configMap[k.data()] = v.as_string()->value_or("");
            }

            return true;
        }

    private:
        std::map<std::string, std::string> _configMap;
    };
}
