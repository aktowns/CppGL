#pragma once

#include "fmt/format.h"

#include <filesystem>

template <>
struct fmt::formatter<std::filesystem::path>
{
	template <typename ParseContext>
	constexpr auto parse(ParseContext &ctx) { return ctx.begin(); }

	template <typename FormatContext>
	auto format(const std::filesystem::path& path, FormatContext &ctx)
	{
		return format_to(ctx.out(), "{}", path.string());
	}
};
