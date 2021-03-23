#pragma once
#include <Windows.h>
namespace KG::Input
{
	/// @brief ���콺 ��ǥ�� ����ü�Դϴ�.
	struct MousePosition
	{
		int x;
		int y;
	};
	/// @brief Ű ���� �������Դϴ�.
	enum class KeyState
	{
		Down,
		Pressing,
		Up,
		None
	};

	/// @brief ����� �Է��� ����, �����ϴ� �Ŵ��� Ŭ���� �Դϴ�.
	/// �̱��� �Ŵ����̸� �����ӿ�ũ���� �ѹ� �����˴ϴ�.
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
		/// @brief �ش� �������� ����� �Է��� �а� �����մϴ�.
		/// @param hWnd �Է��� �޾ƿ� �������� �ڵ��Դϴ�.
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
			GetCursorPos( &mouseBuffer );
			ScreenToClient( hWnd , &mouseBuffer);
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

		/// @brief �ش� Ű�� ���� ���¸� �����ɴϴ�.
		/// ���Ǵ� ID�� ������API�� �����ϰ� VK�� �����ϴ� Ư��Ű�� char�� ����մϴ�.
		/// @param keyId ���� Ű�� ID�Դϴ�. 
		/// @return ������ Ű�� �����Դϴ�.
		KeyState GetKeyState( UINT keyId ) const
		{
			return this->keyStates[keyId];
		}

		/// @brief �ش� Ű�� ���� ������ �ִ��� Ȯ���մϴ�.
		/// ���Ǵ� ID�� ������API�� �����ϰ� VK�� �����ϴ� Ư��Ű�� char�� ����մϴ�.
		/// Down ���¿� Pressing �����϶� True�Դϴ�.
		/// @param keyId ���� Ű�� ID�Դϴ�. 
		/// @return ������ Ű�� ���� ���� �Դϴ�.
		bool IsTouching( UINT keyId ) const
		{
			return this->keyStates[keyId] == KeyState::Down ||
				this->keyStates[keyId] == KeyState::Pressing;
		}

		/// @brief ���� ���콺�� ��ġ�� �����ɴϴ�.
		/// @return ȭ�� ���� ���콺 ��ġ ��ǥ�Դϴ�.
		MousePosition GetCurrentMousePosition() const
		{
			return this->mousePosition;
		}

		/// @brief ���� ���콺�� �̵��� ���� �����ɴϴ�.
		/// ���� �������� Process�� �� ������ ������ Delta ���Դϴ�.
		/// @return ȭ�� ���� ���콺 ��ġ ��ǥ�Դϴ�.
		MousePosition GetDeltaMousePosition() const
		{
			return this->deltaPosition;
		}

		/// @brief ��ǲ�Ŵ��� �ν��Ͻ��� �����ɴϴ�.
		/// @return ��ǲ�Ŵ����� �ν��Ͻ��Դϴ�.
		static InputManager* GetInputManager()
		{
			if ( instance == nullptr )
				instance = new InputManager();
			return instance;
		}
	};
}