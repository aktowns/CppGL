#pragma once

#include "Logger.hpp"

#include <string>
#include <map>
#include <optional>
#include <filesystem>

class Resource final : Logger
{
	std::map<std::string, std::string> _opts{};
	std::filesystem::path _path;
public:
	explicit Resource(const std::string& resource);
	const std::optional<std::string> operator [] (const std::string& x) const
	{
		auto it = _opts.find(x);
		if (it != _opts.end())
		{
			return it->second;
		} else
		{
			return {};
		}
	}

	const std::filesystem::path& path() const
	{
		return _path;
	}
};

