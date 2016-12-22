#include "relwarb.h"
#include "relwarb_math.h"
#include "relwarb_renderer.h"
#include "relwarb_opengl.h"
#include "relwarb_debug.h"
#include "relwarb_parser.h"
#include "relwarb_editor.h"

// TODO(Charly): This should go somewhere else.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void InitGame(GameState* gameState)
{
	LoadBitmapData("assets/health_full.png", &gameState->hudHealth[0]);
	LoadBitmapData("assets/health_mid.png", &gameState->hudHealth[1]);
	LoadBitmapData("assets/health_none.png", &gameState->hudHealth[2]);
	LoadBitmapData("assets/mana_full.png", &gameState->hudMana[0]);
	LoadBitmapData("assets/mana_none.png", &gameState->hudMana[1]);

    InitializeRenderer();
    gameState->projMatrix = Ortho(-gameState->viewportSize.x / 2, gameState->viewportSize.x / 2, 
                                  -gameState->viewportSize.y / 2, gameState->viewportSize.y / 2);
    real32 ratio = gameState->viewportSize.x / gameState->viewportSize.y;

	// NOTE(Thomas): Seems like worldSize should be the one we define, and windows size/viewport size are computed accordingly.
    gameState->worldSize = Vec2(48, 24);

    Mat4 worldMat = {0};
    worldMat[0][0] = (gameState->viewportSize.x / 2.f) / (gameState->worldSize.x / 2.f);
    worldMat[1][1] = (gameState->viewportSize.y / 2.f) / (gameState->worldSize.y / 2.f);
    worldMat[2][2] = 1.f;
    worldMat[3][3] = 1.f;
    //worldMat[3][1] = -gameState->viewportSize.y / 2.f;
    gameState->worldMatrix = worldMat;

    gameState->gravity = Vec2(0.f, -1.f);

	Vec2 halfSize = gameState->worldSize * 0.5f;

	// NOTE(Thomas): Must be before any other data is created, as indices are hardcoded in the file
	LoadMapFile(gameState, "config/base_map.ini");
    
	Bitmap* bitmap_p1[2];
	bitmap_p1[0] = CreateBitmap(gameState);
    LoadBitmapData("assets/p1_stand.png", bitmap_p1[0]);
	bitmap_p1[1] = CreateBitmap(gameState);
	LoadBitmapData("assets/p1_stand2.png", bitmap_p1[1]);

	Sprite* sprite_p1 = CreateTimeSprite(gameState, 2, bitmap_p1, 0.5f);

	Bitmap* bitmap_p2[2];
	bitmap_p2[0] = CreateBitmap(gameState);
	LoadBitmapData("assets/p2_stand.png", bitmap_p2[0]);
	bitmap_p2[1] = CreateBitmap(gameState);
	LoadBitmapData("assets/p2_stand2.png", bitmap_p2[1]);

	Sprite* sprite_p2 = CreateTimeSprite(gameState, 2, bitmap_p2, 0.5f);

	uint8 tiles_indices[] = { 1 };
	
	Shape* heroShape = CreateShape(gameState, Vec2(1.f, 1.5f));

	RenderingPattern* heroPattern1 = CreateUniqueRenderingPattern(gameState, sprite_p1);
	CreatePlayerEntity(gameState, Vec2(-2, -2), heroPattern1, heroShape, &gameState->controllers[0]);

	
	RenderingPattern* heroPattern2 = CreateUniqueRenderingPattern(gameState, sprite_p2);
	CreatePlayerEntity(gameState, Vec2(2, -2), heroPattern2, heroShape, &gameState->controllers[1]);
	
    LoadBitmapData("assets/corner_topleft.png", CreateBitmap(gameState));
    LoadBitmapData("assets/horizontal_up.png", CreateBitmap(gameState));
    LoadBitmapData("assets/corner_topright.png", CreateBitmap(gameState));
    LoadBitmapData("assets/vertical_left.png", CreateBitmap(gameState));
    LoadBitmapData("assets/vertical_right.png", CreateBitmap(gameState));
    LoadBitmapData("assets/corner_bottomleft.png", CreateBitmap(gameState));
    LoadBitmapData("assets/horizontal_down.png", CreateBitmap(gameState));
    LoadBitmapData("assets/corner_bottomright.png", CreateBitmap(gameState));
    LoadBitmapData("assets/horizontal_up.png", CreateBitmap(gameState));
}

void UpdateGame(GameState* gameState, real32 dt)
{
    // NOTE(Charly): Toggle game mode on presses
    if (gameState->keyStates[Key_F1].stateChange && gameState->keyStates[Key_F1].clicked)
    {
        gameState->mode = gameState->mode == GameMode_Game ? GameMode_Editor : GameMode_Game;
    }

    local_persist bool32 slowDownTime = false;
    if (InputUpFront(&gameState->keyStates[Key_T]))
    {
        slowDownTime ^= true;
    }

    if (slowDownTime) dt *= 0.1f;

	switch (gameState->mode)
	{
		case GameMode_Game:
		{
			UpdateGameLogic(gameState, dt);
			UpdateWorld(gameState, dt);
		} break;

		case GameMode_Editor:
		{
			UpdateEditor(gameState);
		} break;
		
		default:
		{
			Assert(!"Wrong code path");
		}
	}
}

// TODO(Charly): Move this in renderer ?
void RenderGame(GameState* gameState, real32 dt)
{
	switch (gameState->mode)
	{
		case GameMode_Game:
		{
			glViewport(0, 0, gameState->viewportSize.x, gameState->viewportSize.y);

			glClearColor(0.3f, 0.8f, 0.7f, 0.f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			for (uint32 elementIdx = 0; elementIdx < gameState->nbEntities; ++elementIdx)
			{
				Entity* entity = &gameState->entities[elementIdx];
				if (EntityHasComponent(entity, ComponentFlag_Renderable))
				{
					RenderingPattern* pattern = entity->pattern;
					Vec2 pos(entity->p);

					Transform transform;
					transform.offset = Vec2(0);
					transform.position = pos;
					transform.scale = Vec2(1);
					transform.proj = gameState->projMatrix;
					transform.world = gameState->worldMatrix;

					// TODO(Thomas): Handle drawing size with a drawing size
					if (EntityHasComponent(entity, ComponentFlag_Collidable))
					{
						transform.scale = entity->shape->size;
						transform.offset = entity->shape->offset;
					}

					if (entity->entityType == EntityType_Player)
					{
						if (entity->orientation < 0.f)
						{
							transform.scale.x *= -1.f;
						}
					}

					RenderPattern(pattern, &transform, entity->shape->size);
				}
			}

			RenderHUD(gameState);
		} break;

		case GameMode_Editor:
		{
			RenderEditor(gameState);
		} break;

		default:
		{
			Assert(!"Wrong code path");
		}
	}
}

void RenderHUD(GameState* gameState)
{
	real32 ratio = gameState->viewportSize.x / gameState->viewportSize.y;

	Transform transform;
	transform.offset = Vec2(0);
	transform.proj = Identity();
	transform.world = Identity();

	Vec2 onScreenPos = Vec2(-1.f + 0.08f, 0.92f);
	for (uint32 i = 0; i < gameState->nbPlayers; ++i)
	{
		Entity* player = gameState->players[i];

		transform.scale = Vec2(0.125f, 0.125f*ratio);

		// Avatar
		transform.position = onScreenPos + Times(transform.scale, Vec2(0.5f, -0.5f));
		RenderBitmap(player->avatar, &transform);

		// Health
		transform.scale = Vec2(0.05f, 0.05f*ratio);
		Vec2 healthPos = onScreenPos + Vec2(0.15f, 0.f) + Times(transform.scale, Vec2(0.5f, -0.5f));
		for (uint32 hp = 0; hp < player->max_health; hp+=2)
		{
			transform.position = healthPos;
			if (hp < player->health)
			{
				if (hp + 1 < player->health)
				{
					RenderBitmap(&gameState->hudHealth[0], &transform);
				}
				else
				{
					RenderBitmap(&gameState->hudHealth[1], &transform);
				}
			}
			else
			{
				RenderBitmap(&gameState->hudHealth[2], &transform);
			}

			healthPos.x += 0.051f;
		}

		// Mana
		Vec2 manaPos = onScreenPos + Vec2(0.15f, -0.075f*ratio) + Times(transform.scale, Vec2(0.5f, -0.5f));
		for (uint32 mp = 0; mp < player->max_mana; ++mp)
		{
			transform.position = manaPos;
			if (mp < player->mana)
			{
				RenderBitmap(&gameState->hudMana[0], &transform);
			}
			else
			{
				RenderBitmap(&gameState->hudMana[1], &transform);
			}
			manaPos.x += 0.051f;
		}

		onScreenPos.x += 0.48f;
	}
}

void LoadBitmapData(const char* filename, Bitmap* bitmap)
{
	// NOTE(Charly): Get images bottom-up
	stbi_set_flip_vertically_on_load(true);
	int n;
	bitmap->data = stbi_load(filename, &bitmap->width, &bitmap->height, &n, 4);
	Assert(bitmap->data);
}

void ReleaseBitmapData(Bitmap* bitmap)
{
	ReleaseBitmap(bitmap);
	stbi_image_free(bitmap->data);
}

Entity* CreateEntity(GameState* gameState, EntityType type, Vec2 p, Vec2 dp, Vec2 ddp)
{
	EntityID id = gameState->nbEntities++;
	Assert(id < WORLD_SIZE);

	Entity* result = &gameState->entities[id];
	*result = {0};

	result->id = id;

	result->p = p;
	result->dp = dp;
	result->ddp = ddp;

	result->entityType = type;

	return result;
}

Bitmap* CreateBitmap(GameState* gameState)
{
	ComponentID id = gameState->nbBitmaps++;
	Assert(id < WORLD_SIZE);
	Bitmap* result = &gameState->bitmaps[id];

	return result;
}

Vec2 ViewportToWorld(GameState* state, Vec2 in)
{
	// [0, viewport] -> [0, 1], origin top left
	Vec2 result = in / state->viewportSize;
	// [0, 1] -> [0, 1] origin bot left
	result.y = 1 - result.y;
	// [0, 1] -> [-0.5, 0.5]
	result = result - Vec2(0.5);
	// [-0.5, 0.5] -> [-world / 2, world / 2]
	result = result * state->worldSize;

	return result;
}
