#include "InputManager.h"
#include "ImguiHelper.h"

/// @brief �ش� �������� ����� �Է��� �а� �����մϴ�.
/// @param hWnd �Է��� �޾ƿ� �������� �ڵ��Դϴ�.

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
					//Ŭ����
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
					//Ŭ�� ����
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

	//���콺 ��ȭ ��
	this->deltaPosition.x = mouseBuffer.x - mousePosition.x;
	this->deltaPosition.y = mouseBuffer.y - mousePosition.y;

	mousePosition.x = mouseBuffer.x;
	mousePosition.y = mouseBuffer.y;

}
