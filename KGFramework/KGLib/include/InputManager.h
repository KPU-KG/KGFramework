#pragma once
#include <Windows.h>
namespace KG::Input
{
	/// @brief 마우스 좌표계 구조체입니다.
	struct MousePosition
	{
		int x;
		int y;
	};
	/// @brief 키 상태 열거형입니다.
	enum class KeyState
	{
		Down,
		Pressing,
		Up,
		None
	};

	/// @brief 사용자 입력을 저장, 관리하는 매니저 클래스 입니다.
	/// 싱글턴 매니저이며 프레임워크에서 한번 생성됩니다.
	class InputManager
	{
		static inline InputManager* instance = nullptr;

		unsigned char keyBuffer[256];
		KeyState keyStates[256];

		MousePosition mousePosition;
		MousePosition deltaPosition;

		bool startProcessing = false;

		InputManager() = default;
	public:
		/// @brief 해당 프레임의 사용자 입력을 읽고 저장합니다.
		/// @param hWnd 입력을 받아올 윈도우의 핸들입니다.
		void ProcessInput(HWND hWnd)
		{
			if ( hWnd != GetFocus() )
			{
				return;
			}
			if ( GetKeyboardState( keyBuffer ) )
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
			GetCursorPos( &mouseBuffer );
			ScreenToClient( hWnd , &mouseBuffer);
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

		/// @brief 해당 키의 현재 상태를 가져옵니다.
		/// 사용되는 ID는 윈도우API와 동일하게 VK로 시작하는 특수키와 char로 기능합니다.
		/// @param keyId 구할 키의 ID입니다. 
		/// @return 지정한 키의 상태입니다.
		KeyState GetKeyState( UINT keyId ) const
		{
			return this->keyStates[keyId];
		}

		/// @brief 해당 키가 현재 눌려져 있는지 확인합니다.
		/// 사용되는 ID는 윈도우API와 동일하게 VK로 시작하는 특수키와 char로 기능합니다.
		/// Down 상태와 Pressing 상태일때 True입니다.
		/// @param keyId 구할 키의 ID입니다. 
		/// @return 지정한 키의 눌림 여부 입니다.
		bool IsTouching( UINT keyId ) const
		{
			return this->keyStates[keyId] == KeyState::Down ||
				this->keyStates[keyId] == KeyState::Pressing;
		}

		/// @brief 현재 마우스의 위치를 가져옵니다.
		/// @return 화면 위의 마우스 위치 좌표입니다.
		MousePosition GetCurrentMousePosition() const
		{
			return this->mousePosition;
		}

		/// @brief 현재 마우스의 이동한 값을 가져옵니다.
		/// 이전 프레임의 Process와 현 프레임 사이의 Delta 값입니다.
		/// @return 화면 위의 마우스 위치 좌표입니다.
		MousePosition GetDeltaMousePosition() const
		{
			return this->deltaPosition;
		}

		/// @brief 인풋매니저 인스턴스를 가져옵니다.
		/// @return 인풋매니저의 인스턴스입니다.
		static InputManager* GetInputManager()
		{
			if ( instance == nullptr )
				instance = new InputManager();
			return instance;
		}
	};
}