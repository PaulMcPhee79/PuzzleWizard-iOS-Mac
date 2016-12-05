#ifndef __CM_TYPES_H__
#define __CM_TYPES_H__

typedef struct Coord
{
	Coord(int x = 0, int y = 0) : x(x), y(y) { }
	bool operator==(const Coord& other) const { return x == other.x && y == other.y; }
	bool operator!=(const Coord& other) const { return x != other.x || y != other.y; }
	int x;
	int y;
} Coord, *PCoord;

#define CMCoordMake(x, y) Coord((int)(x), (int)(y))
#define cmc(__X__,__Y__) CMCoordMake((int)(__X__), (int)(__Y__))
#define CMCoordZero cmc(0, 0)

typedef struct vec2
{
	vec2(float x = 0.0f, float y = 0.0f) : x(x), y(y) { }
	bool operator==(const vec2& other) const { return x == other.x && y == other.y; }
	bool operator!=(const vec2& other) const { return x != other.x || y != other.y; }
	float x;
	float y;
} vec2, *pvec2;

#define CMVec2Make(x, y) vec2((float)(x), (float)(y))
#define cmv2(__X__,__Y__) CMVec2Make((float)(__X__), (float)(__Y__))
#define CMVec2Zero cmv2(0.0f, 0.0f)

typedef struct vec4
{
	vec4(float x = 0.0f, float y = 0.0f, float z = 0.0f, float w = 0.0f) : x(x), y(y), z(z), w(w) { }
	bool operator==(const vec4& other) const { return x == other.x && y == other.y && z == other.z && w == other.w; }
	bool operator!=(const vec4& other) const { return x != other.x || y != other.y || z != other.z || w != other.w; }
	float x;
	float y;
	float z;
	float w;
} vec4, *pvec4;

#define CMVec4Make(x, y, z, w) vec4((float)(x), (float)(y), (float)(z), (float)(w))
#define cmv4(__X__,__Y__,__Z__,__W__) CMVec4Make((float)(__X__), (float)(__Y__), (float)(__Z__), (float)(__W__))
#define CMVec4Zero cmv4(0.0f, 0.0f, 0.0f, 0.0f)

typedef struct mat4
{
	float _m[16];

	mat4 (
		float m11 = 0.0f,
        float m12 = 0.0f,
        float m13 = 0.0f,
        float m14 = 0.0f,
        float m21 = 0.0f,
        float m22 = 0.0f,
        float m23 = 0.0f,
        float m24 = 0.0f,
        float m31 = 0.0f,
        float m32 = 0.0f,
        float m33 = 0.0f,
        float m34 = 0.0f,
        float m41 = 0.0f,
        float m42 = 0.0f,
        float m43 = 0.0f,
        float m44 = 0.0f
		)
	{
		_m[0] = m11, _m[1] = m12, _m[2] = m13, _m[3] = m14,
		_m[4] = m21, _m[5] = m22, _m[6] = m23, _m[7] = m24,
		_m[8] = m31, _m[9] = m32, _m[10] = m33, _m[11] = m34,
		_m[12] = m41, _m[13] = m42, _m[14] = m43, _m[15] = m44;
	}
	float at(int index) const { return _m[index]; }
	float at(int row, int col) const { return _m[row * 4 + col]; }
	void setAt(int index, float val) { _m[index] = val; }
	void setAt(int row, int col, float val) { _m[row * 4 + col] = val; }
	int len(void) { return 16; }
} mat4, *pmat4;

#define CMMat4Zero mat4()


// Templatize these next two if needed outside tile indexing.
typedef struct int1d
{
	int* a;

	int1d(int size)
		: _size(size), a(new int[size]) { }
	~int1d(void) { delete[] a, a = NULL; }
	int size() const { return _size; }
	int& at(int x) const { return a[x]; }
	int& operator[] (int x)
	{
		return a[x];
	}
	int1d(const int1d& other)
	{
		_size = other._size;
		a = new int[_size];
		memcpy(a, other.a, other.size() * sizeof(*a));
	}
	int1d& operator=(const int1d& rhs)
	{
		_size = rhs._size;
		memcpy(a, rhs.a, rhs.size() * sizeof(*a));
		return *this;
	}
private:
	int _size;
} int1d, *pint1d;

typedef struct int2d
{
	int2d(int w, int h)
		: _w(w), _h(h), _a(new int[w*h]) { }
	~int2d(void) { delete[] _a, _a = NULL; }
	int w() const { return _w; }
	int h() const { return _h; }
	int count() const { return _w * _h; }
	int val(int x, int y) const { return _a[x * _h + y]; }
	int& at(int x, int y) const { return _a[x * _h + y]; }
	int& operator[] (int x)
	{
		return _a[x];
	}
	int2d(const int2d& other)
	{
		_w = other._w;
		_h = other._h;
		_a = new int[_w*_h];
		memcpy(_a, other._a, other.count() * sizeof(*_a));
	}
	int2d& operator=(const int2d& rhs)
	{
		_w = rhs._w;
		_h = rhs._h;
		memcpy(_a, rhs._a, rhs.count() * sizeof(*_a));
		return *this;
	}
private:
	int* _a;
	int _w;
	int _h;
} int2d, *pint2d;

#endif // __CM_TYPES_H__
