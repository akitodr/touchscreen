#include "TouchWindow.h"
#include <stdio.h>
#include <SDL_image.h>
#include "InputHandler.h"


TouchWindow::TouchWindow(const char* title) : title(title) {}

TouchWindow::TouchWindow() : TouchWindow("TouchPad") {}

int runProjectionWindow(void * data)
{
	((Window*)data)->loop();
	return 0;
}

void TouchWindow::init()
{
	//Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL Error: %s\n", SDL_GetError());
	}
	else
	{
		//Set texture filtering to linear
		if (!SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "1"))
		{
			printf("Warning: Linear texture filtering not enabled!");
		}

		//Create window
		window = SDL_CreateWindow(title, 0, 0, 1920, 1080, SDL_WINDOW_BORDERLESS);
		if (window == NULL)
		{
			printf("Window could not be created! SDL Error: %s\n", SDL_GetError());
		}
		else
		{
			//Create renderer for window
			canvas = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
			if (canvas == NULL)
			{
				printf("Renderer could not be created! SDL Error: %s\n", SDL_GetError());
			}
			else {


				//Initialize PNG loading
				int imgFlags = IMG_INIT_PNG;
				if (!(IMG_Init(imgFlags) & imgFlags))
				{
					printf("SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
				}
			}
		}
	}

	std::string path = "vader\\";
	thumb = new Thumb(200, Vec2(1920 - 300, 1080 - 300));
	thumb->init(canvas);
	object = new ScannedObject(canvas);
	object->setupImages(path);
	object->init();
	
	projectionWindow = new Window();
	projectionWindow->init();
	projectionWindow->setThumb(thumb);
	projectionWindow->setObjectPath(path);
	thread = SDL_CreateThread(runProjectionWindow, "Projection Window", projectionWindow);

	rightButton = new Button(canvas, 300,50,50,50, true);
	rightButton->init();

	leftButton = new Button(canvas, 50, 50, 50, 50, false);
	leftButton->init();

	defViewPort();
}

void TouchWindow::loop() {
	//Main loop flag
	bool quit = false;

	SDL_Event e;

	init();

	float time = SDL_GetTicks();

	//While application is running
	while (!quit)
	{
		float secs = (SDL_GetTicks() - time) / 1000;
		time = SDL_GetTicks();

		InputHandler::instance().update();

		//F64740
		SDL_SetRenderDrawColor(canvas, 0x00, 0x00, 0x00, 0xFF);
		//Clear screen
		SDL_RenderClear(canvas);


		thumb->update();

		SDL_RenderSetViewport(canvas, &viewPort);
		object->incrementAngle(Vec2(thumb->getInputVector().x, -thumb->getInputVector().y) * 100 * secs);
		object->update(secs);
		//object->draw();

		SDL_RenderCopy(canvas, object->getCurrentTexture(), NULL, &objectRect);
		SDL_SetRenderDrawColor(canvas, 0xFF, 0x00, 0x00, 0xFF);
		SDL_RenderDrawRect(canvas, NULL);

		SDL_RenderSetViewport(canvas, NULL);
		thumb->draw();

		rightButton->update();
		leftButton->update();

		rightButton->draw();
		leftButton->draw();

		if (rightButton->isClicked())
			std::cout << "Clicked Right" << std::endl;

		if (leftButton->isClicked())
			std::cout << "Clicked Left" << std::endl;


		SDL_RenderPresent(canvas);

		//Handle events on queue
		while (SDL_PollEvent(&e) != 0)
		{
			//User requests quit (inputs)
			if (e.type == SDL_QUIT)
			{
				quit = true;
			}

			InputHandler::instance().handleInput(e);
		}
	}

	SDL_WaitThread(thread, NULL);
	thumb->deinit();
	object->deinit();
	close();
}

void TouchWindow::close() {

	//Destroy window
	SDL_DestroyRenderer(canvas);
	SDL_DestroyWindow(window);
	delete thumb;
	delete object;
	canvas = NULL;
	window = NULL;
	thumb = NULL;
	object = NULL;

	//Quit SDL subsystems
	IMG_Quit();
	SDL_Quit();
}

SDL_Renderer* TouchWindow::getCanvas() const
{
	return canvas;
}

void TouchWindow::defViewPort()
{
	int w, h;
	SDL_GetRendererOutputSize(canvas, &w, &h);
	viewPort.w = w * 0.4;
	viewPort.h = h * 0.9;
	viewPort.x = (h - viewPort.h) * 0.5;
	viewPort.y = (h - viewPort.h) * 0.5;

	int imgW, imgH;
	SDL_QueryTexture(object->getCurrentTexture(), NULL, NULL, &imgW, &imgH);

	float proportion = imgW / imgH;

	objectRect.w = viewPort.w;
	objectRect.h = objectRect.w * (imgH / imgW);
	objectRect.y = viewPort.h / 2 - objectRect.h / 2;

}
