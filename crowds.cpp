#include "crowds.h"

crowds::crowds() {}

crowds::~crowds() {}

void crowds::Initialise(Scene* sc, ResourceCache* rc) {
	const int numAgents = 2;
	for (int j = 0; j < 20; j++) {
		Node* AgentGroup = sc->CreateChild("AgentGroup");
		AgentGroup->AddTag("AgentGroup");
		float agentMatchingSpeed = Random(0.6f, 0.75f);
		Vector3 agentMatchingPosition = Vector3(Random(-5.0f, -25.0f), 0.0f, Random(25.0f, 45.0f));

		for (int i = 0; i < numAgents; i++) {
			Node* newAgent = AgentGroup->CreateChild("Agent");
			newAgent->SetPosition(agentMatchingPosition + Vector3(Random(-0.1f, 0.1f), 0.0f, Random(-0.1f, 0.1f)));
			newAgent->SetScale(0.45f);
			StaticModel* mNewAgent = newAgent->CreateComponent<StaticModel>();
			mNewAgent->SetModel(rc->GetResource<Model>("Models/Jack.mdl"));
			mNewAgent->SetMaterial(rc->GetResource<Material>("Materials/Jack.xml"));
			mNewAgent->SetCastShadows(true);

			CrowdAgent* ca = newAgent->CreateComponent<CrowdAgent>();
			ca->SetHeight(2.0f);
			ca->SetRadius(0.2f);
			ca->SetMaxSpeed(agentMatchingSpeed);
			ca->SetMaxAccel(3.0f);
		}
	}
	
}