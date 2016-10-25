#include "Globals.h"
#include "Application.h"
#include "ModuleRender.h"
#include "ModuleSceneIntro.h"
#include "ModuleInput.h"
#include "ModuleTextures.h"
#include "ModuleAudio.h"
#include "ModulePhysics.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	circle = ball_tex = background = NULL;
	ball = NULL;
	ray_on = false;
	sensed = false;
	score = 0;
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	App->renderer->camera.x = App->renderer->camera.y = 0;

	circle = App->textures->Load("pinball/wheel.png"); 
	ball_tex = App->textures->Load("pinball/ball.png");
	background = App->textures->Load("pinball/background.png");
	flipper_fx = App->audio->LoadFx("pinball/Flipper_fx.wav");
	triangle_fx = App->audio->LoadFx("pinball/triangle_fx.wav");
	bumper_fx = App->audio->LoadFx("pinball/bumper_fx.wav");

	App->audio->PlayMusic("pinball/pinballmusic.ogg");

	sensor = App->physics->CreateRectangleSensor(SCREEN_WIDTH / 2, SCREEN_HEIGHT, SCREEN_WIDTH, 50);

	LoadMap();

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update: draw background
update_status ModuleSceneIntro::Update()
{
	if(App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		ray_on = !ray_on;
		ray.x = App->input->GetMouseX();
		ray.y = App->input->GetMouseY();
	}

	if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
	{

		ball = App->physics->CreateCircle(App->input->GetMouseX(), App->input->GetMouseY(), 10, b2BodyType::b2_dynamicBody);
		ball->listener = this;
		
	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
	{

		left_joint->EnableMotor(true);
		left_joint->SetMaxMotorTorque(100);
		left_joint->SetMotorSpeed(-800 * DEGTORAD);

	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_UP)
	{
		left_joint->SetMotorSpeed(800 * DEGTORAD);
	}

	if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_DOWN)
	{
		App->audio->PlayFx(flipper_fx);


	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
	{

		right_joint->EnableMotor(true);
		right_joint->SetMaxMotorTorque(100);
		right_joint->SetMotorSpeed(800 * DEGTORAD);

	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_UP)
	{
		right_joint->SetMotorSpeed(-800 * DEGTORAD);
	}

	if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_DOWN)
	{
		App->audio->PlayFx(flipper_fx);
	}

	// Prepare for raycast ------------------------------------------------------
	
	iPoint mouse;
	mouse.x = App->input->GetMouseX();
	mouse.y = App->input->GetMouseY();
	int ray_hit = ray.DistanceTo(mouse);

	fVector normal(0.0f, 0.0f);

	// All draw functions ------------------------------------------------------

	App->renderer->Blit(background, 0, 0);
	if (ball) {
		int ballx, bally;
		ball->GetPosition(ballx, bally);
		App->renderer->Blit(ball_tex, ballx, bally);
	}



	// ray -----------------
	if(ray_on == true)
	{
		fVector destination(mouse.x-ray.x, mouse.y-ray.y);
		destination.Normalize();
		destination *= ray_hit;

		App->renderer->DrawLine(ray.x, ray.y, ray.x + destination.x, ray.y + destination.y, 255, 255, 255);

		if(normal.x != 0.0f)
			App->renderer->DrawLine(ray.x + destination.x, ray.y + destination.y, ray.x + destination.x + normal.x * 25.0f, ray.y + destination.y + normal.y * 25.0f, 100, 255, 100);
	}

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody* bodyA, PhysBody* bodyB)
{
	int x, y;

	if (bodyA == ball) {

		//App->audio->PlayFx(flipper_fx);


		int index = bumpers.find(bodyB);
		if (index != -1) {

			App->audio->PlayFx(bumper_fx);

			score += 100;

			b2Vec2 bouncing_force;
			bouncing_force.x = bodyA->body->GetPosition().x - bodyB->body->GetPosition().x;
			bouncing_force.y = bodyA->body->GetPosition().y - bodyB->body->GetPosition().y;
			bouncing_force.x *= 1.5;
			bouncing_force.y *= 1.5;
			bodyA->body->ApplyLinearImpulse(bouncing_force, bodyA->body->GetLocalCenter(), true);
		}
		else if (bodyB == left_bouncer) {
			b2Vec2 bouncing_force;
			bouncing_force.Set(0.75, -1.25);
			bodyA->body->ApplyLinearImpulse(bouncing_force, bodyA->body->GetLocalCenter(), true);
			App->audio->PlayFx(triangle_fx);
		}

		else if (bodyB == right_bouncer) {
			b2Vec2 bouncing_force;
			bouncing_force.Set(-0.75, -1.25);
			bodyA->body->ApplyLinearImpulse(bouncing_force, bodyA->body->GetLocalCenter(), true);
		}

	}
	/*
	if(bodyA)
	{
		bodyA->GetPosition(x, y);
		App->renderer->DrawCircle(x, y, 50, 100, 100, 100);
	}

	if(bodyB)
	{
		bodyB->GetPosition(x, y);
		App->renderer->DrawCircle(x, y, 50, 100, 100, 100);
	}*/
}

void ModuleSceneIntro::LoadMap() {

	chains.add(App->physics->CreateChain(0, 0, pinball, 338, b2BodyType::b2_staticBody));
	chains.add(App->physics->CreateChain(0, 0, bottom_right, 38, b2BodyType::b2_staticBody));
	chains.add(App->physics->CreateChain(0, 0, bottom_left, 46, b2BodyType::b2_staticBody));

	chains.add(App->physics->CreateChain(0, 0, bouncer_block_right, 14, b2BodyType::b2_staticBody));
	chains.add(App->physics->CreateChain(0, 0, bouncer_block_left, 28, b2BodyType::b2_staticBody));

	left_bouncer = App->physics->CreateChain(93, 638, bouncer_left, 8, b2BodyType::b2_staticBody);

	flippers.add(App->physics->CreateFlipper(129, 746, left_flipper));
	joint_anchors.add(App->physics->CreateCircle(129, 746, 10, b2BodyType::b2_staticBody));

	b2Vec2 left_anchor(-0.2, -0.15);
	b2Vec2 left_flipper_anchor(0, 0);
	left_joint = App->physics->CreateJoint(joint_anchors.getLast()->data, flippers.getLast()->data, left_anchor, left_flipper_anchor);

	flippers.add(App->physics->CreateFlipper(324, 746, right_flipper));
	joint_anchors.add(App->physics->CreateCircle(319, 746, 10, b2BodyType::b2_staticBody));

	b2Vec2 right_anchor(0.2, -0.15);
	b2Vec2 right_flipper_anchor(0, 0);
	right_joint = App->physics->CreateJoint(joint_anchors.getLast()->data, flippers.getLast()->data, right_anchor, right_flipper_anchor);


	bumpers.add(App->physics->CreateCircle(199, 224, 24, b2BodyType::b2_staticBody));
	bumpers.add(App->physics->CreateCircle(283, 220, 30, b2BodyType::b2_staticBody));
	bumpers.add(App->physics->CreateCircle(283, 315, 40, b2BodyType::b2_staticBody));
	bumpers.add(App->physics->CreateCircle(190, 302, 30, b2BodyType::b2_staticBody));



}
