#include "crowdsim.h"

URHO3D_DEFINE_APPLICATION_MAIN(crowdsim)

crowdsim::crowdsim(Context* context) : Sample(context), firstPerson_(false) {}

crowdsim::~crowdsim() {}

void crowdsim::Start()
{
    Sample::Start();
	OpenConsoleWindow();
	CreateScene();
	LoadScene();

	CreateAgents();
	SubscribeToEvents();
	Sample::InitMouseMode(MM_RELATIVE);
}

void crowdsim::CreateScene()
{
	// SCENE CREATION
	ResourceCache* cache = GetSubsystem<ResourceCache>();

	scene_ = new Scene(context_);
	scene_->CreateComponent<Octree>();
	scene_->CreateComponent<PhysicsWorld>();

	// CAMERA CREATION
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 5.0f, 0.0f));
	camera->SetFarClip(300.0f);

	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
}

void crowdsim::CreateAgents() {
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	cr.Initialise(scene_, cache);
}

void crowdsim::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(crowdsim, HandleUpdate));
}

void crowdsim::HandleUpdate(StringHash eventType, VariantMap& eventData)
{
	using namespace Update;

	float timeStep = eventData[P_TIMESTEP].GetFloat();

	if (GetSubsystem<UI>()->GetFocusElement()) return;

	Input* input = GetSubsystem<Input>();

	const float MOVE_SPEED = 40.0f;
	const float MOUSE_SENSITIVITY = 0.1f;

	IntVector2 mouseMove = input->GetMouseMove();
	yaw_ += MOUSE_SENSITIVITY * mouseMove.x_;
	pitch_ += MOUSE_SENSITIVITY * mouseMove.y_;
	pitch_ = Clamp(pitch_, -90.0f, 90.0f);

	cameraNode_->SetRotation(Quaternion(pitch_, yaw_, 0.0f));

	if (input->GetKeyDown(KEY_W)) cameraNode_->Translate(Vector3::FORWARD * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_S)) cameraNode_->Translate(Vector3::BACK * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_A)) cameraNode_->Translate(Vector3::LEFT * MOVE_SPEED * timeStep);
	if (input->GetKeyDown(KEY_D)) cameraNode_->Translate(Vector3::RIGHT * MOVE_SPEED * timeStep);
	if (input->GetKeyPress(KEY_KP_7)) {
		SaveScene();
	}
	if (input->GetKeyPress(KEY_KP_8)) {
		LoadScene();
	}
}

void crowdsim::SaveScene() {
	File saveFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/map.xml", FILE_WRITE);
	scene_->SaveXML(saveFile);
}

void crowdsim::LoadScene() {
	File loadFile(context_, GetSubsystem<FileSystem>()->GetProgramDir() + "Data/Scenes/map.xml", FILE_READ);
	scene_->LoadXML(loadFile);
}