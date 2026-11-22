#include "item_packages.h"
#include "Extension/Customization/item_origins.h"

namespace dingosdk::multiplayer {
std::string item_package(std::string_view asset) noexcept {
    try {
        const auto origin = item_origins::find(mods::item_key(asset));
        if (!origin) return {};
        return map_package_name(origin->folder, origin->version);
    } catch (...) {
        return {};
    }
}
} // namespace dingosdk::multiplayer
