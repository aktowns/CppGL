#pragma once

#include "Logger.hpp"
#include "utils/Resource.hpp"
#include "utils/FmtExtensions.hpp"

#include <optional>

template <class T>
class Loader final : Logger
{
	std::map<std::string, T*> _cached{};
	std::function<std::optional<T*>(const Resource&, const Console&)> _fetch;
public:
	explicit Loader(std::function<std::optional<T*>(const Resource&, const Console&)> fetch)
		: Logger("loader"), _fetch(fetch) {}

	std::optional<T*> load(std::string path) 
	{
		auto cached = _cached.find(path);
		if (cached == _cached.end())
		{
			console->debug("asset {} not in cache", path);
			auto item = _fetch(Resource(path), console);
			if (item.has_value())
			{
				_cached.insert(std::pair<std::string, T*>(path, item.value()));
			}
			return item;
		}

		return cached->second;
	}

    void clear()
	{
        // TODO: Free
        _cached.clear();
	}
};

