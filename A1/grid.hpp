#pragma once

class Grid
{
public:
	Grid( size_t dim );
	~Grid();

	void reset();

	size_t getDim() const;

	int getHeight( int x, int y ) const;
	int getColour( int x, int y ) const;
	int getColourPicker( int x, int y ) const;

	void setHeight( int x, int y, int h );
	void setColour( int x, int y, int c );
	void setColourPicker( int x, int y, int cp );
	void print();
	
private:
	size_t m_dim;
	int *m_heights;
	int *m_cols;
	int *m_colpickers;
};
