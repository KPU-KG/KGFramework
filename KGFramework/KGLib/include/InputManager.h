#pragma once
#include <Windows.h>
namespace KG::Input
{
	struct MousePosition
	{
		int x;
		int y;
	};
	enum class KeyState
	{
		Down,
		Pressing,
		Up,
		None
	};
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
		void ProcessInput(HWND hWnd)
		{
			//Keyboard
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

			this->deltaPosition.x = mouseBuffer.x - mousePosition.x;
			this->deltaPosition.y = mouseBuffer.y - mousePosition.y;

			mousePosition.x = mouseBuffer.x;
			mousePosition.y = mouseBuffer.y;
			
		}

		KeyState GetKeyState( UINT keyId ) const
		{
			return this->keyStates[keyId];
		}
		bool IsTouching( UINT keyId ) const
		{
			return this->keyStates[keyId] == KeyState::Down ||
				this->keyStates[keyId] == KeyState::Pressing;
		}

		MousePosition GetCurrentMousePosition() const
		{
			return this->mousePosition;
		}

		MousePosition GetDeltaMousePosition() const
		{
			return this->deltaPosition;
		}

		static InputManager* GetInputManager()
		{
			if ( instance == nullptr )
				instance = new InputManager();
			return instance;
		}
	};
}