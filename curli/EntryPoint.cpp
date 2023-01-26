#include <Application.h>
#include <stdio.h>

int main(int argc, char const *argv[])
{
	Application<TeapotRenderer> app(argc, argv);
	app.Run();

    return 0;
}
