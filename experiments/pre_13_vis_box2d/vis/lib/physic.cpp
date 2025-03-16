module;

#include <box2d/box2d.h>
#include <optional>

export module vis:physic;

import std;
import :math;

export namespace vis::physics {

class World;
class ShapeDef;
class Polygon;
struct Circle;

enum class BodyType {
	fixed = 0,
	kinematic = 1,
	dynamic = 2,
};

class RigidBodyDef {
public:
	RigidBodyDef() : def{::b2DefaultBodyDef()} {}

	RigidBodyDef& set_body_type(BodyType type) {
		def.type = static_cast<b2BodyType>(type);
		return *this;
	}

	RigidBodyDef& set_position(vis::vec2 pos) {
		def.position = b2Vec2(pos.x, pos.y);
		return *this;
	}

	explicit operator const b2BodyDef*() const {
		return &def;
	}

private:
	::b2BodyDef def;
};

struct Rotation {
	float cos_angle, sin_angle;
};

struct Transformation {
	vec2 position{};
	vec2 scale{1.0f, 1.0f};
	Rotation rotation{};

	mat4 get_model() const {
		auto model = vis::ext::identity<vis::mat4>();
		model[0][0] = rotation.cos_angle;
		model[1][0] = rotation.sin_angle;
		model[0][1] = -rotation.sin_angle;
		model[1][1] = rotation.cos_angle;
		model[3][0] = position.x;
		model[3][1] = position.y;
		return model;
	}
};

class RigidBody {
public:
	friend class World;

	RigidBody& create_shape(const ShapeDef& shape, const Polygon& polygon);
	RigidBody& create_shape(const ShapeDef& shape, const Circle& circle);

	Transformation get_transform() const {
		Transformation res;
		auto t = b2Body_GetTransform(id);
		res.position = vec2{t.p.x, t.p.y};
		res.rotation = {t.q.c, t.q.s};
		return res;
	}

private:
	RigidBody(const World& world, const RigidBodyDef& def);

	::b2BodyId id;
};

class WorldDef {
public:
	WorldDef() : def{::b2DefaultWorldDef()} {}

	void set_gravity(vec2 g) {
		def.gravity = b2Vec2(g.x, g.y);
	}

	explicit operator const b2WorldDef*() const {
		return &def;
	}

private:
	b2WorldDef def;
};

class World {
public:
	friend std::optional<World> create_world(const WorldDef& world_def);

	World(const World&) = delete;
	World operator=(const World&) = delete;

	World(World&& rhs) : id{rhs.id} {
		rhs.id.index1 = std::numeric_limits<uint16_t>::max();
	}
	World& operator=(World&& rhs) {
		id = rhs.id;
		rhs.id.index1 = std::numeric_limits<uint16_t>::max();
		return *this;
	}

	~World() {
		if (id.index1 == std::numeric_limits<uint16_t>::max()) {
			return;
		}

		b2DestroyWorld(id);
	}

	void step(float time_step, int sub_step_count) const {
		b2World_Step(id, time_step, sub_step_count);
	}

	RigidBody create_body(const RigidBodyDef& def) const {
		return RigidBody{*this, def};
	}

	explicit operator b2WorldId() const {
		return id;
	}

private:
	explicit World(const WorldDef& world_def) : id{b2CreateWorld(static_cast<const b2WorldDef*>(world_def))} {}

private:
	b2WorldId id;
};

std::optional<World> create_world(const WorldDef& world_def) {
	auto w = World{world_def};
	return std::optional<World>{std::move(w)};
}

RigidBody::RigidBody(const World& world, const RigidBodyDef& def) {
	id = b2CreateBody(static_cast<b2WorldId>(world), static_cast<const b2BodyDef*>(def));
}

class Polygon {
public:
	friend Polygon create_box2d(vis::vec2 half_extent);

	explicit operator const b2Polygon*() const {
		return &poly;
	}

private:
	explicit Polygon(b2Polygon poly) : poly{poly} {}

private:
	b2Polygon poly;
};

class ShapeDef {
public:
	ShapeDef() : def{b2DefaultShapeDef()} {}

	ShapeDef& set_restitution(float restitution) {
		def.restitution = restitution;
		return *this;
	}

	explicit operator const b2ShapeDef*() const {
		return &def;
	}

private:
	b2ShapeDef def;
};

Polygon create_box2d(vis::vec2 half_extent) {
	auto poly = ::b2MakeBox(half_extent.x, half_extent.y);
	return Polygon{poly};
}

struct Circle {
	vec2 center{};
	float radius{};

	explicit operator const b2Circle*() const {
		return reinterpret_cast<const b2Circle*>(this);
	}
};

RigidBody& RigidBody::create_shape(const ShapeDef& shape, const Polygon& polygon) {
	b2CreatePolygonShape(id, static_cast<const b2ShapeDef*>(shape), static_cast<const b2Polygon*>(polygon));
	return *this;
}

RigidBody& RigidBody::create_shape(const ShapeDef& shape, const Circle& circle) {
	b2CreateCircleShape(id, static_cast<const b2ShapeDef*>(shape), static_cast<const b2Circle*>(circle));
	return *this;
}

} // namespace vis::physics