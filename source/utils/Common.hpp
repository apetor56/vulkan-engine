#pragma once

#include <cstdint>
#include <ranges>

namespace utils {

constexpr auto size( const std::ranges::range auto& container ) {
    return static_cast< std::uint32_t >( std::size( container ) );
}

} // namespace utils
