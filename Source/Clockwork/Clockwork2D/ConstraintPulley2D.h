#pragma once

#include "../Clockwork2D/Constraint2D.h"

namespace Clockwork
{

/// 2D pulley constraint component.
class CLOCKWORK_API ConstraintPulley2D : public Constraint2D
{
    OBJECT(ConstraintPulley2D);

public:
    /// Construct.
    ConstraintPulley2D(Context* scontext);
    /// Destruct.
    virtual ~ConstraintPulley2D();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Set other body ground anchor point.
    void SetOwnerBodyGroundAnchor(const Vector2& groundAnchor);
    /// Set other body ground anchor point.
    void SetOtherBodyGroundAnchor(const Vector2& groundAnchor);
    /// Set owner body anchor point.
    void SetOwnerBodyAnchor(const Vector2& anchor);
    /// Set other body anchor point.
    void SetOtherBodyAnchor(const Vector2& anchor);
    /// Set ratio.
    void SetRatio(float ratio);

    /// Return owner body ground anchor.
    const Vector2& GetOwnerBodyGroundAnchor() const { return ownerBodyGroundAnchor_; }
    /// return other body ground anchor.
    const Vector2& GetOtherBodyGroundAnchor() const { return otherBodyGroundAnchor_; }
    /// Return owner body anchor.
    const Vector2& GetOwnerBodyAnchor() const { return ownerBodyAnchor_; }
    /// Return other body anchor.
    const Vector2& GetOtherBodyAnchor() const { return otherBodyAnchor_; }
    /// Return ratio.
    float GetRatio() const { return jointDef_.ratio; }


private:
    /// Return Joint def.
    virtual b2JointDef* GetJointDef();

    /// Box2D joint def.
    b2PulleyJointDef jointDef_;
    /// Owner body ground anchor.
    Vector2 ownerBodyGroundAnchor_;
    /// Other body ground anchor.
    Vector2 otherBodyGroundAnchor_;
    /// Owner body anchor.
    Vector2 ownerBodyAnchor_;
    /// Other body anchor.
    Vector2 otherBodyAnchor_;
};

}
