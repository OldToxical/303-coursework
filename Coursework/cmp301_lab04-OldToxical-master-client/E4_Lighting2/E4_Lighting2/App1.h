// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "DXF.h"	// include dxframework
#include "LightShader.h"
#include <string>
#include <vector>
#include <fstream>
#include "SFML/Network.hpp"

// The UDP port number for the server
#define SERVERPORT 4444

struct Player
{
	XMFLOAT3 position;
	XMFLOAT3 interpolatedPosition;
	XMFLOAT3 scaling;
	SphereMesh* mesh;
	int index;
	std::vector<XMFLOAT3> lastKnownPositions_;
	std::vector<float> lastKnownPositionsTimes;
	bool moving;
	bool predicting;
	float timeout_;
	float idleTimeout;
	float predictionTimeout;
	float interpolationSpeed;
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
	std::ifstream inStream;
	std::string LOCAL_IP;
	std::string SERVER_IP;

	XMFLOAT3 LinearPrediction(Player* player);
	XMFLOAT3 QuadraticPrediction(Player* player);
	void InterpolatePosition(Player* player, const XMFLOAT3 &vec2, float speed);
	void InterpolateCamera(const XMFLOAT3 &newPos);
	void SetupConnection();
	void ServerIntroduction();
	void ReportToServer(bool isActive);
	void ServerUpdate();
	void ProcessServerUpdates();
	Input* input;

	LightShader* shader;
	PlaneMesh* mesh;
	Light* light;
	Light* light2;
	Light* light3;
	XMFLOAT4 lightDiffuse;
	XMFLOAT3 lightPos;

	sf::TcpSocket socket;
	sf::Socket::Status status;
	sf::UdpSocket socket_;
	sf::IpAddress sender;
	sf::Packet pack;

	float time_;
	unsigned short port;
	int debug;
	bool once;
	int iteration;
	std::vector<Player*> players;
	Player* currentPlayer;
	int playerCount;
	float timeout;
	XMFLOAT3 interpolatedCamPos;
};
#endif