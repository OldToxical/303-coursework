// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	srand(time(NULL));

	mesh = nullptr;
	shader = nullptr;
	light = nullptr;
	light2 = nullptr;
	light3 = nullptr;
	input = nullptr;
	debug = 0;
	time_ = 0;
	update = false;
	lightDiffuse = XMFLOAT4(0, 0, 0, 0);
	lightPos = XMFLOAT3(0, 0, 0);
	pack.clear();

	SetupServerListening();
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	input = in;

	// Create Mesh object and shader object
	mesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"wood", L"res/wood.png");
	shader = new LightShader(renderer->getDevice(), hwnd);
	
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
	debug = clients.size();

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}

	// Update 
	ListenForNewConnections();
	CommunicateWithClients();

	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	return true;
}

bool App1::render()
{
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix;

	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

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
	ImGui::Text("Connected players: %i", debug);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::SetupServerListening()
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
		inStream >> SERVERIP;
		inStream.close();
		if (SERVERIP == "")
		{
			MessageBoxA(0, "The entered IP address inside the configuration file is invalid. Please make sure it is your machine's IPv4 address.", "Error reading IP address", MB_OK);
			exit(0);
		}
	}
	catch (int e)
	{
		MessageBoxA(0, "Invalid configuration file. Please check if the file exists", "Error reading Config.dat", MB_OK);
		exit(0);
	}

	// bind the listener to a port
	if (!listener.listen(SERVERPORT, SERVERIP) == sf::Socket::Done)
	{
		MessageBoxA(0, "The listener socket encountered a problem during binding it to a port. Please check that the IP address inside the .dat config file is your machine's IPv4 address", "Binding Error", MB_OK);
		exit(0);
	}
	listener.setBlocking(false);
}

void App1::ListenForNewConnections()
{
	// Create e new tcp socket for each new client
	sf::TcpSocket* client = new sf::TcpSocket;
	client->setBlocking(false);

	// Accept a new connection and add the client's instance and his socket to the corresponding lists of the connected clients
	if (listener.accept(*client) == sf::Socket::Done)
	{
		clients.push_back(client);
		Player* player = new Player;
		player->mesh = new SphereMesh(renderer->getDevice(), renderer->getDeviceContext());
		player->position = XMFLOAT3(50.0f, 2.0f, 50.0f);
		player->scaling = XMFLOAT3(2, 2, 2);
		player->socket = client;
		player->moving = false;
		player->timeout = 0.5f;
		players.push_back(player);
		update = true;
	}
	else if (listener.accept(*client) == sf::Socket::Partial || listener.accept(*client) == sf::Socket::Error)
	{
		MessageBoxA(0, "The listener socket couldn't accept the new connection. Make sure no defenders are blocking it.", "Socket error", MB_OK);
		exit(0);
	}
	client = NULL;
}

void App1::CommunicateWithClients()
{
	for (std::list<sf::TcpSocket*>::iterator i = clients.begin(); i != clients.end();)
	{
		// Get the current socket and its player from the current iteration
		sf::TcpSocket& clientSock = **i;
		Player* currentPlayer = new Player;
		float currentPlayerIndex = 0;

		for (int j = 0; j < players.size(); ++j)
		{
			if (&clientSock == players[j]->socket)
			{
				currentPlayer = players[j];
				currentPlayerIndex = j;
				break;
			}
		}

		// Initialize a variable that will hold information about all the updates that the server will have to report to the clients
		XMFLOAT3 update_ = XMFLOAT3(-1.f, -1.f, -1.f);

		// Tell the new player how many clients are connected if there was a change in the connections
		if (update)
		{
			update_ = XMFLOAT3(1.f, (float)clients.size(), currentPlayerIndex);
			pack << update_.x << update_.y << update_.z;
			clientSock.send(pack);
			pack.clear();
		}

		// Decrease the "receive timeout" value for the current client
		currentPlayer->timeout -= timer->getTime();

		// The player has sent something
		if (clientSock.receive(pack) == sf::Socket::Done)
		{
			XMFLOAT3 newPos;
			if (pack >> newPos.x >> newPos.y >> newPos.z)
			{
				// The client wants to quit
				if (newPos.x >= -235.236f && newPos.x <= -235.234f)
				{
					// Remove the player from the list with the connected clients
					for (int i = 0; i < players.size(); ++i)
					{
						if (players[i] == currentPlayer)
						{
							players[i]->socket = NULL;
							players[i]->mesh = NULL;
							players.erase(players.begin() + i);
							pack.clear();

							// Tell all the connected players that he quits
							update_ = XMFLOAT3(0.f, currentPlayerIndex, -1.f);
							pack << update_.x << update_.y << update_.z;
							for (int k = 0; k < players.size(); ++k)
							{
								players[k]->socket->send(pack);
							}
							pack.clear();
							break;
						}
					}
					std::list<sf::TcpSocket*>::iterator pos = clients.erase(i);
					i = pos;
					clientSock.disconnect();
				}
				// Client says he's here, but he hasn't changed his position
				else if (newPos.x >= -200.236f && newPos.x <= -200.234f)
				{
					// Reset his timeout value, he reported he is here
					currentPlayer->timeout = 0.5f;
					++i;
				}
				// Client wants do update his position
				else
				{
					pack.clear();
					currentPlayer->timeout = 0.5f;
					currentPlayer->position = newPos;
					currentPlayer->moving = true;
					// Send this new position and the player's index to all the connected players
					for (int k = 0; k < players.size(); ++k)
					{
						XMFLOAT4 packet;
						packet.x = newPos.x;
						packet.y = newPos.y;
						packet.z = newPos.z;
						packet.w = currentPlayerIndex;
						pack << packet.x << packet.y << packet.z << packet.w;
						udp.send(pack, players[k]->socket->getRemoteAddress(), 54);
					}
					++i;
				}
				pack.clear();
			}
		}
		// The player hasn't sent anything
		else
		{
			// Check if the last thing he reported was if he was moving or standing still. If the condition is true, it means the player has lifted his finger from the keyboard, so his sudden stop of movement is intentional, his timeout value is still positive
			if (currentPlayer->moving && currentPlayer->timeout > 0.f)
			{
				// Tell all the players that he had been moving but he stopped
				for (int k = 0; k < players.size(); ++k)
				{
					XMFLOAT4 packet;
					packet.x = 0;
					packet.y = 0;
					packet.z = -100.235;
					packet.w = currentPlayerIndex;
					pack << packet.x << packet.y << packet.z << packet.w;
					udp.send(pack, players[k]->socket->getRemoteAddress(), 54);
					pack.clear();
					currentPlayer->moving = false;
				}
			}
			// The player doesn't seem to respond - tell all the players to consider him disconnected and wait for him for 10 seconds, if there is still no reply - disconnect him
			else if (currentPlayer->timeout < -0.1f) 
			{
				if (currentPlayer->timeout < -10.f)
				{
					// Disconnect the player and remove his socket from the list
					for (int i = 0; i < players.size(); ++i)
					{
						if (players[i] == currentPlayer)
						{
							players[i]->socket = NULL;
							players[i]->mesh = NULL;
							players.erase(players.begin() + i);
							pack.clear();

							// Tell all the players that he is disconnected
							update_ = XMFLOAT3(0.f, currentPlayerIndex, -1.f);
							pack << update_.x << update_.y << update_.z;
							for (int k = 0; k < players.size(); ++k)
							{
								players[k]->socket->send(pack);
							}
							pack.clear();
							break;
						}
					}
					std::list<sf::TcpSocket*>::iterator pos = clients.erase(i);
					i = pos;
					clientSock.disconnect();
					return;
				}
				// Tell all the players that the player is not talking
				for (int k = 0; k < players.size(); ++k)
				{
					XMFLOAT4 packet;
					packet.x = 0;
					packet.y = 0;
					packet.z = -300.235;
					packet.w = currentPlayerIndex;
					pack << packet.x << packet.y << packet.z << packet.w;
					udp.send(pack, players[k]->socket->getRemoteAddress(), 54);
					pack.clear();
					currentPlayer->moving = false;
				}
			}
			++i;
		}

		currentPlayer = NULL;
	}
}