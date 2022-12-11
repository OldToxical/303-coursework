// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand */
#include <time.h>
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include <string>
#include <vector>
#include <list>
#include <fstream>
#include "SFML/Network.hpp"

// The UDP port number for the server
#define SERVERPORT 4444

struct Player
{
	XMFLOAT3 position;
	XMFLOAT3 scaling;
	SphereMesh* mesh;
	sf::TcpSocket* socket;
	bool moving;
	float timeout;
};

class App1 : public BaseApplication
{
public:
	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void gui();

private:
	void SetupServerListening();
	void ListenForNewConnections();
	void CommunicateWithClients();

	Input* input;
	std::ifstream inStream;
	std::string SERVERIP;

	LightShader* shader;
	PlaneMesh* mesh;
	Light* light;
	Light* light2;
	Light* light3;
	XMFLOAT4 lightDiffuse;
	XMFLOAT3 lightPos;

	sf::TcpListener listener;
	sf::UdpSocket udp;
	sf::Packet pack;
	int debug;
	float time_;
	bool update;
	std::vector<Player*> players;
	std::list<sf::TcpSocket*> clients;
};

#endif