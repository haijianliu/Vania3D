
#ifndef Input_hpp
#define Input_hpp

enum PS4_CONTROLLER {
	JOY_SQUARE, JOY_CROSS, JOY_CIRCLE, JOY_TRIANGLE,
	JOY_L1, JOY_R1, JOY_L2, JOY_R2,
	JOY_SHARE, JOY_OPTION,
	JOY_L3, JOY_R3, JOY_HOME, JOY_TOUCHPAD,
	JOY_UP, JOY_RIGHT, JOY_DOWN, JOY_LEFT
};

class Input {
private:
	int keys[GLFW_KEY_LAST] = {GLFW_RELEASE};
	int joyButtons[25] = {GLFW_RELEASE};
	int joyButtonCount, joyEvent = GLFW_DISCONNECTED, joyConnect = 0;

	Input();

	void updateJoystick(int button);

public:
	static Input* getInstance();
	~Input();

	// joystick
	void joystickcallback(int joy, int event);
	bool getJoystickPress(int button);
	bool getJoystickTrigger(int button);

	// keyboard
	void keycallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	bool getButtonPress(int button);
	bool getButtonTrigger(int button);
};

#endif /* Input_hpp */