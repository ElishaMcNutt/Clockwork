#pragma once

#include "../Clockwork2D/Constraint2D.h"

namespace Clockwork
{

/// 2D prismatic constraint component.
class CLOCKWORK_API ConstraintPrismatic2D : public Constraint2D
{
    OBJECT(ConstraintPrismatic2D);

public:
    /// Construct.
    ConstraintPrismatic2D(Context* scontext);
    /// Destruct.
    virtual ~ConstraintPrismatic2D();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Set anchor.
    void SetAnchor(const Vector2& anchor);
    /// Set axis.
    void SetAxis(const Vector2& axis);
    /// Set enable limit.
    void SetEnableLimit(bool enableLimit);
    /// Set lower translation.
    void SetLowerTranslation(float lowerTranslation);
    /// Set upper translation.
    void SetUpperTranslation(float upperTranslation);
    /// Set enable motor.
    void SetEnableMotor(bool enableMotor);
    /// Set maxmotor force.
    void SetMaxMotorForce(float maxMotorForce);
    /// Set motor speed.
    void SetMotorSpeed(float motorSpeed);

    /// Return anchor.
    const Vector2& GetAnchor() const { return anchor_; }
    /// Return axis.
    const Vector2& GetAxis() const { return axis_; }
    /// Return enable limit.
    bool GetEnableLimit() const { return jointDef_.enableLimit; }
    /// Return lower translation.
    float GetLowerTranslation() const { return jointDef_.lowerTranslation; }
    /// Return upper translation.
    float GetUpperTranslation() const { return jointDef_.upperTranslation; }
    /// Return enable motor.
    bool GetEnableMotor() const { return jointDef_.enableMotor; }
    /// Return maxmotor force.
    float GetMaxMotorForce() const { return jointDef_.maxMotorForce; }
    /// Return motor speed.
    float GetMotorSpeed() const { return jointDef_.motorSpeed; }

private:
    /// Return joint def.
    virtual b2JointDef* GetJointDef();

    /// Box2D joint def.
    b2PrismaticJointDef jointDef_;
    /// Anchor.
    Vector2 anchor_;
    /// Axis.
    Vector2 axis_;
};

}
