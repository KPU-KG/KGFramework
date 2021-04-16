#pragma once
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include "ComponentProvider.h"
#include "Scene.h"
#include "ServerComponent.h"

using namespace std;
struct Systems;

class Server {
	//std::unique_ptr<Systems> system;
	KG::Component::ComponentProvider componentProvider;
	KG::Core::Scene scene;
	ServerComponent* serverComponent;

public:
	void Initialize();
};
