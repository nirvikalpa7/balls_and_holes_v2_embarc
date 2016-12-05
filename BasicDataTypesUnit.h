
#ifndef BasicDataTypeUnitH
#define BasicDataTypeUnitH

//------------------------------------------------------------------------------

class CBall {
public:

	CBall() { x = y = number = 0; }
	int x, y;
	unsigned int number;

	inline __fastcall void SetCoord(const unsigned int nX, const unsigned int nY) { x = nX; y = nY; }

	bool operator==(const CBall & Ball)
	{
		if (x == Ball.x && y == Ball.y)
		{
			return true;
		}

		return false;
	}

	CBall operator=(const CBall & Ball)
	{
		x = Ball.x;
		y = Ball.y;
		number = Ball.number;
		return *this;
	}
};

//------------------------------------------------------------------------------

class CBallCoord {
public:
	CBallCoord() { bExist = false; Ball.x = Ball.y = Ball.number = 0; }

	CBallCoord operator=(const CBallCoord & NewBall)
	{
		bExist = NewBall.bExist; // ball is not deleted
		Ball = NewBall.Ball;
		return *this;
	}

	bool bExist; // ball is not deleted
	CBall Ball;  // x, y, number
};

//------------------------------------------------------------------------------

class CBorder {
public:
	CBall p1, p2;

	CBorder() { p1.x = p1.y = p2.x = p2.y = 0; }
	CBorder(const CBall & NewP1, const CBall & NewP2) { p1 = NewP1; p2 = NewP2; }

	bool operator==(const CBorder & Border)
	{
		if (p1 == Border.p1 && p2 == Border.p2)
		{
			return true;
		}

		return false;
	}

	CBorder operator=(const CBorder & Border)
	{
		p1.SetCoord(Border.p1.x, Border.p1.y);
		p2.SetCoord(Border.p2.x, Border.p2.y);
		return *this;
	}
};

//------------------------------------------------------------------------------
#endif
