module;

#include <entt/entt.hpp>

export module vis:ecs;

export namespace vis::ecs {
using ::entt::adjacency_matrix;
using ::entt::adl_meta_pointer_like;
using ::entt::allocate_unique;
using ::entt::allocation_deleter;
using ::entt::any;
using ::entt::any_cast;
using ::entt::any_policy;
using ::entt::as_cref_t;
using ::entt::as_group;
using ::entt::as_is_t;
using ::entt::as_ref_t;
using ::entt::as_view;
using ::entt::as_void_t;
using ::entt::basic_any;
using ::entt::basic_collector;
using ::entt::basic_common_view;
using ::entt::basic_continuous_loader;
using ::entt::basic_dispatcher;
using ::entt::basic_entt_traits;
using ::entt::basic_flow;
using ::entt::basic_group;
using ::entt::basic_handle;
using ::entt::basic_hashed_string;
using ::entt::basic_meta_associative_container_traits;
using ::entt::basic_meta_sequence_container_traits;
using ::entt::basic_observer;
using ::entt::basic_organizer;
using ::entt::basic_poly;
using ::entt::basic_reactive_mixin;
using ::entt::basic_registry;
using ::entt::basic_runtime_view;
using ::entt::basic_scheduler;
using ::entt::basic_sigh_mixin;
using ::entt::basic_snapshot;
using ::entt::basic_snapshot_loader;
using ::entt::basic_sparse_set;
using ::entt::basic_storage;
using ::entt::basic_storage_view;
using ::entt::basic_table;
using ::entt::basic_view;
using ::entt::collector;
using ::entt::continuous_loader;
using ::entt::dispatcher;
using ::entt::entity;
using ::entt::entt_traits;
using ::entt::flow;
using ::entt::get;
using ::entt::get_t;
using ::entt::group;
using ::entt::handle;
using ::entt::hashed_string;
using ::entt::input_iterator_pointer;
using ::entt::iota_iterator;
using ::entt::is_iterator;
using ::entt::iterable_adaptor;
using ::entt::meta_associative_container_traits;
using ::entt::meta_range;
using ::entt::meta_sequence_container_traits;
using ::entt::monostate;
using ::entt::null;
using ::entt::null_t;
using ::entt::observer;
using ::entt::organizer;
using ::entt::poly;
using ::entt::reactive_mixin;
using ::entt::registry;
using ::entt::runtime_view;
using ::entt::scheduler;
using ::entt::sigh_mixin;
using ::entt::snapshot;
using ::entt::snapshot_loader;
using ::entt::sparse_set;
using ::entt::storage;
using ::entt::table;
using ::entt::view;

using ::entt::operator""_hws;
using ::entt::operator""_hs;
using ::entt::operator!=;
using ::entt::operator==;
using ::entt::operator<;
using ::entt::operator<=;
using ::entt::operator>;
using ::entt::operator>=;
using ::entt::operator+;
// using ::entt::
} // namespace vis::ecs

export {
	using vis::ecs::operator""_hws;
	using vis::ecs::operator""_hs;
	using vis::ecs::operator!=;
	using vis::ecs::operator==;
	using vis::ecs::operator<;
	using vis::ecs::operator<=;
	using vis::ecs::operator>;
	using vis::ecs::operator>=;
	using vis::ecs::operator+;

	using ::entt::operator!=;
}