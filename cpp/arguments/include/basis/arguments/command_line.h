#pragma once
/**
 * @file command_line.h
 *
 * Only contains CommandLineTypes, to avoid pulling in argparse to the main unit headers
 */
#include <string>
#include <utility>
#include <variant>
#include <vector>

namespace basis::arguments {
/**
 * Wrapper to allow specifying argc+argv, key value pairs, or a vector of strings for arguments.
 */
using CommandLineTypes = std::variant<std::vector<std::pair<std::string, std::string>>, // key value
                                      std::vector<std::string>,
                                      std::pair<int, char const *const *>>; // argc+argv - NOTE: includes program name

} // namespace basis::arguments