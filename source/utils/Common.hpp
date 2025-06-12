#pragma once

#include <ranges>

namespace ve::utils {

constexpr auto size( const std::ranges::range auto& container ) {
    return static_cast< uint32_t >( std::size( container ) );
}

} // namespace ve::utils
