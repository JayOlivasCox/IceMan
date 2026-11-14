#include "Actor.h"
#include "StudentWorld.h"

#include <algorithm>
#include <cstdlib>
#include <queue>
#include <utility>

// --------------------------------------------------------- PROTESTER --------------------------------------------------------- // 

Protester::Protester(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth) :
	Actor(imageID, startX, startY, dir, size, depth)
{
}

// Breadth-first search from (startX, startY) to the exit at (60, 60).
// Writes the moves as a string of 'E', 'W', 'N', 'S' into pathToExit
// (left empty if there is no path right now, so it will retry next tick).
// The old version was a backtracking depth-first search that tried every
// possible path, which could take exponential time and freeze the game.
void Protester::FindExitPath(int startX, int startY, std::string& pathToExit, IceMan* player)
{
	const int EXIT_X = 60, EXIT_Y = 60, SIZE = 61;
	StudentWorld* Game = player->getWorld();

	pathToExit.clear();
	if (startX < 0 || startX >= SIZE || startY < 0 || startY >= SIZE)
		return;

	int cameFrom[SIZE][SIZE];						//	direction index used to reach a cell, -1 = not visited
	for (int x = 0; x < SIZE; x++)
		for (int y = 0; y < SIZE; y++)
			cameFrom[x][y] = -1;

	std::queue<std::pair<int, int>> q;
	q.push(std::make_pair(startX, startY));
	cameFrom[startX][startY] = 4;					//	start marker

	while (!q.empty() && cameFrom[EXIT_X][EXIT_Y] == -1)
	{
		std::pair<int, int> cur = q.front();
		q.pop();

		for (int i = 0; i < 4; i++)
		{
			int next_x = cur.first + direction[i][0];
			int next_y = cur.second + direction[i][1];

			if (next_x < 0 || next_x >= SIZE || next_y < 0 || next_y >= SIZE || cameFrom[next_x][next_y] != -1)
				continue;

			if (Game->isWithinRange_Boulder(next_x, next_y))
				continue;

			//	check the edge of the 4x4 sprite that moves into new squares
			bool blocked = false;
			for (int j = 0; j < 4 && !blocked; j++)
			{
				int col, row;
				switch (i)
				{
				case 0:  col = next_x + 3; row = next_y + j; break;		//	E
				case 1:  col = next_x;     row = next_y + j; break;		//	W
				case 2:  col = next_x + j; row = next_y + 3; break;		//	N
				default: col = next_x + j; row = next_y;     break;		//	S
				}
				if (Game->isIceIndex(col, row) && Game->getIce()[col][row])
					blocked = true;
			}
			if (blocked)
				continue;

			cameFrom[next_x][next_y] = i;
			q.push(std::make_pair(next_x, next_y));
		}
	}

	if (cameFrom[EXIT_X][EXIT_Y] == -1)
		return;

	//	walk back from the exit to the start, then reverse
	std::string reversed;
	int x = EXIT_X, y = EXIT_Y;
	while (x != startX || y != startY)
	{
		int i = cameFrom[x][y];
		reversed += dir_path[i];
		x -= direction[i][0];
		y -= direction[i][1];
	}
	pathToExit.assign(reversed.rbegin(), reversed.rend());
}


void Protester::checkPerpenPath(int col, int row, Direction currDir, IceMan* player, std::vector<Direction>& dir) const
{
	StudentWorld* Game = player->getWorld();

	switch (currDir)
	{
		bool flag;

	case right: case left:
	{
		flag = true;

		for (int i = 0; i < 4; i++)
			if (!Game->isOpenSpot(col + i, row - 1))
				flag = false;

		if (flag) dir.push_back(down);

		flag = true;

		for (int i = 0; i < 4; i++)
			if (!Game->isOpenSpot(col + i, row + 4))
				flag = false;

		if (flag) dir.push_back(up);
		break;
	}
	case down: case up:
	{
		flag = true;

		for (int i = 0; i < 4; i++)
			if (!Game->isOpenSpot(col - 1, row + i))
				flag = false;

		if (flag) dir.push_back(left);

		flag = true;

		for (int i = 0; i < 4; i++)
			if (!Game->isOpenSpot(col + 4, row + i))
				flag = false;

		if (flag) dir.push_back(right);

	}
	default:
		break;
	}

}


void Protester::doSomething(IceMan* player)
{

	StudentWorld* Game = player->getWorld();


	if (update(player)) return;
	else
	{
		decreShoutingTime();
		decreTicksSincePerpenMoves();
	}

	if (Game->isPlayerCloseBy(getX(), getY()) && !leaveOilFieldState() && getShoutingInterval() == 0)
	{
		Game->playSound(SOUND_PROTESTER_YELL);
		player->decreHitpts(20);
		setShoutingInterval(15);
	}

	if (canSeePlayer(getX(), getY(), player))			//	If a player is in horizontal or vertical sight without objects in between
	{
		setNumSquaresToMoveInCurrentDirection(0);
		int currLevel = player->getWorld()->getLevel();
		int N = std::max(0, static_cast<int>(3 - currLevel / 4));
		setTicksToWaitBetweenMoves(N);
		return;
	}

	if (getTicksSincePerpenMoves() <= 0)
		GoToPerpenPath(player);

	if (getNumSquaresToMoveInCurrentDirection() > 0)
		GoToCurrDirection(player);
	else
		pickNewDirection(player);


	int currLevel = player->getWorld()->getLevel();
	int N = std::max(0, static_cast<int>(3 - currLevel / 4));
	setTicksToWaitBetweenMoves(N);
}


bool Protester::update(IceMan* player)
{
	if (getX() == 60 && getY() == 60 && leaveOilFieldState()) {
		setDead();
		return true;
	}

	if (getTicksToWaitBetweenMoves() > 0)
	{
		decreTicksToWaitBetweenMoves();
		return true;
	}

	if (leaveOilFieldState())
	{
		GoToExit(player);
		return true;
	}

	return false;
}

void Protester::GoToPerpenPath(IceMan* player)
{
	std::vector<Direction> perpenDir;

	checkPerpenPath(getX(), getY(), getDirection(), player, perpenDir);

	if (perpenDir.size() == 1) {
		setDirection(perpenDir[0]);
		setTicksSincePerpenMoves(200);

		int N = rand() % 53 + 8;
		setNumSquaresToMoveInCurrentDirection(N);
	}

	else if (perpenDir.size() > 1)
	{
		int i = rand() % perpenDir.size();

		setDirection(perpenDir[i]);
		setTicksSincePerpenMoves(200);

		int N = rand() % 53 + 8;
		setNumSquaresToMoveInCurrentDirection(N);
	}
}

void Protester::GoToCurrDirection(IceMan* player)
{
	StudentWorld* Game = player->getWorld();
	Direction currDir = getDirection();

	switch (currDir)
	{
	case up:
		if (Game->isOpenSpot(getX(), getY() + 4))
			moveTo(getX(), getY() + 1);
		else setNumSquaresToMoveInCurrentDirection(0);
		break;
	case down:
		if (Game->isOpenSpot(getX(), getY() - 1))
			moveTo(getX(), getY() - 1);
		else setNumSquaresToMoveInCurrentDirection(0);
		break;
	case right:
		if (Game->isOpenSpot(getX() + 4, getY()))
			moveTo(getX() + 1, getY());
		else setNumSquaresToMoveInCurrentDirection(0);
		break;
	case left:
		if (Game->isOpenSpot(getX() - 1, getY()))
			moveTo(getX() - 1, getY());
		else setNumSquaresToMoveInCurrentDirection(0);
		break;
	default:
		break;
	}

	decreNumSquaresToMoveInCurrentDirection();
}

void Protester::pickNewDirection(IceMan* player)
{
	StudentWorld* Game = player->getWorld();
	int N;
	Direction newDir;
	bool rightDir;

	do
	{
		rightDir = true;

		newDir = static_cast<Direction>(rand() % 4 + 1);					// 1 to 4
		N = rand() % 53 + 8;												//	8 <= numSquaresToMoveInCurrentDirection <= 60 

		setNumSquaresToMoveInCurrentDirection(N);

		switch (newDir)
		{

		case right:
			if (Game->isOpenSpot(getX() + 1, getY()))
				setDirection(right);
			else rightDir = false;
			break;

		case left:
			if (Game->isOpenSpot(getX() - 1, getY()))
				setDirection(left);
			else rightDir = false;
			break;

		case up:
			if (Game->isOpenSpot(getX(), getY() + 1))
				setDirection(up);
			else rightDir = false;
			break;

		case down:
			if (Game->isOpenSpot(getX(), getY() - 1))
				setDirection(down);
			else rightDir = false;
			break;
		default:
			break;
		}
	} while (!rightDir);

}

void Protester::GoToExit(IceMan* player)
{
	if (getPathToExit() == "") {
		std::string exitPath;
		FindExitPath(getX(), getY(), exitPath, player);
		setPathToExit(exitPath);
	}
	else
	{
		char currDir = getDirection();
		char Dir = getPathToExit()[0];
		getPathToExit().erase(0, 1);

		switch (Dir)
		{
		case 'W':
			if (currDir != left)
				setDirection(left);
			moveTo(getX() - 1, getY());
			break;
		case 'E':
			if (currDir != right)
				setDirection(right);
			moveTo(getX() + 1, getY());
			break;
		case 'N':
			if (currDir != up)
				setDirection(up);
			moveTo(getX(), getY() + 1);
			break;
		case 'S':
			if (currDir != down)
				setDirection(down);
			moveTo(getX(), getY() - 1);
			break;
		}
	}

	int currLevel = player->getWorld()->getLevel();
	int N = std::max(0, static_cast<int>(3 - currLevel / 4));
	setTicksToWaitBetweenMoves(N);
}


bool Protester::checkForObstacles(int col, int row, Direction dir, IceMan* player) const
{
	StudentWorld* Game = player->getWorld();

	//	Used to dereference Boulders.begin() without checking, which crashed
	//	once every boulder had fallen (empty vector), and only ever checked the first boulder.
	if (Game->isWithinRange_Boulder(col, row))
		return true;

	switch (dir)
	{
	case right: case left:
		for (int j = 0; j < 4; j++)
			if (Game->isIceIndex(col, row + j) && Game->getIce()[col][row + j])
				return true;
		break;

	case up: case down:
		for (int j = 0; j < 4; j++)
			if (Game->isIceIndex(col + j, row) && Game->getIce()[col + j][row])
				return true;
		break;

	default:
		break;
	}

	return false;
}

bool Protester::canSeePlayer(int col, int row, IceMan* player)
{
	StudentWorld* Game = player->getWorld();
	int playerX = player->getX();
	int playerY = player->getY();
	int i = 0;

	if (col == playerX && row != playerY)								//	if a player is in vertical sight
	{
		int distance = row - playerY;
		if (distance < 0)												//	if a player is above protester
		{
			while (distance + i != 0)
			{
				if (checkForObstacles(col, row + i, up, player))
					return false;
				i++;
			}

			setDirection(up);
			moveTo(getX(), getY() + 1);

			return true;
		}
		else																//	if a player is below protester
		{
			while (distance - i != 0)
			{
				if (checkForObstacles(col, row - i, down, player))
					return false;
				i++;
			}

			setDirection(down);
			moveTo(getX(), getY() - 1);

			return true;
		}
	}

	else if (row == playerY && col != playerX)							// if a player is in horizontal sight
	{
		int distance = col - playerX;

		if (distance < 0)												//	if a player is more right 
		{
			while (distance + i != 0)
			{
				if (checkForObstacles(col + i, row, right, player))
					return false;
				i++;
			}

			setDirection(right);
			moveTo(getX() + 1, getY());

			return true;

		}
		else																//	if a player is more left
		{
			while (distance - i != 0)
			{
				if (checkForObstacles(col - i, row, left, player))
					return false;
				i++;
			}

			setDirection(left);
			moveTo(getX() - 1, getY());

			return true;

		}
	}

	return false;
}

Regular_Protester::Regular_Protester(IceMan* player) :
	Protester(IID_PROTESTER, 60, 60, left, 1.0, 0)
{
	int currLevel = player->getWorld()->getLevel();
	int T = std::max(0, static_cast<int>(3 - currLevel / 4));
	int N = rand() % 53 + 8;

	setTicksToWaitBetweenMoves(T);
	setVisible(true);
	setHitPoints(5);
	setTicksToWaitBetweenTurns(200);
	setNumSquaresToMoveInCurrentDirection(N);
	setTicksSincePerpenMoves(0);
}

Hardcore_Protester::Hardcore_Protester(IceMan* player) :
	Protester(IID_HARD_CORE_PROTESTER, 60, 60, left, 1.0, 0)
{
	int currLevel = player->getWorld()->getLevel();
	int T = std::max(0, static_cast<int>(3 - currLevel / 4));
	int N = rand() % 53 + 8;

	totalDistanceFromPlayer = 16 + currLevel * 2;
	setTicksToWaitBetweenMoves(T);
	setVisible(true);
	setHitPoints(20);
	setTicksToWaitBetweenTurns(200);
	setNumSquaresToMoveInCurrentDirection(N);
	setTicksSincePerpenMoves(0);
}

// --------------------------------------------------------- BARREL OIL --------------------------------------------------------- // 

BarrelOil::BarrelOil(int startX, int startY) : Goodies(IID_BARREL, startX, startY, right, 1.0, 2)
{
	setVisible(false);
}

void BarrelOil::doSomething(IceMan* player)
{
	StudentWorld* Game = player->getWorld();

	if (Game->isPlayerCloseBy(getX(), getY()))
		setVisible(true);

	if (Game->isWithinRange(getX(), getY(), player))
	{
		Game->playSound(SOUND_FOUND_OIL);
		player->addOil();
		player->addScore(1000);
		Game->setGameText();

		setDead();
	}
}

// --------------------------------------------------------- GOLDS --------------------------------------------------------- // 


Golds::Golds(int startX, int startY, bool setting, Actors owner) :
	Goodies(IID_GOLD, startX, startY, right, 1.0, 2)
{
	setOwnership(owner);
	setVisible(setting);
	setTemporState(100);

}

void Golds::doSomething(IceMan* player)
{
	StudentWorld* Game = player->getWorld();

	if (getTemporState() == TIME_UP) {
		setDead();
		return;
	}

	else if (getOwnership() == PLAYER && !Game->isWithinRange(getX(), getY()))
	{																		//	If gold nugget is in Iceless spot, set the timer
		decreTemporState();
	}

	if (Game->isPlayerCloseBy(getX(), getY()))
	{
		setVisible(true);
	}

	if (Game->isWithinRange(getX(), getY(), player) && getOwnership() == Ice)
	{
		Game->playSound(SOUND_GOT_GOODIE);
		player->addGold();
		player->addScore(10);
		Game->setGameText();

		setDead();
	}

	Protester* bribedProtester = Game->hitProtester(getX(), getY());

	if (bribedProtester && !bribedProtester->leaveOilFieldState() && getOwnership() == PLAYER)
	{
		Game->playSound(SOUND_PROTESTER_FOUND_GOLD);
		if (dynamic_cast<Hardcore_Protester*>(bribedProtester)) {

			player->addScore(50);
			int currLevel = player->getWorld()->getLevel();
			int ticks_to_stare = std::max(50, 100 - currLevel * 10);
			bribedProtester->setTicksToWaitBetweenMoves(ticks_to_stare);
			Game->setGameText();
			setDead();
			return;
			;
		}
		else
			player->addScore(25);

		bribedProtester->setToLeaveOilFieldState();
		Game->setGameText();

		setDead();
	}

}



// --------------------------------------------------------- SONAR --------------------------------------------------------- // 

Sonar::Sonar(int currLevel) : Goodies(IID_SONAR, 0, 60, right, 1.0, 2)
{
	setVisible(true);
	int T = std::max(100, 300 - 10 * currLevel);
	setTemporState(T);
}

void Sonar::doSomething(IceMan* player)
{
	if (getTemporState() == TIME_UP) {
		setDead();
		return;
	}
	else decreTemporState();

	StudentWorld* Game = player->getWorld();

	if (Game->isWithinRange(getX(), getY(), player))
	{
		Game->playSound(SOUND_GOT_GOODIE);
		player->addSonar();
		player->addScore(75);
		Game->setGameText();

		setDead();
	}
}


// --------------------------------------------------------- BOULDER --------------------------------------------------------- // 

Boulder::Boulder(int startX, int startY) :
	Actor(IID_BOULDER, startX, startY, right, 1.0, 1)
{
	setVisible(true);
	setTemporState(30);
}

void Boulder::doSomething(IceMan* player)
{
	StudentWorld* Game = player->getWorld();

	if (!Game->isIceBelow(getX(), getY())) 						//	if there is no Ice below, fall
		fallingState = true;


	if (fallingState && getTemporState() == TIME_UP)
	{

		if (getSwitch() == SOUND_OFF)
		{
			Game->playSound(SOUND_FALLING_ROCK);
			Turn_On_Sound();
		}

		Protester* damagedProtester = Game->hitProtester(getX(), getY());

		if (damagedProtester && !damagedProtester->leaveOilFieldState())
		{
			Game->playSound(SOUND_PROTESTER_GIVE_UP);
			player->addScore(500);
			damagedProtester->setToLeaveOilFieldState();
		}

		if (Game->isWithinRange(getX(), getY(), player))				// Collision with a player
		{
			player->setDead();
			setDead();
		}
		else if (Game->isWithinRange_sameObjs(getX(), getY(), Game->getBoulders().begin(), Game->getBoulders().end()))		// Collision with another boulder	
		{
			setDead();
		}
		else if (Game->isWithinRange(getX(), getY()))			//	Collision with Ice
		{
			setDead();
		}
		else if (getY() <= 0)									//	Hit the bottom of the oil field
		{
			setDead();
		}

		else
		{
			moveTo(getX(), getY() - 1);
		}

	}
}


// --------------------------------------------------------- GAME OBJECT --------------------------------------------------------- // 


Actor::Actor(int imageID, int startX, int startY, Direction dir, double size, unsigned int depth)
	: GraphObject(imageID, startX, startY, dir, size, depth)
{}



void Actor::decreTemporState()
{
	if (temporState == TIME_UP)
		return;

	temporState--;
}


// --------------------------------------------------------- Ice MAN --------------------------------------------------------- // 


IceMan::IceMan(StudentWorld* game_adr) : Actor(IID_PLAYER, 30, 60, right, 1.0, 0)
{
	setVisible(true);
	setWorld(game_adr);
}

void IceMan::addScore(int score)
{
	getWorld()->increaseScore(score);
}

int IceMan::getCurrScore() const
{
	return getWorld()->getScore();
}

void IceMan::move_Squirts()
{

	if (m_squirts.size() == 0)
		return;

	auto it = m_squirts.begin();

	while (it != m_squirts.end())
	{
		(*it)->doSomething(this);

		if (!(*it)->isAlive()) {
			delete* it;
			*it = nullptr;
			it = m_squirts.erase(it);
		}

		else it++;
	}

}

void IceMan::init_Squirts()
{
	Direction dir = getDirection();

	if (numOfWater > 0)
	{

		switch (dir)
		{
		case right:
			if (!getWorld()->isWithinRange(getX() + 3, getY()) && !getWorld()->isWithinRange_Boulder(getX() + 3, getY()))
				m_squirts.push_back(new Squirt(getX() + 3, getY(), dir));
			break;

		case left:
			if (!getWorld()->isWithinRange(getX() - 3, getY()) && !getWorld()->isWithinRange_Boulder(getX() - 3, getY()))
				m_squirts.push_back(new Squirt(getX() - 3, getY(), dir));
			break;

		case up:
			if (!getWorld()->isWithinRange(getX(), getY() + 3) && !getWorld()->isWithinRange_Boulder(getX(), getY() + 3))
				m_squirts.push_back(new Squirt(getX(), getY() + 3, dir));
			break;

		case down:
			if (!getWorld()->isWithinRange(getX(), getY() - 3) && !getWorld()->isWithinRange_Boulder(getX(), getY() - 3))
				m_squirts.push_back(new Squirt(getX(), getY() - 3, dir));
			break;

		default:
			break;
		}

		getWorld()->playSound(SOUND_PLAYER_SQUIRT);
		numOfWater--;

	}
}


// --------------------------------------------------------- GOODIES --------------------------------------------------------- // 

Goodies::Goodies(int imageID, int startX, int startY, Direction dir,
	double size, unsigned int depth) : Actor(imageID, startX, startY, dir, size, depth) {}

void Goodies::setOwnership(Actors actor)
{
	whoPickedUp = actor;
}



// --------------------------------------------------------- WATER --------------------------------------------------------- // 

Water::Water(int col, int row, int currLevel) : Goodies(IID_WATER_POOL, col, row, right, 1.0, 2)
{
	setVisible(true);
	int T = std::max(100, 300 - 10 * currLevel);
	setTemporState(T);
}

void Water::doSomething(IceMan* player)
{
	if (getTemporState() == TIME_UP) {
		setDead();
		return;
	}
	else decreTemporState();

	StudentWorld* Game = player->getWorld();

	if (Game->isWithinRange(getX(), getY(), player))
	{
		Game->playSound(SOUND_GOT_GOODIE);
		player->addWater();
		player->addScore(100);
		Game->setGameText();

		setDead();
	}
}


// --------------------------------------------------------- SQUIRT --------------------------------------------------------- // 

Squirt::Squirt(int col, int row, Direction dir) :
	Actor(IID_WATER_SPURT, col, row, dir, 1.0, 1)
{
	setVisible(true);
}

void Squirt::move(IceMan* player)
{
	StudentWorld* Game = player->getWorld();
	int col = getX();
	int row = getY();

	Direction dir = getDirection();

	switch (dir)
	{
	case right:
		if (Game->isOpenSpot(col + 1, row)) {
			moveTo(col + 1, row);
			distanceTravelled++;
		}
		else setDead();
		break;

	case left:
		if (Game->isOpenSpot(col - 1, row)) {
			moveTo(col - 1, row);
			distanceTravelled++;
		}
		else setDead();
		break;

	case up:
		if (Game->isOpenSpot(col, row + 1)) {
			moveTo(col, row + 1);
			distanceTravelled++;
		}
		else setDead();
		break;

	case down:
		if (Game->isOpenSpot(col, row - 1)) {
			moveTo(col, row - 1);
			distanceTravelled++;
		}
		else setDead();
		break;
	default:
		break;
	}
}

void Squirt::doSomething(IceMan* player)
{
	StudentWorld* Game = player->getWorld();

	if (distanceTravelled == 4)
	{
		setDead();
		return;
	}

	Protester* damagedProtester = Game->hitProtester(getX(), getY());

	if (damagedProtester && !damagedProtester->leaveOilFieldState())		//	If the protester isn't in leave state
	{
		damagedProtester->decreHitPoints(2);

		if (damagedProtester->getHitPoints() <= 0)
		{
			Game->playSound(SOUND_PROTESTER_GIVE_UP);

			if (dynamic_cast<Hardcore_Protester*>(damagedProtester))
				player->addScore(250);
			else
				player->addScore(100);

			damagedProtester->setToLeaveOilFieldState();
			damagedProtester->setTicksToWaitBetweenMoves(0);
			setDead();
			return;
		}

		Game->playSound(SOUND_PROTESTER_ANNOYED);

		int N = std::max(50, static_cast<int>(100 - Game->getLevel() * 10));
		damagedProtester->setTicksToWaitBetweenMoves(N);
		setDead();
		return;
	}

	move(player);
}



// --------------------------------------------------------- Ice MAN --------------------------------------------------------- // 

void IceMan::doSomething()
{
	if (healthpts == 0) {
		setDead();
		return;
	}

	int input;

	if (getWorld()->getKey(input))
	{

		Direction curDir = getDirection();

		switch (input)
		{
		case KEY_PRESS_RIGHT:
			if (curDir == right && getX() < VIEW_WIDTH - 4) {
				if (!getWorld()->isWithinRange_Boulder(getX() + 1, getY()))
					moveTo(getX() + 1, getY());
			}
			else moveTo(getX(), getY());

			setDirection(right);
			break;

		case KEY_PRESS_LEFT:
			if (curDir == left && getX() > 0) {
				if (!getWorld()->isWithinRange_Boulder(getX() - 1, getY()))
					moveTo(getX() - 1, getY());
			}
			else moveTo(getX(), getY());

			setDirection(left);
			break;

		case KEY_PRESS_UP:
			if (curDir == up && getY() < VIEW_HEIGHT - 4) {
				if (!getWorld()->isWithinRange_Boulder(getX(), getY() + 1))
					moveTo(getX(), getY() + 1);
			}
			else moveTo(getX(), getY());

			setDirection(up);
			break;

		case KEY_PRESS_DOWN:
			if (curDir == down && getY() > 0) {
				if (!getWorld()->isWithinRange_Boulder(getX(), getY() - 1))
					moveTo(getX(), getY() - 1);
			}
			else moveTo(getX(), getY());

			setDirection(down);
			break;

		case KEY_PRESS_SPACE:
			init_Squirts();
			break;


		case 'z': case 'Z':
			move_Sonars();
			break;


		case KEY_PRESS_TAB:
			init_Golds();
		}
	}

	getWorld()->dig_Ice();
}

void IceMan::init_Golds()
{
	Direction dir = getDirection();

	if (numOfGolds > 0)
	{
		numOfGolds--;
		m_golds.push_back(new Golds(getX(), getY(), true, PLAYER));
	}
}

void IceMan::addGolds(int col, int row, bool setting, Actors owner)
{
	m_golds.push_back(new Golds(col, row, setting, owner));
}

void IceMan::move_Golds()
{

	GoldContainer::iterator it = m_golds.begin();

	while (it != m_golds.end()) {

		if (!((*it)->isAlive())) {
			delete* it;
			*it = nullptr;
			it = m_golds.erase(it);
		}

		else {
			(*it)->doSomething(this);
			it++;
		}
	}
}


void IceMan::move_Sonars()
{
	if (numOfSonar > 0)
	{
		getWorld()->activateSonars();
		numOfSonar--;
		getWorld()->playSound(SOUND_SONAR);
	}

}
