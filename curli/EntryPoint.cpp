#include <stdio.h>
#include <Application.h>

int main(int argc, char const *argv[])
{
	Application<TeapotRenderer> app(argc, argv);
	app.Run();

    return 0;
}
