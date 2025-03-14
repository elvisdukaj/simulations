module;

// #define GLM_GTC_INLINE_NAMESPACE to inline glm::gtc into glm
// #define GLM_EXT_INLINE_NAMESPACE to inline glm::ext into glm
// #define GLM_GTX_INLINE_NAMESPACE to inline glm::gtx into glm

#include <entt/entt.hpp>

export module vis:entt;

export namespace vis {

using entt::entity;
using entt::registry;
using entt::view;

} // namespace vis