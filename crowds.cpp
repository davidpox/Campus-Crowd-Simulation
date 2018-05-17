#include "crowds.h"

crowds::crowds() {}

crowds::~crowds() {}

enum weights {
	UNDERWEIGHT,
	NORMAL,
	OVERWEIGHT,
	OBESE
};

void crowds::Initialise(Scene* sc, ResourceCache* rc) {

	const int numAgents = 1;
	for (int j = 0; j < 200; j++) {
		Node* AgentGroup = sc->CreateChild("AgentGroup");
		AgentGroup->AddTag("AgentGroup");
		Vector3 agentMatchingPosition = Vector3(Random(-5.0f, -25.0f), 0.0f, Random(25.0f, 45.0f));

		for (int i = 0; i < numAgents; i++) {
			weights weight = weights(Random(0, 4));

			Node* newAgent = AgentGroup->CreateChild("Agent");
			newAgent->SetPosition(agentMatchingPosition + Vector3(Random(-0.1f, 0.1f), 0.0f, Random(-0.1f, 0.1f)));
			newAgent->SetScale(0.45f);
			StaticModel* mNewAgent = newAgent->CreateComponent<StaticModel>();
			mNewAgent->SetModel(rc->GetResource<Model>("Models/Jack.mdl"));
			mNewAgent->SetMaterial(rc->GetResource<Material>("Materials/Jack.xml"));
			mNewAgent->SetCastShadows(true);

			
			float speed;
			switch (weight) {
				case weights::UNDERWEIGHT:
				case weights::OVERWEIGHT:
					speed = Random(0.7f, 0.9f) + Random(-0.1f, 0.0f);
					break;				   
				case weights::OBESE:	   
					speed = Random(0.7f, 0.9f) + Random(-0.2f, -0.1f);
					break;				   
				case weights::NORMAL:	   
				default:				   
					speed = Random(0.7f, 0.9f);
					break;
			}

			//std::cout << "Chosen speed: " << speed << std::endl;

			CrowdAgent* ca = newAgent->CreateComponent<CrowdAgent>();
			ca->SetHeight(2.0f);
			ca->SetRadius(0.05f);
			ca->SetMaxSpeed(speed);
			ca->SetMaxAccel(3.0f);
		}
	}
	
}

Vector3 crowds::getSpawnPosition() {
	return Vector3(Random(-64, 64), 20, Random(-63, 70));


}
