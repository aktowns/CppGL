#include "utils/Resource.hpp"

#include <string>

using namespace std;

tuple<string, string> kv(const string& inp)
{
    const auto v = inp.find('=');
    auto key = inp.substr(0, v);
    auto val = inp.substr(v + 1);

    return tuple<string, string>(key, val);
}

Resource::Resource(const std::string& resource) : Logger("resource")
{
    const auto pathI = resource.find('?');
    if (pathI == string::npos)
    {
        _path = filesystem::path(resource);
        return;
    }

    auto path = resource.substr(0, pathI);
    auto query = resource.substr(pathI + 1);

    _path = path;

    size_t start = 0U;
    auto end = query.find('&');

    if (end == string::npos)
    {
        auto[key, value] = kv(query);
        _opts[key] = value;
    }

    while (end != string::npos)
    {
        auto substr = query.substr(start, end - start);
        auto[key, value] = kv(substr);

        _opts[key] = value;

        start = end + 1;
        end = query.find('&', start);
    }
}
