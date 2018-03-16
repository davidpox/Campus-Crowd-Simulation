#include "crowds.h"

crowds::crowds() {}

crowds::~crowds() {}

void crowds::Initialise(Scene* sc, ResourceCache* rc) {
	const int numAgents = 10;
	std::vector<Node*> agents;

	Node* AgentGroup = sc->CreateChild("AgentGroup");

	for (int i = 0; i < numAgents; i++) {
		Node* newAgent = AgentGroup->CreateChild("Agent_" + i);
		newAgent->SetPosition(Vector3(Random(-5.0f, -25.0f), 0.0f, Random(25.0f, 45.0f)));
		newAgent->SetScale(0.45f);
		StaticModel* mNewAgent = newAgent->CreateComponent<StaticModel>();
		mNewAgent->SetModel(rc->GetResource<Model>("Models/Jack.mdl"));
		mNewAgent->SetMaterial(rc->GetResource<Material>("Materials/Jack.xml"));
		mNewAgent->SetCastShadows(true);

		//RigidBody* rbNewAgent = newAgent->CreateComponent<RigidBody>();
		//rbNewAgent->SetCollisionLayer(2);

		//CollisionShape* csNewAgent = newAgent->CreateComponent<CollisionShape>();
		//csNewAgent->SetBox(Vector3::ONE);

		CrowdAgent* ca = newAgent->CreateComponent<CrowdAgent>();
		ca->SetHeight(2.0f);
		ca->SetRadius(0.1f);
		ca->SetMaxSpeed(Random(0.5f, 2.0f));
		ca->SetMaxAccel(3.0f);




		agents.push_back(newAgent);
	}
	
}