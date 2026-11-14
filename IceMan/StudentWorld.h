#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "GameConstants.h"
#include "Actor.h"

#include <ctime>
#include <string>
#include <vector>
#include <algorithm>

typedef std::vector<std::vector<Ice*>> IceContainer;
typedef std::vector<Squirt*> SquirtContainer;
typedef std::vector<Water*> WaterContainer;
typedef std::vector<Boulder*> BoulderContainer;
typedef std::vector<Protester*> ProtesterContainer;
typedef std::vector<Golds*> GoldContainer;
typedef std::vector<BarrelOil*> OilContainer;
typedef std::vector<Sonar*> SonarContainer;
typedef std::vector<Protester*> ProtesterContainer;
typedef Actor::Direction Direction;

const static int PLAYER_SIZE = 4;
const static int Ice_START_X = 30;
const static int Ice_END_X = 33;
const static int Ice_START_Y = 4;
const static int Ice_END_Y = 59;
const static int Ice_WIDTH = 63;
const static int Ice_HEIGHT = 59;


class StudentWorld : public GameWorld
{
public:

	StudentWorld(std::string assetDir)
		: GameWorld(assetDir) {}
	virtual ~StudentWorld() {};

	virtual int init();
	virtual int move();
	virtual void cleanUp();


	//	ACTION FUNCTIONS //

	void	move_Water();
	void	move_Boulders();
	void	move_Sonar();
	void	move_Oils();
	void	move_Protesters();

	void	dig_Ice();


	// GAME LOGIC FUNCTIONS //


	template <class iterator_type>
	bool	isWithinRange_sameObjs(int col, int row, iterator_type it, iterator_type end) const;
	bool	isWithinRange(int x_cord, int y_cord, Actor* actor) const;
	bool	isWithinRange(int col, int row) const;		//	CHECK IF Ice IS WITHIN RANGE
	bool	isWithinRange_Boulder(int col, int row) const;

	bool	isPlayerCloseBy(int col, int row) const;
	bool	isOpenSpot(int col, int row) const;
	bool	isIceIndex(int col, int row) const;
	bool	isIceBelow(int col, int row) const;

	void	activateSonars();



	Protester* hitProtester(int x_cord, int y_cord) const;

	//	GAME SCREEN	//

	void setGameText();

	// GETTER FUNCTIONS //

	IceContainer& getIce() { return m_IceList; };
	ProtesterContainer& getProtesters() { return m_protesters; };
	BoulderContainer& getBoulders() { return m_boulders; };
	OilContainer& getOils() { return m_oils; };
	WaterContainer& getWaters() { return m_waters; };

private:

	// PROTESTERS // 

	int	    numOfProtesters;
	int	    maxNumOfProtesters;
	int	    timeSinceProtesterAdded;

	// INITIALIZE //

	void	init_IceMan();
	void	init_Ices();
	void	init_Boulders();
	void	init_Golds();
	void	init_Oils();
	void	init_Protesters();


	// HELPER FUNCTIONS //
	bool    isValidSpotForGoodies(int col, int row) const;
	double  calculateRadius(int x_1, int y_1, int x_2, int y_2) const;
	bool    isWithinIce(int col, int row) const;

	IceContainer		m_IceList;
	IceMan* m_player;
	BoulderContainer	m_boulders;
	OilContainer		m_oils;
	WaterContainer		m_waters;
	SonarContainer		m_sonars;
	ProtesterContainer  m_protesters;
};

template <class iterator_type>
bool StudentWorld::isWithinRange_sameObjs(int col, int row, iterator_type it, iterator_type end) const
{
	while (it != end)
	{
		if ((*it)->getX() == col && (*it)->getY() == row) {
			it++;
			continue;
		}

		if (calculateRadius(col, row, (*it)->getX(), (*it)->getY()) <= 3)
			return true;

		it++;
	}

	return false;

}

#endif // STUDENTWORLD_H_
