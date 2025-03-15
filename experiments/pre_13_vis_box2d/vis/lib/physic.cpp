module;

#include <box2d/box2d.h>
#include <optional>

export module vis:physic;

import std;
import :math;

export namespace vis::physics {

class world_def {
public:
	world_def() : def{::b2DefaultWorldDef()} {}

	void set_gravity(vec2 g) {
		def.gravity = b2Vec2(g.x, g.y);
	}

	explicit operator const b2WorldDef*() const {
		return &def;
	}

private:
	b2WorldDef def;
};

class world {
public:
	friend std::optional<world> create_world(const world_def& world_def);

	world(const world&) = delete;
	world operator=(const world&) = delete;

	world(world&& rhs) : id{rhs.id} {
		rhs.id.index1 = std::numeric_limits<uint16_t>::max();
	}
	world& operator=(world&& rhs) {
		id = rhs.id;
		rhs.id.index1 = std::numeric_limits<uint16_t>::max();
		return *this;
	}

	~world() {
		if (id.index1 == std::numeric_limits<uint16_t>::max()) {
			return;
		}

		b2DestroyWorld(id);
	}

	void step(float time_step, int sub_step_count) const {
		b2World_Step(id, time_step, sub_step_count);
	}

	b2BodyId create_body(const b2BodyDef* def) const {
		return b2CreateBody(id, def);
	}

private:
	world(const world_def& world_def) : id{b2CreateWorld(static_cast<const b2WorldDef*>(world_def))} {}

private:
	b2WorldId id;
};

std::optional<world> create_world(const world_def& world_def) {
	auto w = world{world_def};
	return std::optional<world>{std::move(w)};
}

// TODO: We want to wrap the underlying library and change the naming

// data types
using ::b2BodyDef;
using ::b2BodyId;
using ::b2BodyType;
using ::b2Circle;
using ::b2Polygon;
using ::b2ShapeDef;
using ::b2Vec2;
using ::b2WorldDef;
using ::b2WorldId;

// functions
using ::b2Body_ApplyForce;
using ::b2Body_GetLinearVelocity;
using ::b2Body_GetLocalCenterOfMass;
using ::b2Body_GetMass;
using ::b2Body_GetPosition;
using ::b2Body_GetRotation;
using ::b2Body_GetTransform;
using ::b2Body_GetWorldCenterOfMass;
using ::b2CreateBody;
using ::b2CreateCircleShape;
using ::b2CreatePolygonShape;
using ::b2CreateWorld;
using ::b2DefaultBodyDef;
using ::b2DefaultShapeDef;
using ::b2DefaultWorldDef;
using ::b2DestroyWorld;
using ::b2MakeBox;
using ::b2MakeSquare;
// using ::b2Make
using ::b2World_Step;

} // namespace vis::physics