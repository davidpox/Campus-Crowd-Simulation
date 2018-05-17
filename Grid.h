#pragma once

#include "crowds.h"
#include <vector>

struct Cell {
	std::vector<Node*> nodes;
}; 

class Grid {
public:
	Grid(int width, int height, int cellsize);
	~Grid();

	Cell& getCell(int x, int y);


private: 
	std::vector<Cell> m_cells;
	int m_cellsize;
	int m_width;
	int m_height;
	int m_numXCells;
	int m_numYCells;
};