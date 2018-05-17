#include "Grid.h"

Grid::Grid(int width, int height, int cellsize) : 
	m_width(width),
	m_height(height),
	m_cellsize(cellsize) {

	m_numXCells = ceil((float)m_width / m_cellsize);
	m_numYCells = ceil((float)m_height / m_cellsize);

	m_cells.resize(m_numYCells * m_numXCells);
}

Grid::~Grid() {

}


Cell& Grid::getCell(int x, int y) {
	if( x < 0)  x = 0;
	if (x >= m_numXCells) x = m_numXCells -1;
	if (y < 0)  y = 0;
	if (y >= m_numYCells) y = m_numYCells - 1;

	return m_cells[y * m_numXCells + m_numXCells];
}