#include <iostream>
#include <vector>

#include <SDL.h>
#include <SDL_image.h>

using namespace std;

SDL_Window* window;
SDL_Renderer* renderer;
bool isRunning = true;

typedef struct {
	float x;
	float y;
} Vector2d;

struct Player {
	Vector2d pos;
	Vector2d vel;
	SDL_Rect rect;
	SDL_Texture* texture;
	double rotation = 0;
};

Player player;

struct Mouse {
	int x;
	int y;
};

Mouse mouse;

struct Boundry {
	Vector2d a;
	Vector2d b;
	Vector2d intersection;
	Vector2d reflection;
};

Boundry boundry;
Boundry boundry2;
Boundry boundry3;
Vector2d intersection;
Vector2d reflection;

vector<Boundry> boundries;

SDL_Texture* LoadTexture(string filepath) {
	SDL_Texture* texture = nullptr;
	SDL_Surface* surface = IMG_Load(filepath.c_str());

	if (surface == nullptr) {
		cout << "Failed to set surface: " << IMG_GetError() << endl;
	}

	texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (texture == nullptr) {
		cout << "Failed to set texture: " << SDL_GetError() << endl;
	}

	return texture;
}

void Init() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		cout << "SDL failed to initialize: " << SDL_GetError() << endl;
	}

	if (IMG_Init(IMG_INIT_PNG) == 0) {
		cout << "SDL_Image failed to initialize: " << IMG_GetError() << endl;
	}

	window = SDL_CreateWindow("Escape", 400, 100, 800, 600, 0);
	if (window == NULL) {
		cout << "Window failed to initialize: " << SDL_GetError() << endl;
	}

	renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (renderer == NULL) {
		cout << "Renderer failed to initialize: " << SDL_GetError() << endl;
	}

	player.pos = { 350.f, 200.f };
	player.vel = { 0.f, 0.f };
	player.rect = { (int)player.pos.x - 16, (int)player.pos.y - 16, 32, 32 };
	player.texture = LoadTexture("player.png");

	boundry.a.x = 400;
	boundry.a.y = 100;
	boundry.b.x = 400;
	boundry.b.y = 400;

	boundry2.a.x = 400;
	boundry2.a.y = 100;
	boundry2.b.x = 100;
	boundry2.b.y = 100;

	boundry3.a.x = 300;
	boundry3.a.y = 100;
	boundry3.b.x = 300;
	boundry3.b.y = 400;

	boundries.emplace_back(boundry);
	boundries.emplace_back(boundry2);
	boundries.emplace_back(boundry3);
}

void HandleEvents() {
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			isRunning = false;
		}
		if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) {
			isRunning = false;
		}
	}

	const Uint8* keystates = SDL_GetKeyboardState(NULL);

	if (keystates[SDL_SCANCODE_S]) {
		player.pos.x -= 5;
	}
	if (keystates[SDL_SCANCODE_F]) {
		player.pos.x += 5;
	}
	if (keystates[SDL_SCANCODE_E]) {
		player.pos.y -= 5;
	}
	if (keystates[SDL_SCANCODE_D]) {
		player.pos.y += 5;
	}
}

void Update() {
	player.rect.x = (int)player.pos.x - 16;
	player.rect.y = (int)player.pos.y - 16;

	SDL_GetMouseState(&mouse.x, &mouse.y);

	double radians = atan2(mouse.y - player.pos.y, mouse.x - player.pos.x);
	double degrees = radians * 180 / M_PI;

	player.rotation = degrees;

	// 1 = player
	// 2 = mouse
	// 3 = boundry a
	// 4 = boundry b

	// Loop through boundries and check line intersection, store intersection points, and store reflection points
	for (int i = 0; i < boundries.size(); i++)
	{
		float den = ((player.pos.x - mouse.x) * (boundries[i].a.y - boundries[i].b.y)) - ((player.pos.y - mouse.y) * (boundries[i].a.x - boundries[i].b.x));

		if (den == 0) {
			return;
		}

		// T - ((x1 - x3) * (y3 - y4)) - ((y1 - y3) * (x3 - x4))
		float t = (((player.pos.x - boundries[i].a.x) * (boundries[i].a.y - boundries[i].b.y)) - ((player.pos.y - boundries[i].a.y) * (boundries[i].a.x - boundries[i].b.x))) / den;

		// U - ((x1 - x3) * (y1 - y2)) - ((y1 - y3) * (x1 - x2))
		float u = (((player.pos.x - boundries[i].a.x) * (player.pos.y - mouse.y)) - ((player.pos.y - boundries[i].a.y) * (player.pos.x - mouse.x))) / den;


		if (t > 0 && t < 1 && u > 0 && u < 1) {
			boundries[i].intersection = { player.pos.x + (t * (mouse.x - player.pos.x)), player.pos.y + (t * (mouse.y - player.pos.y)) };

			// Get distance from intersection to mouse position
			float dis = sqrt(pow(mouse.x - boundries[i].intersection.x, 2) + pow(mouse.y - boundries[i].intersection.y, 2));

			// Reflect this ray
			float reflectionRadians = atan2(mouse.y - boundries[i].intersection.y, mouse.x - boundries[i].intersection.x);
			boundries[i].reflection.x = boundries[i].intersection.x - dis * cos(-reflectionRadians);
			boundries[i].reflection.y = boundries[i].intersection.y - dis * sin(-reflectionRadians);
		}
		else {
			boundries[i].intersection = { NULL, NULL };
			boundries[i].reflection = { NULL, NULL };
		}
	}


	// denominator
	// ((x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4))
	//float den = ((player.pos.x - mouse.x) * (boundry.a.y - boundry.b.y)) - ((player.pos.y - mouse.y) * (boundry.a.x - boundry.b.x));

	//if (den == 0) {
	//	return;
	//}
	//
	//// T - ((x1 - x3) * (y3 - y4)) - ((y1 - y3) * (x3 - x4))
	//float t = (((player.pos.x - boundry.a.x) * (boundry.a.y - boundry.b.y)) - ((player.pos.y - boundry.a.y) * (boundry.a.x - boundry.b.x))) / den;

	//// U - ((x1 - x3) * (y1 - y2)) - ((y1 - y3) * (x1 - x2))
	//float u = (((player.pos.x - boundry.a.x) * (player.pos.y - mouse.y)) - ((player.pos.y - boundry.a.y) * (player.pos.x - mouse.x))) / den;


	//if (t > 0 && t < 1 && u > 0 && u < 1) {
	//	intersection = { player.pos.x + (t * (mouse.x - player.pos.x)), player.pos.y + (t * (mouse.y - player.pos.y)) };
	//	
	//	// Get distance from intersection to mouse position
	//	float dis = sqrt(pow(mouse.x - intersection.x, 2) + pow(mouse.y - intersection.y, 2));
	//	cout << "DIS: " << dis << endl;

	//	// Reflect this ray
	//	float reflectionRadians = atan2(mouse.y - intersection.y, mouse.x - intersection.x);
	//	reflection.x = intersection.x - dis * cos(-reflectionRadians);
	//	reflection.y = intersection.y - dis * sin(-reflectionRadians);
	//	cout << "REFLECTION RAD: " << reflectionRadians << endl;
	//}
	//else {
	//	intersection.x = NULL;
	//	intersection.y = NULL;
	//	reflection.x = NULL;
	//	reflection.y = NULL;
	//}
}


void Render() {
	SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
	SDL_RenderClear(renderer);

	// Ray
	//SDL_SetRenderDrawColor(renderer, 255, 20, 255, 255);
	//if (intersection.x && intersection.y) {
	//	SDL_RenderDrawLine(renderer, player.pos.x, player.pos.y, intersection.x, intersection.y);
	//}
	//else {
	//	SDL_RenderDrawLine(renderer, player.pos.x, player.pos.y, mouse.x, mouse.y);
	//}

	// Character
	SDL_RenderCopyEx(renderer, player.texture, NULL, &player.rect, player.rotation, NULL, SDL_FLIP_NONE);

	// Boundries
	for (int i = 0; i < boundries.size(); i++) {
		SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
		SDL_RenderDrawLine(renderer, boundries[i].a.x, boundries[i].a.y, boundries[i].b.x, boundries[i].b.y);


		// Ray
		SDL_SetRenderDrawColor(renderer, 255, 20, 255, 255);
		if (boundries[i].intersection.x && boundries[i].intersection.y) {
			SDL_RenderDrawLine(renderer, player.pos.x, player.pos.y, boundries[i].intersection.x, boundries[i].intersection.y);
		}
		else {
			SDL_RenderDrawLine(renderer, player.pos.x, player.pos.y, mouse.x, mouse.y);
		}


		// Intersection
		if (boundries[i].intersection.x && boundries[i].intersection.y) {
			SDL_Rect temp = { boundries[i].intersection.x - 3, boundries[i].intersection.y - 3, 6, 6 };
			SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
			SDL_RenderFillRect(renderer, &temp);
		}


		// Reflection Point
		SDL_RenderDrawPoint(renderer, boundries[i].reflection.x, boundries[i].reflection.y);
		SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
		SDL_RenderDrawLine(renderer, boundries[i].intersection.x, boundries[i].intersection.y, boundries[i].reflection.x, boundries[i].reflection.y);
	}


	SDL_RenderPresent(renderer);
}

void Clean() {
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

int main(int argc, char* argv[]) {
	Init();

	while (isRunning) {
		HandleEvents();
		Update();
		Render();
	}

	Clean();

	return 0;
}