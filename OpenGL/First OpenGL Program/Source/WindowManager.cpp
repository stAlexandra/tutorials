#include "GL\glew.h"
#include "GLFW\glfw3.h"
#include "../Headers/WindowManager.h"
#include "../Headers/Main.h"

/*
This file is created to abstract out any environment information that will be used to create
the window and handle input.  Depending on the API used to create the window, it will have
different functions on how to create an OpenGL Context, which is used to store the OpenGL
state and handle interaction between the framebuffer, which is swapped to the front when
all objects have been drawn each frame (used for double buffering).  When the context is
destroyed, OpenGL is destroyed.

Currently we are using the third-party library GLFW to handle cross-platform compiling
so that Windows, Mac OSX and Linux users can compile the tutorials, where before GT was
just focused on Windows (Win32) development.  You can consider this our attempt to
broaden our tent to the other technologies :)  If you want to use Win32, you can just
add the WinMain and code to this file, replacing the main() function below, then just
replace the rest of the functions internals to be Win32 specific.  You would then use
the Window void pointer to be the handle to the window.  A better implementation would
be to just use this as the base class and inherit your own Win32WindowManager, then
you can override the virtual functions in the WindowManager base class.  I will try and
create a tutorial with this for those who are interested.
*/

// This is the entry point into our application, which is just a console application.
// Using the GLFW library, this allows us to create a separate window for our OpenGL rendering
// that is cross-platform.  If you wanted to use Win32, you would replace this with WinMain.
int main()
{
	// Create a local instance of our GLApplication (defined in Main.cpp).
	GLApplication application;
	
	// Return the GLMain() defined in Main.cpp, which handles the flow of our application
	// and immediately starts our game loop.
	return application.GLMain();
}


// This initializes our window and creates the OpenGL context
int WindowManager::Initialize(int width, int height, std::string strTitle, bool bFullScreen)
{
	// This tries to first init the GLFW library and make sure it is available
	if ( !glfwInit() )
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}

	// Now we need to use GLFW to tell OpenGL what settings we want, and most importantly, 
	// which version of OpenGL we are using.  We do this with glfwWindowHint() function calls.
	// See http://www.glfw.org/docs/latest/window.html#window_hints for all the window hints.

	// This tells OpenGL that our OpenGL context will have a multisampling value of 4, which means
	// it will use antialiasing to smooth out the jagged pixels when rendering.  So it will same
	// the final rendered image 4 times to make it smooth as silk.  Comment this out to see it jagged.
	glfwWindowHint(GLFW_SAMPLES, 4);

	// This requires that we must be using OpenGL 4 and by specifying the CORE_PROFILE we are saying
	// that we don't want to support any older OpenGL version and don't care about compatibility.
	// If you want to use older OpenGL functions, you would need to pass in GLFW_OPENGL_COMPAT_PROFILE.
	// If your computer doesn't run with 4, try changing it to 3 or download the latest drivers here:
	// https://www.opengl.org/wiki/Getting_Started
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// This next function creates the window with the settings given above. It takes the window width 
	// and height, the title, and monitor information that tells GLFW if we want a fullscreen window or not.
	// To make the window fullscreen you pass in glfwGetPrimaryMonitor() to the second to last parameter.
	if( bFullScreen )
		Window = glfwCreateWindow(width, height, strTitle.c_str(), glfwGetPrimaryMonitor(), nullptr);
	else
		Window = glfwCreateWindow(width, height, strTitle.c_str(), nullptr, nullptr);

	// Make sure the window is valid, if not, throw an error.
	if ( Window == nullptr )
	{
		fprintf(stderr, "Failed to create a GLFW window, you might need to download the latest drivers or change the OpenGL version to 3\n");
		Destroy();

		return -1;
	}

	// Create the OpenGL context from the window and settings specified.  Notice that we have to cast
	// the Window object as a GLFWwindow* object, this is so that our WindowManager isn't coupled to
	// the GLFW class, but can be inherited from to create your own Win32 or GLUT window setup.
	glfwMakeContextCurrent((GLFWwindow*)Window);

	// This turns on STICKY_KEYS for keyboard input, so that glfwgetKey() returns GLFW_PRESS the next 
	// time it's called if the key has been released before the call.  We pass in our casted Window 
	// object as the first parameter, then the settings to turn STICKY_KEYS set to TRUE.
	// Keyboard input will still work without this, just not "sticky".
	glfwSetInputMode((GLFWwindow*)Window, GLFW_STICKY_KEYS, GL_TRUE);

	// This is an important part, at least for my system I had to set this parameter to true otherwise
	// OpenGL wouldn't work and the GLEW library would fail to load.  Apparently, this is because by
	// setting it to Experimental, it goes and search for all supported OpenGL functions even if the
	// video card doesn't mark them as standard.  So be sure to set this TRUE if you have the same issue.
	glewExperimental = GL_TRUE;

	// Initialize the GLEW library so that it can go and find the correct OpenGL functions for the
	// current environment, and any new functions that were defined after OpenGL 1.1, since if you
	// are on Windows for instance, the gl.h library won't recognize anything after version 1.1.
	// That means no shader support or OpenGL 4 support.  We could define them ourselves like we
	// did in our old shader tutorials, but this just makes it easier and has become a standard.
	GLenum err = glewInit();

	// If we had an error, return -1.  Be sure to see if glewExperimental isn't true, this worked for me.
	if ( GLEW_OK != err )
	{
		fprintf(stderr, "Failed to initialize glew\n");
		return -1;
	}

	// Return success
	return 0;
}


// This swaps the backbuffer with the front buffer to display the content rendered in OpenGL
void WindowManager::SwapBuffers()
{
	// This takes the Window and swaps the backbuffer to the front.  This way we don't see tearing
	// as the content is drawn to the screen each frame.  This is called double buffering.  This
	// should be called at the end of the Render() function in the GLApplication, once all content
	// in done rendering.  We cast the Window object to it's original type, a GLFWwindow*.
	glfwSwapBuffers((GLFWwindow*)Window);
}


// This function processes all the application's input and returns a bool to tell us if we should continue
bool WindowManager::ProcessInput(bool continueGame = true)
{
	// Use the GLFW function to check for the user pressing the Escape button, as well as a window close event.
	// If any of these checks return true, return false back to the caller to let them know the user has quit.
	if ( glfwGetKey((GLFWwindow*)Window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose((GLFWwindow*)Window) != 0 )
		return false;

	// Poll the input events to see if the user quit or closed the window.  This can only be called
	// in the main thread of the application, so apparently you can't spin off another thread for this.
	glfwPollEvents();

	// Return the value passed in to tell the game loop that we should continue or not.  This would be used
	// to have an external variable tracking the state of the game, like from a menu or something.
	return continueGame;
}


// This destroys the window
void WindowManager::Destroy()
{
	// This closes the OpenGL window and terminates the application
	glfwTerminate();
}


/////////////////////////////////////////////////////////////////////////////////
//
// * QUICK NOTES * 
//
// This file is to abstract our window code from the rest of the application so
// that we can change or inherit from to do another API like Win32, GLUT or SDL.
// We are using the cross-platform GLFW library which should allow you to compile
// this on Windows, Mac OSX or Linux by using the necessary compilers.  We'll try
// include these cross-platform projects for Mac and Linux so you don't have to
// generate them yourself.  You will also need to make sure you have the library
// compiled for your environment instead of the Windows environment in this 
// tutorial.  Please refer to the Main.cpp file for the start of this tutorial.
// 
//
// � 2000-2014 GameTutorials