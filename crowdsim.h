#pragma once

#include <Urho3D/Core/CoreEvents.h>
#include <Urho3D/Core/ProcessUtils.h>
#include <Urho3D/Engine/Engine.h>
#include <Urho3D/Graphics/AnimatedModel.h>
#include <Urho3D/Graphics/AnimationController.h>
#include <Urho3D/Graphics/Camera.h>
#include <Urho3D/Graphics/Light.h>
#include <Urho3D/Graphics/Material.h>
#include <Urho3D/Graphics/Octree.h>
#include <Urho3D/Graphics/Renderer.h>
#include <Urho3D/Graphics/Zone.h>
#include <Urho3D/Input/Controls.h>
#include <Urho3D/Input/Input.h>
#include <Urho3D/Graphics/Terrain.h>
#include <Urho3D/IO/FileSystem.h>
#include <Urho3D/Physics/CollisionShape.h>
#include <Urho3D/Physics/PhysicsWorld.h>
#include <Urho3D/Physics/RigidBody.h>
#include <Urho3D/Resource/ResourceCache.h>
#include <Urho3D/Scene/Scene.h>
#include <Urho3D/UI/Font.h>
#include <Urho3D/UI/Text.h>
#include <Urho3D/UI/UI.h>
#include <Urho3D/Engine/Console.h>
#include <Urho3D\Graphics\Skybox.h>
#include <Urho3D/Navigation/CrowdAgent.h>
#include <Urho3D/Navigation/DynamicNavigationMesh.h>
#include <Urho3D/Navigation/Navigable.h>
#include <Urho3D/Navigation/NavigationEvents.h>
#include <Urho3D/Navigation/Obstacle.h>

#include "Character.h"
#include "crowds.h"
#include "Touch.h"

#include <Urho3D/DebugNew.h>
#include "Sample.h"

namespace Urho3D
{

class Node;
class Scene;

}

class Character;
class Touch;

class crowdsim : public Sample
{
    URHO3D_OBJECT(crowdsim, Sample);

public:
    /// Construct.
    crowdsim(Context* context);
    /// Destruct.
    ~crowdsim();

    /// Setup after engine initialization and before running the main loop.
    virtual void Start();

private:
    /// Create static scene content.
    void CreateScene();
    /// Subscribe to necessary events.
    void SubscribeToEvents();
    /// Handle application update. Set controls to character.
    void HandleUpdate(StringHash eventType, VariantMap& eventData);
    /// Handle application post-update. Update camera position after character has moved.
    void HandlePostUpdate(StringHash eventType, VariantMap& eventData);

	void UpdateStreaming();

	void SaveNavigationData();

	void HandleFormationFailure(StringHash eventType, VariantMap& eventData);
	void HandleFormationReposition(StringHash eventType, VariantMap& eventData); // unused for now, for animation purposes if needed
	void HandleFormationSuccess(StringHash eventType, VariantMap& eventData);

	void SaveScene();
	
	void LoadScene();

	void CreateAgents();
	
	void CreateNavScene();

	void SetPathPoint(); // temp

	bool RayCast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable); // temp

    /// Touch utility object.
    SharedPtr<Touch> touch_;
    /// The controllable character component.
    WeakPtr<Character> character_;
    /// First person camera flag.
    bool firstPerson_;

	crowds cr;

	HashMap<IntVector2, PODVector<unsigned char>> tdata;
	HashSet<IntVector2> activeTiles;

	bool drawDebug_;
};
