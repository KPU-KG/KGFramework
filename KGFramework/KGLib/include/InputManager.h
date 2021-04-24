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
		None,
		Down,
		Pressing,
		Up,
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
		bool startMouseCapture = false;

		InputManager() = default;
		void GetVKState(int keyId);
	public:
		void SetUIContext(void* context);


		LRESULT HandlingInputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		/// @brief 해당 프레임의 사용자 입력을 읽고 저장합니다.
		/// @param hWnd 입력을 받아올 윈도우의 핸들입니다.
		void ProcessInput(HWND hWnd);
		void PostProcessInput();

		void SetMouseCapture(bool startMouseCapture);
		bool GetMouseCapture() const;

		/// @brief 해당 키의 현재 상태를 가져옵니다.
		/// 사용되는 ID는 윈도우API와 동일하게 VK로 시작하는 특수키와 char로 기능합니다.
		/// @param keyId 구할 키의 ID입니다. 
		/// @return 지정한 키의 상태입니다.
		KeyState GetKeyState( UINT keyId ) const
		{
			return this->keyStates[toupper(keyId)];
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

		static void SetInputManager(InputManager* manager)
		{
			instance = manager;
		}
	};
}