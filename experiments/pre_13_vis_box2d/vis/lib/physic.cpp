module;

#include <box2d/box2d.h>

export module vis:physic;

export namespace vis::physics {

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
using ::b2Body_GetLinearVelocity;
using ::b2Body_GetMass;
using ::b2Body_GetPosition;
using ::b2Body_GetRotation;
using ::b2Body_GetTransform;
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