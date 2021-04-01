#include "InputManager.h"
#include "ImguiHelper.h"

/// @brief 해당 프레임의 사용자 입력을 읽고 저장합니다.
/// @param hWnd 입력을 받아올 윈도우의 핸들입니다.

void KG::Input::InputManager::SetUIContext(void* context)
{
	ImGui::SetCurrentContext(reinterpret_cast<ImGuiContext*>(context));
}

void KG::Input::InputManager::ProcessInput(HWND hWnd)
{
	
	if ( hWnd != GetFocus() || ImGui::IsAnyItemActive() )
	{
		return;
	}
	if ( GetKeyboardState(keyBuffer) )
	{
		for ( size_t i = 0; i < 256; i++ )
		{
			if ( keyBuffer[i] & 0xF0 )
			{
				switch ( keyStates[i] )
				{
					//클릭중
					case KeyState::Up:
					case KeyState::None:
						keyStates[i] = KeyState::Down;
						break;
					case KeyState::Down:
						keyStates[i] = KeyState::Pressing;
						break;
					case KeyState::Pressing:
						break;
				}
			}
			else
			{
				switch ( keyStates[i] )
				{
					//클릭 안함
					case KeyState::Down:
					case KeyState::Pressing:
						keyStates[i] = KeyState::Up;
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

	//Mouse
	POINT mouseBuffer;
	GetCursorPos(&mouseBuffer);
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

	mousePosition.x = mouseBuffer.x;
	mousePosition.y = mouseBuffer.y;

}
