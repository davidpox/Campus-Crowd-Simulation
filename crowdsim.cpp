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
	params.velBias = 0.5f;			// 0.5
	params.adaptiveDivs = 7;		// 7
	params.adaptiveRings = 3;		// 3
	params.adaptiveDepth = 3;		// 3
	crowdManager->SetObstacleAvoidanceParams(0, params);
}

void crowdsim::SubscribeToEvents()
{
	SubscribeToEvent(E_UPDATE, URHO3D_HANDLER(crowdsim, HandleUpdate));
	SubscribeToEvent(E_POSTUPDATE, URHO3D_HANDLER(crowdsim, HandlePostUpdate));
	SubscribeToEvent(E_CROWD_AGENT_FORMATION, URHO3D_HANDLER(crowdsim, HandleFormationSuccess));
	SubscribeToEvent(E_CROWD_AGENT_FAILURE, URHO3D_HANDLER(crowdsim, HandleFormationFailure));
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

	UpdateStreaming();
}

void crowdsim::UpdateStreaming() {
	Vector3 agentPositions;
	if (Node* agents = scene_->GetChild("AgentGroup")) {				// calculate the mean position within a formation of agents.
		int agentCount = agents->GetNumChildren();
		for (int i = 0; i < agentCount; i++) {
			agentPositions += agents->GetChild(i)->GetWorldPosition();
		}
		agentPositions /= agentCount;
	}
	
	DynamicNavigationMesh* mesh = scene_->GetComponent<DynamicNavigationMesh>();
	IntVector2 agent_t = mesh->GetTileIndex(agentPositions);
	IntVector2 tileCount = mesh->GetNumTiles();
	IntVector2 beginTile = VectorMax(IntVector2::ZERO, agent_t - IntVector2::ONE * 2);
	IntVector2 endTile = VectorMin(agent_t + IntVector2::ONE * 2, tileCount - IntVector2::ONE);

	for (auto i = activeTiles.Begin(); i != activeTiles.End();) {
		IntVector2 tile = *i;
		if (beginTile.x_ <= tile.x_ && tile.x_ <= endTile.x_ && beginTile.y_ <= tile.y_ && tile.y_ <= endTile.y_) {
			++i;
		}
		else {
			mesh->RemoveTile(tile);
			i = activeTiles.Erase(i);
		}
	}

	for (int y = beginTile.y_; y <= endTile.y_; ++y) {
		for (int x = beginTile.x_; x <= endTile.x_; ++x) {
			IntVector2 tile(x, y);
			if (!mesh->HasTile(tile) && tdata.Contains(tile)) {
				activeTiles.Insert(tile);
				mesh->AddTile(tdata[tile]);
			}
		}
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

void crowdsim::SaveNavigationData() {
	DynamicNavigationMesh* mesh = scene_->GetComponent<DynamicNavigationMesh>();
	tdata.Clear();
	activeTiles.Clear();
	IntVector2 tileCount = mesh->GetNumTiles();
	for (int y = 0; y < tileCount.y_; ++y) {
		for (int x = 0; x < tileCount.x_; ++x) {
			IntVector2 tile = IntVector2(x, y);
			tdata[tile] = mesh->GetTileData(tile);
		}
	}
}

void crowdsim::HandleFormationFailure(StringHash eventType, VariantMap& eventData) {		// if formation fails, get a position nearby that is free. 
	using namespace CrowdAgentFailure;

	Node* node = static_cast<Node*>(eventData[P_NODE].GetPtr());
	CrowdAgentState agentState = (CrowdAgentState)eventData[P_CROWD_AGENT_STATE].GetInt();
	if(agentState == CA_STATE_INVALID) {
		Vector3 pos = scene_->GetComponent<DynamicNavigationMesh>()->FindNearestPoint(node->GetPosition(), Vector3(5.0f, 5.0f, 5.0f));
		node->SetPosition(pos);
	}
}

void crowdsim::HandleFormationSuccess(StringHash eventType, VariantMap& eventData) {		// main agent will get formation position, all others will be nearby. 
	using namespace CrowdAgentFormation;

	int agentIndex = eventData[P_INDEX].GetUInt();
	int agentSize = eventData[P_SIZE].GetUInt();
	Vector3 pos = eventData[P_POSITION].GetVector3();

	if (agentIndex) {
		CrowdManager* cm = static_cast<CrowdManager*>(GetEventSender());
		CrowdAgent* agent = static_cast<CrowdAgent*>(eventData[P_CROWD_AGENT].GetPtr());
		eventData[P_POSITION] = cm->GetRandomPointInCircle(pos, agent->GetRadius(), agent->GetQueryFilterType());
	}
}