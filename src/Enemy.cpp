#include "Enemy.h"

Enemy::Enemy(){

}

Enemy::Enemy(uint textureBufferID, Vector2<float> position):
							Sprite(textureBufferID, position){
	m_Type = Chaser;

	m_PrevPlayerTilePos.x = -1;
	m_PrevPlayerTilePos.y = -1;

	m_NextTilePos.x       = -1.0f;
	m_NextTilePos.y       = -1.0f;

	m_Stuck               = false;
	m_OnAction           = true;
	m_ActionTicks       = 0;
}

Enemy::Enemy(uint textureBufferID, Vector2<float> position, EnemyType type):
							Sprite(textureBufferID, position){
	this->m_Type          = type;

	m_PrevPlayerTilePos.x = -1;
	m_PrevPlayerTilePos.y = -1;

	m_NextTilePos.x       = -1.0f;
	m_NextTilePos.y       = -1.0f;

	m_Stuck               = false;
	m_OnAction           = true;
	m_ActionTicks       = 0;
}

Enemy::~Enemy(){
}

void Enemy::setArea(std::unique_ptr<Area> area){
	this->m_Area = std::move(area);
}

Vector2<float> Enemy::getPosition(){
	return this->m_Position;
}

void Enemy::setHealth(int new_health){
	this->m_Health = new_health;
}

int Enemy::getHealth(){
	return this->m_Health;
}

void Enemy::setTextureBufferID(uint textureBufferID)
{
   	Sprite::setTextureBufferID(textureBufferID);
}

uint Enemy::TextureBufferID(){
    return Sprite::getTextureBufferID();
}

EnemyType Enemy::getType(){
	return this->m_Type;
}

void Enemy::setType(EnemyType targetType){
	this->m_Type = targetType;
}

bool Enemy::onAction(){
	return this->m_OnAction;
}

void Enemy::resetAction(){
	this->m_OnAction = true;
	this->m_ActionTicks = 0;
}

void Enemy::stopAction(){
	this->m_OnAction = false;
}

std::unique_ptr<Enemy> Enemy::Copy(){
    std::unique_ptr<Enemy> copy = std::make_unique<Enemy>();
	copy->setTextureBufferID(Sprite::getTextureBufferID());
	
	copy->m_Position  = this->m_Position;
    copy->m_Velocity  = this->m_Velocity;

	copy->setRotation(Sprite::getRotation());
	copy->setArea(this->m_Area->Copy());
    return copy;
}

void Enemy::update(Vector2<float> playerPos, shared_point_matrix grid){
	switch (this->m_Type)
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
	Vector2<int> player_tile_pos = m_Maze->pos2MtrCoord(playerPos);
	Vector2<int> current_tile = m_Maze->pos2MtrCoord(m_Position);

	Vector2<float> current_tile_pos(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE); 

	bool player_pos_changed = player_tile_pos != m_PrevPlayerTilePos;
	bool moving_to_the_next_cell = (m_Position - m_NextTilePos).magnitute() >= 1.0f;
	bool chasing_on = m_NextTilePos.x >= 0.0f;

	if((player_pos_changed && !moving_to_the_next_cell) || !chasing_on || m_Stuck){
		// Get the route to player
		std::vector<Vector2<int>> route = utility::findRoute(m_Maze->m_Matrix, grid, current_tile, player_tile_pos);

		// Store new route to player
		m_PrevRoute = route;
		m_PrevPlayerTilePos = player_tile_pos;

		// Move to next tile in the route
		Vector2<int> nextTile = m_PrevRoute.back();

		// Store next and prev tile positions
		m_NextTilePos = Vector2<float>(nextTile.y * SQUARE_SIZE, WIDTH - nextTile.x * SQUARE_SIZE);
		m_PrevTilePos = Vector2<float>(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE);

		// Directonaly movement to next tile
		Vector2<float> dirToNext = (m_NextTilePos - current_tile_pos).normalize();
		setVelocity(dirToNext * ENEMY_SPEED);

		m_Stuck = false;
	}
	else if(!moving_to_the_next_cell && chasing_on){
		// Remove the previous tile
		m_PrevRoute.pop_back();
		
		// Move to next tile in the route
		Vector2<int> nextTile = m_PrevRoute.back();

		// Store next and prev tile positions
		m_NextTilePos = Vector2<float>(nextTile.y * SQUARE_SIZE, WIDTH - nextTile.x * SQUARE_SIZE);
		m_PrevTilePos = Vector2<float>(current_tile.y * SQUARE_SIZE, WIDTH - current_tile.x * SQUARE_SIZE);

		// Directonaly movement to next tile
		Vector2<float> dirToNext = (m_NextTilePos - current_tile_pos).normalize();
		setVelocity(dirToNext * ENEMY_SPEED);
	}

	this->move();
}

void Enemy::move(){
	// Directional Boundary Check Otherwise Object can pass through obstacles
	Direction horizontalDirection = (this->getVelocity().y > 0.0f) ? Up : Down;
	Direction verticalDirection = (this->getVelocity().x > 0.0f) ? Right : Left;

	if(!checkDirectionCollision(horizontalDirection) || !checkDirectionCollision(verticalDirection)){
		this->m_Position += this->getVelocity();
	}
	else{
		// Move to closest tile 
		this->m_Position = Vector2<float>(std::roundf(this->m_Position.x / SQUARE_SIZE) * SQUARE_SIZE, 
										std::roundf(this->m_Position.y / SQUARE_SIZE) * SQUARE_SIZE);
		m_Stuck = true;
	}
}

void Enemy::sniperUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
	float distance = utility::dist_euc<float>(m_Position, playerPos);

	// Action Status Check
	if(m_ActionTicks >= ENEMY_FIRE_TICKS)
	{
		// It can attack, but reset ticks
		resetAction();
	}
	else{
		// Reloading ticks
		++m_ActionTicks;
	}

	bool ray_cast_result = true;

	if(distance < SNIPER_SIGHT_DISTANCE){
		// Ray cast and check the line is free
		Vector2<int> playerTilePos = m_Maze->pos2MtrCoord(playerPos);
		Vector2<int> currentTilePos = m_Maze->pos2MtrCoord(m_Position);

		bool isSightOpen = utility::rayCastObstacleCheck(m_Maze->m_Matrix, currentTilePos, playerTilePos);
		
		if(isSightOpen){
			// Shoot
			m_OnAction &= true;
			ray_cast_result = true;
		}
		else{
			m_OnAction &= false;
			ray_cast_result = false;
		}
	}
	else{
		m_OnAction = false;
	}
}

void Enemy::suicideBomberUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
	// Find the closest teleportation point and settle at the other side (REAL MOFO)
}

void Enemy::aiUpdate(Vector2<float> playerPos, shared_point_matrix grid){
	// TODO : Coming Soon
}