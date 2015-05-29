#include "../Core/Context.h"
#include "../Clockwork2D/CollisionCircle2D.h"
#include "../Clockwork2D/PhysicsUtils2D.h"

#include "../DebugNew.h"

namespace Clockwork
{

extern const char* CLOCKWORK2D_CATEGORY;
static const float DEFAULT_CLRCLE_RADIUS(0.01f);

CollisionCircle2D::CollisionCircle2D(Context* context) :
    CollisionShape2D(context),
    radius_(DEFAULT_CLRCLE_RADIUS),
    center_(Vector2::ZERO)
{
    circleShape_.m_radius = DEFAULT_CLRCLE_RADIUS * cachedWorldScale_.x_;
    fixtureDef_.shape = &circleShape_;
}

CollisionCircle2D::~CollisionCircle2D()
{
}

void CollisionCircle2D::RegisterObject(Context* context)
{
    context->RegisterFactory<CollisionCircle2D>(CLOCKWORK2D_CATEGORY);

    ACCESSOR_ATTRIBUTE("Is Enabled", IsEnabled, SetEnabled, bool, true, AM_DEFAULT);
    ACCESSOR_ATTRIBUTE("Radius", GetRadius, SetRadius, float, DEFAULT_CLRCLE_RADIUS, AM_DEFAULT);
    ACCESSOR_ATTRIBUTE("Center", GetCenter, SetCenter, Vector2, Vector2::ZERO, AM_DEFAULT);
    COPY_BASE_ATTRIBUTES(CollisionShape2D);
}

void CollisionCircle2D::SetRadius(float radius)
{
    if (radius == radius_)
        return;

    radius_ = radius;

    RecreateFixture();
    MarkNetworkUpdate();
}

void CollisionCircle2D::SetCenter(const Vector2& center)
{
    if (center == center_)
        return;

    center_ = center;

    RecreateFixture();
    MarkNetworkUpdate();
}

void CollisionCircle2D::SetCenter(float x, float y)
{
    SetCenter(Vector2(x, y));
}

void CollisionCircle2D::ApplyNodeWorldScale()
{
    RecreateFixture();
}

void CollisionCircle2D::RecreateFixture()
{
    ReleaseFixture();

    // Only use scale in x axis for circle
    float worldScale = cachedWorldScale_.x_;
    circleShape_.m_radius = radius_ * worldScale;
    circleShape_.m_p = ToB2Vec2(center_ * worldScale);

    CreateFixture();
}

}