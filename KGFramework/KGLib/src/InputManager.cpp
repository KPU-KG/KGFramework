#include "InputManager.h"
#include "ImguiHelper.h"

/// @brief 해당 프레임의 사용자 입력을 읽고 저장합니다.
/// @param hWnd 입력을 받아올 윈도우의 핸들입니다.

void KG::Input::InputManager::GetVKState(int keyId)
{
	if ( GetAsyncKeyState(keyId) & 0x8000 )
	{
		keyStates[keyId] = KeyState::Down;
	}
	else
	{
		keyStates[keyId] = KeyState::Up;
	}
}

void KG::Input::InputManager::SetUIContext(void* context)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(context));
}

void KG::Input::InputManager::SetUsingImgui(bool isUsingImgui)
{
    this->isUsingImgui = isUsingImgui;
}

LRESULT KG::Input::InputManager::HandlingInputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int key = static_cast<int>(wParam);
	if ( key > 256 )
	{
		return true;
	}
	switch ( message )
	{
		case WM_LBUTTONDOWN:
			keyStates[VK_LBUTTON] = KeyState::Down;
			break;
		case WM_LBUTTONUP:
			keyStates[VK_LBUTTON] = KeyState::Up;
			break;
		case WM_RBUTTONDOWN:
			keyStates[VK_RBUTTON] = KeyState::Down;
			break;
		case WM_RBUTTONUP:
			keyStates[VK_RBUTTON] = KeyState::Up;
			break;
		case WM_KEYDOWN:
			keyStates[key] = KeyState::Down;
			break;
		case WM_KEYUP:
			keyStates[key] = KeyState::Up;
			break;
	}
	return true;
}

void KG::Input::InputManager::ProcessInput(HWND hWnd)
{

	if ( hWnd != GetFocus() || (this->isUsingImgui && ImGui::IsAnyItemActive() && !this->startMouseCapture) )
	{
		for ( auto& i : this->keyStates )
		{
			i = KeyState::None;
		}
		return;
	}

	//특수키
    this->GetVKState(VK_LSHIFT);
    this->GetVKState(VK_RSHIFT);
    this->GetVKState(VK_SPACE);
    //this->GetVKState(VK_LBUTTON);



	//Mouse
	POINT mouseBuffer = {};
	GetCursorPos(&mouseBuffer);
	RECT rect = {};
	GetClientRect(hWnd, &rect);
	ScreenToClient(hWnd, &mouseBuffer);
	if ( !startProcessing )
	{
		mousePosition.x = mouseBuffer.x;
		mousePosition.y = mouseBuffer.y;
		startProcessing = true;
	}

	//마우스 변화 값
	this->deltaPosition.x = mouseBuffer.x - mousePosition.x;
	this->deltaPosition.y = mouseBuffer.y - mousePosition.y;


	if ( this->startMouseCapture )
	{
		POINT newMouseBuffer;
		newMouseBuffer.x = (rect.right - rect.left) / 2;
		newMouseBuffer.y = (rect.bottom - rect.top) / 2;
		mousePosition.x = newMouseBuffer.x;
		mousePosition.y = newMouseBuffer.y;
		//DebugNormalMessage("MouseCsr : " << newMouseBuffer.x << ", " << newMouseBuffer.y << " / Delta : " << this->deltaPosition.x << ", " << this->deltaPosition.y);
		ClientToScreen(hWnd, &newMouseBuffer);
		SetCursorPos(newMouseBuffer.x, newMouseBuffer.y);
	}
	else
	{
		mousePosition.x = mouseBuffer.x;
		mousePosition.y = mouseBuffer.y;
	}

}

void KG::Input::InputManager::PostProcessInput()
{
	if ( GetKeyboardState(keyBuffer) )
	{
		for ( size_t i = 0; i < 256; i++ )
		{
			switch ( keyStates[i] )
			{
				//클릭 안함
				case KeyState::Down:
					keyStates[i] = KeyState::Pressing;
					break;
				case KeyState::Up:
					keyStates[i] = KeyState::None;
					break;
				case KeyState::None:
					break;
			}
		}
	}

}

void KG::Input::InputManager::SetMouseCapture(bool startMouseCapture)
{
	this->startMouseCapture = startMouseCapture;
    if (this->startMouseCapture)
    {
        while (ShowCursor(!this->startMouseCapture) > -1);
    }
    else 
    {
        while (ShowCursor(!this->startMouseCapture) < 0);
    }
}

bool KG::Input::InputManager::GetMouseCapture() const
{
	return this->startMouseCapture;
}
