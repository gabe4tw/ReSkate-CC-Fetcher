#pragma once
#include "Engine/Vfs/mod_item_sources.h"
#include "Extension/Multiplayer/Net/protocol.h"
#include <algorithm>
#include <functional>
#include <string>
#include <string_view>
#include <vector>

// The Thunderstore package a worn item comes from, which a player's outfit names for each item so
// that a peer without the mod can tell which package to fetch (reskate.session.outfit's `packages`).
// The mod is the one the last merge found listing the item (mod_item_sources.h), named as a map's
// package is (protocol.h: map_package_name): "Owner-Name-1.2.3". Nothing for the game's own items
// and for mods whose folder is not a package's name (a mod built or copied in by hand).
namespace dingosdk::multiplayer {
inline std::string package_of(const std::vector<mods::ItemSource> &sources, std::string_view asset) {
    const auto owner = mods::item_source_owner(sources, asset);
    if (owner.empty()) return {};
    for (const auto &source : sources)
        if (source.mod == owner) return map_package_name(source.mod, source.version);
    return {};
}
// package_of against the merge's file, through item_origins (re-read when the merge rewrites it).
// Any thread; never throws.
std::string item_package(std::string_view asset) noexcept;
// Names each item's package in an outfit about to be sent, skater then board, by `lookup` (empty for
// none). Once max_outfit_packages distinct ones are named, items of further packages name none; items
// of a package already named still do. The outfit always stays within what a peer accepts: when the
// names would take it past max_appearance_bytes, the last-named packages are left out until it fits,
// since an outfit that does not fit is not sent at all.
inline void attach_packages(Appearance &appearance, const std::function<std::string(std::string_view)> &lookup) {
    std::vector<std::string> named;
    for (auto *recipe : {&appearance.skater, &appearance.board})
        for (auto &item : recipe->items) {
            item.package.clear();
            if (item.asset.empty()) continue;
            auto package = lookup(item.asset);
            if (package.empty() || !valid_map_package(package)) continue;
            if (std::find(named.begin(), named.end(), package) == named.end()) {
                if (named.size() == max_outfit_packages) continue;
                named.push_back(package);
            }
            item.package = std::move(package);
        }
    while (!named.empty() && !valid_appearance(appearance)) {
        for (auto *recipe : {&appearance.skater, &appearance.board})
            for (auto &item : recipe->items)
                if (item.package == named.back()) item.package.clear();
        named.pop_back();
    }
}
} // namespace dingosdk::multiplayer
