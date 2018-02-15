#include "crowds.h"

crowds::crowds() {}

crowds::~crowds() {}

void crowds::Initialise(Scene* sc, ResourceCache* rc) {
	const int numAgents = 1000;
	std::vector<Node*> agents;

	for (int i = 0; i < numAgents; i++) {
		Node* newAgent = sc->CreateChild("Agent_" + i);
		newAgent->SetPosition(Vector3(Random(-10.0f, 10.0f), 0.0f, Random(-10.0f, 10.0f)));
		newAgent->SetScale(0.45f);
		StaticModel* mNewAgent = newAgent->CreateComponent<StaticModel>();
		mNewAgent->SetModel(rc->GetResource<Model>("Models/Jack.mdl"));
		mNewAgent->SetMaterial(rc->GetResource<Material>("Materials/Jack.xml"));

		RigidBody* rbNewAgent = newAgent->CreateComponent<RigidBody>();
		rbNewAgent->SetCollisionLayer(2);

		CollisionShape* csNewAgent = newAgent->CreateComponent<CollisionShape>();
		csNewAgent->SetBox(Vector3::ONE);

		agents.push_back(newAgent);
	}
	
}