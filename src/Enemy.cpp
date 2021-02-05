#include "Enemy.h"

Enemy::Enemy(){

}

Enemy::Enemy(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
	type = Chaser;

	prevPlayerTilePos.x = -1;
	prevPlayerTilePos.y = -1;

	nextTilePos.x       = -1.0f;
	nextTilePos.y       = -1.0f;

	stuck               = false;
	on_action           = true;
	m_actionTicks       = 0;
}

Enemy::Enemy(uint textureBufferID, Vector2<float> position, EnemyType type):
							Sprite(textureBufferID, position){
	this->type          = type;

	prevPlayerTilePos.x = -1;
	prevPlayerTilePos.y = -1;

	nextTilePos.x       = -1.0f;
	nextTilePos.y       = -1.0f;

	stuck               = false;
	on_action           = true;
	m_actionTicks       = 0;
}

Enemy::~Enemy(){
}

void Enemy::setArea(std::unique_ptr<Area> area){
	this->area = std::move(area);
}

Vector2<float> Enemy::getPosition(){
	return this->position;
}

void Enemy::setHealth(int new_health){
	this->health = new_health;
}

int Enemy::getHealth(){
	return this->health;
}

void Enemy::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

uint Enemy::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

EnemyType Enemy::getType(){
	return this->type;
}

void Enemy::setType(EnemyType targetType){
	this->type = targetType;
}

bool Enemy::onAction(){
	return this->on_action;
}

void Enemy::resetAction(){
	this->on_action = true;
	this->m_actionTicks = 0;
}

void Enemy::stopAction(){
	this->on_action = false;
}

std::unique_ptr<Enemy> Enemy::Copy(){
    std::unique_ptr<Enemy> copy = std::make_unique<Enemy>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());
	
	copy->position  = this->position;
    copy->velocity  = this->velocity;

	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->area->Copy());
    return copy;
}

void Enemy::update(Vector2<float> playerPos, shared_point_matrix grid){
	switch (this->type)
	{
		case Chaser:
		{	
			// Chase and Bite
			chaserUpdate(playerPos, grid);
				
		}
		break;

		case Sniper:
		{
			// Shoot twice when it is in a valid range 
			sniperUpdate(playerPos, grid);
		}
		break;

		case SuicideBomber:
		{
			// Pick a random spot and explode if player is in range
			suicideBomberUpdate(playerPos, grid);
			std::cout << "Boom" << std::endl;
			
		}
		break;

		case AI:
		{
			// Duplicate of player but controlled by an algorithm
			aiUpdate(playerPos, grid);
			std::cout << "AI My Friend! Artificial Intelligence" << std::endl;
			
		}
		break;
	}
}

void Enemy::chaserUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	Vector2<int> player_tile_pos = maze->pos2MtrCoord(playerPos);
	Vector2<int> current_tile = maze->pos2MtrCoord(position);

	Vector2<float> current_tile_pos(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE); 

	bool player_pos_changed = player_tile_pos != prevPlayerTilePos;
	bool moving_to_the_next_cell = (position - nextTilePos).magnitute() >= 1.0f;
	bool chasing_on = nextTilePos.x >= 0.0f;

	if((player_pos_changed && !moving_to_the_next_cell) || !chasing_on || stuck){
		// Get the route to player
		std::vector<Vector2<int>> route = utility::findRoute(maze->matrix, grid, current_tile, player_tile_pos);

		// Store new route to player
		prevRoute = route;
		prevPlayerTilePos = player_tile_pos;

		// Move to next tile in the route
		Vector2<int> nextTile = prevRoute.back();

		// Store next and prev tile positions
		nextTilePos = Vector2<float>(nextTile.y * SQUARE_SIZE, WIDTH - nextTile.x * SQUARE_SIZE);
		prevTilePos = Vector2<float>(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE);

		// Directonaly movement to next tile
		Vector2<float> dirToNext = (nextTilePos - current_tile_pos).normalize();
		setVelocity(dirToNext * ENEMY_SPEED);

		stuck = false;
	}
	else if(!moving_to_the_next_cell && chasing_on){
		// Remove the previous tile
		prevRoute.pop_back();
		
		// Move to next tile in the route
		Vector2<int> nextTile = prevRoute.back();

		// Store next and prev tile positions
		nextTilePos = Vector2<float>(nextTile.y * SQUARE_SIZE, WIDTH - nextTile.x * SQUARE_SIZE);
		prevTilePos = Vector2<float>(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE);

		// Directonaly movement to next tile
		Vector2<float> dirToNext = (nextTilePos - current_tile_pos).normalize();
		setVelocity(dirToNext * ENEMY_SPEED);
	}

	this->move();
}

void Enemy::move(){
	// Directional Boundary Check Otherwise Object can pass through obstacles
	Direction horizontalDirection = (this->getVelocity().y > 0.0f) ? Up : Down;
	Direction verticalDirection = (this->getVelocity().x > 0.0f) ? Right : Left;

	if(!checkDirectionCollision(horizontalDirection) || !checkDirectionCollision(verticalDirection)){
		this->position += this->getVelocity();
	}
	else{
		// Move to closest tile 
		this->position = Vector2<float>(std::roundf(this->position.x / SQUARE_SIZE) * SQUARE_SIZE, 
										std::roundf(this->position.y / SQUARE_SIZE) * SQUARE_SIZE);
		stuck = true;
	}
}

void Enemy::sniperUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
	float distance = utility::dist_euc<float>(position, playerPos);

	if(distance < SNIPER_SIGHT_DISTANCE){
		// Ray cast and check the line is free
		Vector2<int> playerTilePos = maze->pos2MtrCoord(playerPos);
		Vector2<int> currentTilePos = maze->pos2MtrCoord(position);

		bool isSightOpen = utility::rayCastObstacleCheck(maze->matrix, currentTilePos, playerTilePos);
		
		if(isSightOpen){
			// Shoot
			on_action &= true;
		}
	}

	// Action Status Check
	if(m_actionTicks >= ENEMY_FIRE_TICKS)
		// It can attack, but reset ticks
		resetAction();
	else
		// Reloading ticks
		++m_actionTicks;
}

void Enemy::suicideBomberUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
	// Find the closest teleportation point and settle at the other side (REAL MOFO)
}

void Enemy::aiUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
}