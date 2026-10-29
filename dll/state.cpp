#include "state.hpp"
#include <atomic>

namespace state {

// atomic عشان نمنع تعارض بين الخيوط
static std::atomic<lua_State*> g_state{nullptr};

void set(lua_State* L) {
    g_state.store(L, std::memory_order_release);
}

lua_State* get() {
    return g_state.load(std::memory_order_acquire);
}

} // namespace state
