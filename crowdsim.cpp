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
	CreateNavScene();

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
	//scene_->CreateComponent<DebugRenderer>();

	// CAMERA CREATION
	cameraNode_ = new Node(context_);
	Camera* camera = cameraNode_->CreateComponent<Camera>();
	cameraNode_->SetPosition(Vector3(0.0f, 20.0f, 0.0f));
	camera->SetFarClip(300.0f);
	GetSubsystem<Renderer>()->SetViewport(0, new Viewport(context_, scene_, camera));
}

void crowdsim::CreateAgents() {
	ResourceCache* cache = GetSubsystem<ResourceCache>();
	cr.Initialise(scene_, cache);
}

void crowdsim::CreateNavScene() {
	DynamicNavigationMesh* _navMesh = scene_->CreateComponent<DynamicNavigationMesh>();
	_navMesh->SetTileSize(32);
	_navMesh->SetDrawObstacles(true);
	_navMesh->SetAgentHeight(2.0f);
	_navMesh->SetCellHeight(0.05f);
	
	scene_->CreateComponent<Navigable>();
	_navMesh->SetPadding(Vector3(0.0f, 10.0f, 0.0f)); // defines how high a navigation may be. Tweak this for the stairs added later. 
	_navMesh->Build();

	CrowdManager* crowdManager = scene_->CreateComponent<CrowdManager>();
	CrowdObstacleAvoidanceParams params = crowdManager->GetObstacleAvoidanceParams(0);
	params.velBias = 0.5f;
	params.adaptiveDivs = 7;
	params.adaptiveRings = 3;
	params.adaptiveDepth = 3;
	crowdManager->SetObstacleAvoidanceParams(0, params);
}

void crowdsim::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(crowdsim, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(crowdsim, HandlePostUpdate));
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
	else if (input->GetKeyPress(KEY_KP_8)) {
		LoadScene();
	}
	else if (input->GetKeyPress(KEY_P)) {
		drawDebug_ = !drawDebug_;
	}

	if (input->GetMouseButtonPress(MOUSEB_LEFT)) {
		SetPathPoint();
	}
}

bool crowdsim::RayCast(float maxDistance, Vector3& hitPos, Drawable*& hitDrawable) {		// temp raycast help function from 39_CROWDNAVIGATION
	hitDrawable = 0;
	UI* ui = GetSubsystem<UI>();
	IntVector2 pos = ui->GetCursorPosition();
	
	//if (!ui->GetCursor()->IsVisible() || ui->GetElementAt(pos, true)) return false;

	Graphics* graphics = GetSubsystem<Graphics>();
	Camera* camera = cameraNode_->GetComponent<Camera>();
	Ray cameraRay = camera->GetScreenRay((float)pos.x_ / graphics->GetWidth(), (float)pos.y_ / graphics->GetHeight());
	PODVector<RayQueryResult> results;
	RayOctreeQuery query(results, cameraRay, RAY_TRIANGLE, maxDistance, DRAWABLE_GEOMETRY);
	scene_->GetComponent<Octree>()->RaycastSingle(query);
	if (results.Size()) {
		RayQueryResult& result = results[0];
		hitPos = result.position_;
		hitDrawable = result.drawable_;
		return true;
	}

	return false;
}

void crowdsim::SetPathPoint() {
	Vector3 hitPos;
	Drawable* hitDrawable; 

	if (RayCast(1000.0f, hitPos, hitDrawable)) {

		std::cout << "Setting Path Point to:\t x:" << hitPos.x_ << "\ty: " << hitPos.y_ << "\tz: " << hitPos.z_ << std::endl;

		DynamicNavigationMesh* navMesh = scene_->GetComponent<DynamicNavigationMesh>();
		Vector3 pathPos = navMesh->FindNearestPoint(hitPos, Vector3(1.0f, 1.0f, 1.0f));
		Node* AgentGroup = scene_->GetChild("AgentGroup");
		scene_->GetComponent<CrowdManager>()->SetCrowdTarget(pathPos, AgentGroup);
	}
}

void crowdsim::HandlePostUpdate(StringHash eventType, VariantMap& eventData) {
	if (drawDebug_) {
		//DebugRenderer* dRenderer = scene_->GetComponent<DebugRenderer>();
		scene_->GetComponent<DynamicNavigationMesh>()->DrawDebugGeometry(true);
		scene_->GetComponent<PhysicsWorld>()->DrawDebugGeometry(true);
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