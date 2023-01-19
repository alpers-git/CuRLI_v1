#include <stdio.h>
#include <GLFW/glfw3.h>

class GLFWHandler
{
public:
	static GLFWHandler& GetInstance()
	{
		static GLFWHandler instance;
		return instance;
	}
	GLFWHandler(GLFWHandler const&) = delete;
	void operator=(GLFWHandler const&) = delete;
	void InitAndCreateWindow(int width=800, int height=600, const char* title="CuRLI");
	void Update();
	void Close();
	bool IsRunning();
	void SetWindowSize(int width, int height);
	void SetWindowPosition(int x, int y);
	void SetWindowName(const char* name);
	void SetWindowIcon(const char* path);
	void SetWindowIcon(const char* path, int width, int height);
	void SetKeyboardCallback(GLFWkeyfun callback);

	
	inline GLFWwindow* GetWindowPointer() { return windowHandle; }
	
private:
	GLFWHandler();
	~GLFWHandler();
	bool running;
	GLFWwindow* windowHandle=NULL;
};


