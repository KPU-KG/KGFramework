#include "Server.h"

void Server::Initialize() {
	serverComponent = new ServerComponent;
	/*this->scene.SetComponentProvider(&this->componentProvider);
	this->scene.InitializeRoot();
	this->scene.LoadScene("SceneData.xml");*/
	serverComponent->Loop();
}