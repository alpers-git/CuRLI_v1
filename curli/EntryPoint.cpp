#include <Application.h>
#include <stdio.h>
#include <Eigen/Core>

int main(int argc, char const *argv[])
{
	Eigen::initParallel();
	printf("Set Eigen thread count to %d\n", Eigen::nbThreads());
	Application<MultiTargetRenderer, gui::ControlPanel, FwEulerIntegrator> app(argc, argv);
	app.Run();

    return 0;
}
