#pragma once
#include "DXF.h"
#include "SFML/Network.hpp"

class Player
{
public:
	Player(sf::TcpSocket sock);
	Player(sf::TcpSocket sock, XMFLOAT3 pos, XMFLOAT3 scale);
	~Player();

private:
	XMFLOAT3 position;
	XMFLOAT3 scaling;
	SphereMesh* mesh;
	sf::TcpSocket socket;
};