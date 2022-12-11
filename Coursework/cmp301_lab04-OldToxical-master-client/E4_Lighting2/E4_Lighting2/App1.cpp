// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	mesh = nullptr;
	shader = nullptr;
	light = nullptr;
	light2 = nullptr;
	light3 = nullptr;
	input = nullptr;
	debug = 0;
	playerCount = 0;
	iteration = 0;
	once = false;
	lightDiffuse = XMFLOAT4(0, 0, 0, 0);
	lightPos = XMFLOAT3(0, 0, 0);
	pack.clear();
	timeout = 0.5f;
	time_ = 0.f;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	input = in;

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	shader = new LightShader(renderer->getDevice(), hwnd);

	currentPlayer = new Player;
	currentPlayer->mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
	currentPlayer->position = XMFLOAT3(50, 2, 50);
	currentPlayer->scaling = XMFLOAT3(2, 2, 2);
	currentPlayer->index = -100;
	currentPlayer->idleTimeout = -1.f;
	currentPlayer->interpolatedPosition = currentPlayer->position;
	currentPlayer->interpolationSpeed = 1.5f;
	currentPlayer->predictionTimeout = 1.5f;
	currentPlayer->predicting = false;

	camera->setPosition(50, 3, 40);
	interpolatedCamPos = camera->getPosition();

	// Configure point light.
	light = new Light();
	light->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	light->setDiffuseColour(0.0f, 0.0f, 1.0f, 1.0f);
	light->setPosition(70.0f, 10.0f, 50.0f);
	light->setSpecularPower(100.0f);
	light->setSpecularColour(0.4f, 0.4f, 0.4f, 1.0f);
	light->setDirection(3, -60, -100);

	light2 = new Light();
	light2->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	light2->setDiffuseColour(1.0f, 0.0f, 0.0f, 1.0f);
	light2->setPosition(30.0f, 10.0f, 50.0f);
	light2->setSpecularPower(2.0f);
	light2->setSpecularColour(0.4f, 0.4f, 0.4f, 1.0f);
	light2->setDirection(3, -60, -100);

	light3 = new Light();
	light3->setAmbientColour(0.0f, 0.0f, 0.0f, 1.0f);
	light3->setDiffuseColour(0.3f, 0.3f, 0.3f, 1.0f);
	light3->setPosition(60.0f, 10.0f, 50.0f);
	light3->setSpecularPower(20.0f);
	light3->setSpecularColour(0.4f, 0.4f, 0.4f, 1.0f);
	light3->setDirection(1, 10, 0);

	lightDiffuse = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	lightPos = XMFLOAT3(40.0f, 10.0f, 50.0f);

	SetupConnection();
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.
	if (mesh)
	{
		delete mesh;
		mesh = 0;
	}

	if (shader)
	{
		delete shader;
		shader = 0;
	}
}


bool App1::frame()
{
	bool result;

	if (iteration < 3)
	{
		iteration++;
	}

	ServerIntroduction();
	result = BaseApplication::frame();
	ReportToServer(result);

	if (!result)
	{
		return false;
	}

	ServerUpdate();
	ProcessServerUpdates();

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	debug = players.size();

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();
	InterpolateCamera(interpolatedCamPos);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	worldMatrix = renderer->getWorldMatrix();
	viewMatrix = camera->getViewMatrix();
	projectionMatrix = renderer->getProjectionMatrix();

	light2->setDiffuseColour(lightDiffuse.x, lightDiffuse.y, lightDiffuse.z, lightDiffuse.w);
	light2->setPosition(lightPos.x, lightPos.y, lightPos.z);

	// Send geometry data, set shader parameters, render object with shader
	mesh->sendData(renderer->getDeviceContext());
	shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"), light, light2, light3, camera->getPosition(), 10);
	shader->render(renderer->getDeviceContext(), mesh->getIndexCount());

	worldMatrix = XMMatrixScaling(currentPlayer->scaling.x, currentPlayer->scaling.y, currentPlayer->scaling.z) * XMMatrixTranslation(currentPlayer->position.x, currentPlayer->position.y, currentPlayer->position.z);

	// Send players' geometry data
	for (int i = 0; i < players.size(); i++)
	{
		XMMATRIX worldMatrix_;
		worldMatrix_ = renderer->getWorldMatrix();
		worldMatrix_ = XMMatrixScaling(players[i]->scaling.x, players[i]->scaling.y, players[i]->scaling.z) * XMMatrixTranslation(players[i]->position.x, players[i]->position.y, players[i]->position.z);

		players[i]->mesh->sendData(renderer->getDeviceContext());
		shader->setShaderParameters(renderer->getDeviceContext(), worldMatrix_, viewMatrix, projectionMatrix, textureMgr->getTexture(L"wood"), light, light2, light3, camera->getPosition(), 1);
		shader->render(renderer->getDeviceContext(), players[i]->mesh->getIndexCount());
	}

	// Render GUI
	gui();

	// Swap the buffers
	renderer->endScene();

	return true;
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Text("Playing with %i players", debug - 1);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

XMFLOAT3 App1::LinearPrediction(Player* player)
{
	iteration++;
	float predictedX = 0;
	float predictedY = 0;
	float predictedZ = 0;

	if (player->lastKnownPositions_.size() < 3 && player->lastKnownPositionsTimes.size() < 3)
	{
		return XMFLOAT3(player->position.x, player->position.y, player->position.z);
	}

	float distanceX = player->lastKnownPositions_[1].x - player->lastKnownPositions_[2].x;//msg1.x - msg0.x; 
	float time = player->lastKnownPositionsTimes[1] - player->lastKnownPositionsTimes[2];
	float speedX = distanceX / time;
	float displacementX = speedX * (time_ - player->lastKnownPositionsTimes[2]);
	predictedX = player->lastKnownPositions_[2].x + displacementX;

	float distanceY = player->lastKnownPositions_[1].y - player->lastKnownPositions_[2].y;
	float speedY = distanceY / time;
	float displacementY = speedY * (time_ - player->lastKnownPositionsTimes[2]);
	predictedY = player->lastKnownPositions_[2].y + displacementY;

	float distanceZ = player->lastKnownPositions_[1].z - player->lastKnownPositions_[2].z;
	float speedZ = distanceZ / time;
	float displacementZ = speedZ * (time_ - player->lastKnownPositionsTimes[2]);
	predictedZ = player->lastKnownPositions_[2].z + displacementZ;

	return XMFLOAT3(predictedX, predictedY, predictedZ);
}

XMFLOAT3 App1::QuadraticPrediction(Player* player)
{
	return XMFLOAT3(0, 0, 0);
}

void App1::InterpolatePosition(Player* player, const XMFLOAT3& vec2, float speed)
{
	if (vec2.x != 0)
	{
		float X = player->position.x + timer->getTime() * speed / 0.125f * (vec2.x - player->position.x);
		float Y = player->position.y + timer->getTime() * speed / 0.125f * (vec2.y - player->position.y);
		float Z = player->position.z + timer->getTime() * speed / 0.125f * (vec2.z - player->position.z);
		player->position = XMFLOAT3(X, Y, Z);
	}
}

void App1::InterpolateCamera(const XMFLOAT3& newPos)
{
	float X = camera->getPosition().x + timer->getTime() * 1.1f / 0.125f * (newPos.x - camera->getPosition().x);
	float Y = camera->getPosition().y + timer->getTime() * 1.1f / 0.125f * (newPos.y - camera->getPosition().y);
	float Z = camera->getPosition().z + timer->getTime() * 1.1f / 0.125f * (newPos.z - camera->getPosition().z);
	camera->setPosition(X, Y, Z);
}

void App1::SetupConnection()
{
	// Read config to obtain local ipv4 address and the sever address
	try
	{
		inStream.open("Config.dat");

		// Check for error
		if (inStream.fail())
		{
			throw 0;
		}

		// Read the IP
		inStream >> SERVER_IP;
		inStream >> LOCAL_IP;
		inStream.close();
		if (SERVER_IP == "" || LOCAL_IP == "")
		{
			MessageBoxA(0, "One or borth of the entered IP addresss inside the configuration file is/are invalid. Please make sure the top one is the server's IP address and the bottom one - your machine's IPv4 address", "Error reading IP addresses", MB_OK);
			exit(0);
		}
	}
	catch (int e)
	{
		MessageBoxA(0, "Invalid configuration file. Please check if the file exists", "Error reading Config.dat", MB_OK);
		exit(0);
	}

	// Bind UDP socket -----------
	socket_.setBlocking(false);
	if (!socket_.bind(54, LOCAL_IP) == sf::Socket::Done)
	{
		MessageBoxA(0, "The UDP listener socket encountered a problem during binding it to port '54'. Please make sure no defenders are blocking it and the entered IP address in the 'Config.dat' file is your machine's IPv4 address.", "Binding Error", MB_OK);
		exit(0);
	}
	// ---------------------------

	// Connect to the server using TCP ------------
	if (!socket.connect(SERVER_IP, SERVERPORT) == sf::Socket::Done)
	{
		MessageBoxA(0, "Make sure the server is online. If it is, then make sure the the entered IP addresses in the 'Config.dat' file are correspondingly the server's IP address and your machine's IPv4 address.", "Connection to server unsuccessfull", MB_OK);
		exit(0);
	}
	socket.setBlocking(false);
	// --------------------------------------------
}

void App1::ServerIntroduction()
{
	// Receive updates for the connected players
	if (socket.receive(pack) == sf::Socket::Done)
	{
		XMFLOAT3 update = XMFLOAT3(-1.f, -1.f, -1.f);
		if (pack >> update.x >> update.y >> update.z)
		{
			playerCount = update.y;
			currentPlayer->index = update.z;
			// Somebody has quit
			if (update.z >= -1.1f && update.z <= -0.9f)
			{
				players.erase(players.begin() + ((int)update.y));
			}
		}
		pack.clear();
	}

	// Create an instance for each new client
	if (players.size() != playerCount)
	{
		if (players.size() == 0)
		{
			for (int i = players.size(); i < playerCount - 1; i++)
			{
				Player* newPlayer = new Player;
				newPlayer->mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
				newPlayer->position = XMFLOAT3(50, 2, 50);
				newPlayer->scaling = XMFLOAT3(2, 2, 2);
				newPlayer->index = i;
				newPlayer->moving = false;
				newPlayer->idleTimeout = 10.f;
				newPlayer->interpolatedPosition = XMFLOAT3(0, 0, 0);
				newPlayer->predicting = false;
				newPlayer->predictionTimeout = 1.5f;
				players.push_back(newPlayer);
			}
		}
		else
		{
			for (int i = players.size(); i < playerCount; i++)
			{
				Player* newPlayer = new Player;
				newPlayer->mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
				newPlayer->position = XMFLOAT3(50, 2, 50);
				newPlayer->scaling = XMFLOAT3(2, 2, 2);
				newPlayer->index = i;
				newPlayer->moving = false;
				newPlayer->idleTimeout = 10.f;
				newPlayer->interpolatedPosition = XMFLOAT3(0, 0, 0);
				newPlayer->predicting = false;
				newPlayer->predictionTimeout = 1.5f;
				players.push_back(newPlayer);
			}
		}
	}

	// Push the current player's instance into the list
	if (!once && iteration == 2)
	{
		players.push_back(currentPlayer);
		once = true;
	}
}

void App1::ReportToServer(bool isActive)
{
	// Tell the server that "escape" key has been pressed, which means that you want to quit
	if (!isActive)
	{
		pack << -235.235f << 0.f << 0.f;
		socket.send(pack);
		pack.clear();
		return;
	}

	// Iterrate the game time
	time_ += timer->getTime();

	// Handle input
	if (input->isKeyDown('W'))
	{
		currentPlayer->moving = true;
		timeout = 0.5f;
		interpolatedCamPos = XMFLOAT3(interpolatedCamPos.x, interpolatedCamPos.y, interpolatedCamPos.z + timer->getTime() * 11);
		currentPlayer->interpolatedPosition = XMFLOAT3(currentPlayer->interpolatedPosition.x, currentPlayer->interpolatedPosition.y, currentPlayer->interpolatedPosition.z + timer->getTime() * 11);
	}
	if (input->isKeyDown('A'))
	{
		currentPlayer->moving = true;
		timeout = 0.5f;
		interpolatedCamPos = XMFLOAT3(interpolatedCamPos.x - timer->getTime() * 11, interpolatedCamPos.y, interpolatedCamPos.z);
		currentPlayer->interpolatedPosition = XMFLOAT3(currentPlayer->interpolatedPosition.x - timer->getTime() * 11, currentPlayer->interpolatedPosition.y, currentPlayer->interpolatedPosition.z);
	}
	if (input->isKeyDown('S'))
	{
		currentPlayer->moving = true;
		timeout = 0.5f;
		interpolatedCamPos = XMFLOAT3(interpolatedCamPos.x, interpolatedCamPos.y, interpolatedCamPos.z - timer->getTime() * 11);
		currentPlayer->interpolatedPosition = XMFLOAT3(currentPlayer->interpolatedPosition.x, currentPlayer->interpolatedPosition.y, currentPlayer->interpolatedPosition.z - timer->getTime() * 11);
	}
	if (input->isKeyDown('D'))
	{
		currentPlayer->moving = true;
		timeout = 0.5f;
		interpolatedCamPos = XMFLOAT3(interpolatedCamPos.x + timer->getTime() * 11, interpolatedCamPos.y, interpolatedCamPos.z);
		currentPlayer->interpolatedPosition = XMFLOAT3(currentPlayer->interpolatedPosition.x + timer->getTime() * 11, currentPlayer->interpolatedPosition.y, currentPlayer->interpolatedPosition.z);
	}

	// Report to the server that you have just moved
	if (currentPlayer->moving)
	{
		pack << currentPlayer->position.x << currentPlayer->position.y << currentPlayer->position.z;
		socket.send(pack);
		pack.clear();
	}

	// If you haven't moved in the last 0.5 seconds, report to the server you are still connected but you don't want to move
	timeout -= timer->getTime();
	if (timeout <= 0 && !currentPlayer->moving)
	{
		timeout = 0.5f;
		pack << -200.235f << 0.f << 0.f;
		socket.send(pack);
		pack.clear();
	}
	currentPlayer->moving = false;
}

void App1::ServerUpdate()
{
	// Try to receive 
	if (socket_.receive(pack, sender, port) == sf::Socket::Done)
	{
		// Packet received, a player has made a change to his state, let's see who it was
		XMFLOAT4 packet;
		if (pack >> packet.x >> packet.y >> packet.z >> packet.w)
		{
			// If it was you, don't update your position once more
			if (packet.w != currentPlayer->index)
			{
				// Iterate the players
				for (int i = 0; i < players.size(); ++i) 
				{
					// There is a change related to the player let's see what exactly it is
					if (players[i]->index == packet.w)
					{
						// The player stopped moving, he lifted his finger from his keyboard, that's why he's not reporting change in his position
						if (packet.z >= -100.236f && packet.z <= -100.234f)
						{
							// Let's write down that this player doesn't want to move and decrement his timeout timer
							players[i]->moving = false;
							players[i]->idleTimeout -= timer->getTime();
							if (players[i]->idleTimeout < 0.f)
							{
								// The player has been idle for too long, delete his most recent positions, because he didn't have any new ones recently
								players[i]->lastKnownPositions_.clear();
								players[i]->lastKnownPositionsTimes.clear();
							}
							break;
						}
						// There is no communication with this player
						else if (packet.z >= -300.236f && packet.z <= -300.234f)
						{
							// Let's write down that the player hasn't reported anything recently and let's start predicting what he might be doing right now
							players[i]->moving = false;
							players[i]->predicting = true;
							players[i]->interpolationSpeed = 0.3f;
							players[i]->interpolatedPosition = LinearPrediction(players[i]);
							// After predicting his possible location, add it to the list of the last known ones so that the prediction can work the next time based on them
 							if (players[i]->lastKnownPositions_.size() == 3)
							{
								players[i]->lastKnownPositions_[0] = players[i]->lastKnownPositions_[1];
								players[i]->lastKnownPositions_[1] = players[i]->lastKnownPositions_[2];
								players[i]->lastKnownPositions_.pop_back();
								players[i]->lastKnownPositions_.push_back(players[i]->interpolatedPosition);
							}
							else
							{
								players[i]->lastKnownPositions_.push_back(players[i]->interpolatedPosition);
							}
							// Add last known positions' timestamps
							if (players[i]->lastKnownPositionsTimes.size() == 3)
							{
								players[i]->lastKnownPositionsTimes[0] = players[i]->lastKnownPositionsTimes[1];
								players[i]->lastKnownPositionsTimes[1] = players[i]->lastKnownPositionsTimes[2];
								players[i]->lastKnownPositionsTimes.pop_back();
								players[i]->lastKnownPositionsTimes.push_back(time_);
							}
							else
							{
								players[i]->lastKnownPositionsTimes.push_back(time_);
							}
							break;
						}
						// The player is moving
						else
						{
							players[i]->moving = true;
							players[i]->idleTimeout = 10.f;
							// There was no connection with the player but it resumed, so let's mask the quick teleportation to his real position
							if (players[i]->predicting)
							{
								players[i]->predictionTimeout -= timer->getTime();
								if (players[i]->predictionTimeout > 0.f)
								{
									players[i]->interpolationSpeed = 0.3f;
								}
								else
								{
									players[i]->predicting = false;
								}
							}
							else
							{
								players[i]->predictionTimeout = 1.5f;
								players[i]->interpolationSpeed = 1.2f;
							}
							players[i]->interpolatedPosition.x = packet.x;
							players[i]->interpolatedPosition.y = packet.y;
							players[i]->interpolatedPosition.z = packet.z;
							// Add last known positions
							if (players[i]->lastKnownPositions_.size() == 3)
							{
								players[i]->lastKnownPositions_[0] = players[i]->lastKnownPositions_[1];
								players[i]->lastKnownPositions_[1] = players[i]->lastKnownPositions_[2];
								players[i]->lastKnownPositions_.pop_back();
								players[i]->lastKnownPositions_.push_back(players[i]->position);
							}
							else
							{
								players[i]->lastKnownPositions_.push_back(players[i]->position);
							}
							// Add last known positions' timestamps
							if (players[i]->lastKnownPositionsTimes.size() == 3)
							{
								players[i]->lastKnownPositionsTimes[0] = players[i]->lastKnownPositionsTimes[1];
								players[i]->lastKnownPositionsTimes[1] = players[i]->lastKnownPositionsTimes[2];
								players[i]->lastKnownPositionsTimes.pop_back();
								players[i]->lastKnownPositionsTimes.push_back(time_);
							}
							else
							{
								players[i]->lastKnownPositionsTimes.push_back(time_);
							}
							break;
						}
					}
				}
			}
		}
		pack.clear();
	}
}

void App1::ProcessServerUpdates()
{
	// Interpolate every single change in the other players' positions
	for (int i = 0; i < players.size(); i++)
	{
		InterpolatePosition(players[i], players[i]->interpolatedPosition, players[i]->interpolationSpeed);
	}

	// Interpolate your own position
	if (players.size() > 1)
	{
		InterpolatePosition(currentPlayer, currentPlayer->interpolatedPosition, currentPlayer->interpolationSpeed);
	}
}