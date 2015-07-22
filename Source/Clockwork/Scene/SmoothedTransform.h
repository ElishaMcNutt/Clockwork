

#pragma once

#include "../Scene/Component.h"

namespace Clockwork
{

/// No ongoing smoothing.
static const unsigned SMOOTH_NONE = 0;
/// Ongoing position smoothing.
static const unsigned SMOOTH_POSITION = 1;
/// Ongoing rotation smoothing.
static const unsigned SMOOTH_ROTATION = 2;

/// Transform smoothing component for network updates.
class CLOCKWORK_API SmoothedTransform : public Component
{
    OBJECT(SmoothedTransform);

public:
    /// Construct.
    SmoothedTransform(Context* context);
    /// Destruct.
    ~SmoothedTransform();
    /// Register object factory.
    static void RegisterObject(Context* context);

    /// Update smoothing.
    void Update(float constant, float squaredSnapThreshold);
    /// Set target position in parent space.
    void SetTargetPosition(const Vector3& position);
    /// Set target rotation in parent space.
    void SetTargetRotation(const Quaternion& rotation);
    /// Set target position in world space.
    void SetTargetWorldPosition(const Vector3& position);
    /// Set target rotation in world space.
    void SetTargetWorldRotation(const Quaternion& rotation);

    /// Return target position in parent space.
    const Vector3& GetTargetPosition() const { return targetPosition_; }

    /// Return target rotation in parent space.
    const Quaternion& GetTargetRotation() const { return targetRotation_; }

    /// Return target position in world space.
    Vector3 GetTargetWorldPosition() const;
    /// Return target rotation in world space.
    Quaternion GetTargetWorldRotation() const;

    /// Return whether smoothing is in progress.
    bool IsInProgress() const { return smoothingMask_ != 0; }

protected:
    /// Handle scene node being assigned at creation.
    virtual void OnNodeSet(Node* node);

private:
    /// Handle smoothing update event.
    void HandleUpdateSmoothing(StringHash eventType, VariantMap& eventData);

    /// Target position.
    Vector3 targetPosition_;
    /// Target rotation.
    Quaternion targetRotation_;
    /// Active smoothing operations bitmask.
    unsigned char smoothingMask_;
    /// Subscribed to smoothing update event flag.
    bool subscribed_;
};

}
