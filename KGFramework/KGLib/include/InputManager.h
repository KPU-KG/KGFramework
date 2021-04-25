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
		None,
		Down,
		Pressing,
		Up,
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
		bool startMouseCapture = false;

		InputManager() = default;
		void GetVKState(int keyId);
	public:
		void SetUIContext(void* context);


		LRESULT HandlingInputProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

		/// @brief �ش� �������� ����� �Է��� �а� �����մϴ�.
		/// @param hWnd �Է��� �޾ƿ� �������� �ڵ��Դϴ�.
		void ProcessInput(HWND hWnd);
		void PostProcessInput();

		void SetMouseCapture(bool startMouseCapture);
		bool GetMouseCapture() const;

		/// @brief �ش� Ű�� ���� ���¸� �����ɴϴ�.
		/// ���Ǵ� ID�� ������API�� �����ϰ� VK�� �����ϴ� Ư��Ű�� char�� ����մϴ�.
		/// @param keyId ���� Ű�� ID�Դϴ�. 
		/// @return ������ Ű�� �����Դϴ�.
		KeyState GetKeyState( UINT keyId ) const
		{
			return this->keyStates[toupper(keyId)];
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

		static void SetInputManager(InputManager* manager)
		{
			instance = manager;
		}
	};
}