#ifndef __LINEAR_ALGEBRA__
#define __LINEAR_ALGEBRA__

typedef struct {
	float X, Y;
} vec2d;

static inline vec2d Vec2d2f(float X, float Y) {
	return (vec2d){X,Y};
}
static inline vec2d Vec2d1f(float V) {
	return (vec2d){V,V};
}

static inline vec2d Vec2d(vec2d U) {
	return (vec2d){U.X,U.Y};
}

#define VEC2_OP_EXPAND_1F(name, base2f) static inline vec2d name(vec2d U, float S) \
	{ return base2f(U,S,S); }

#define VEC2_OP_EXPAND_V(name, base2f) static inline vec2d name(vec2d U, vec2d V) \
	{ return base2f(U,V.X,V.Y); }

#define VEC2_OP_EXPAND(name, base2f) VEC2_OP_EXPAND_1F(name ## 1f, base2f); \
	VEC2_OP_EXPAND_V(name, base2f)



static inline vec2d Vec2d_Mul2f(vec2d U, float X, float Y) {
	return (vec2d) { U.X * X, U.Y * Y};
}
VEC2_OP_EXPAND(Vec2d_Mul, Vec2d_Mul2f);

static inline vec2d Vec2d_Div2f(vec2d U, float X, float Y) {
	return (vec2d) { U.X / X, U.Y / Y };
}
VEC2_OP_EXPAND(Vec2d_Div, Vec2d_Div2f);

static inline vec2d Vec2d_Add2f(vec2d U, float X, float Y) {
	return (vec2d) { U.X + X, U.Y + Y };
}
VEC2_OP_EXPAND(Vec2d_Add, Vec2d_Add2f);

static inline vec2d Vec2d_Sub2f(vec2d U, float X, float Y) {
	return (vec2d) { U.X - X, U.Y - Y };
}
VEC2_OP_EXPAND(Vec2d_Sub, Vec2d_Sub2f);


static inline float Vec2d_LenSq(vec2d U) {
	return (U.X * U.X) + (U.Y * U.Y);
}
static inline float Vec2d_Len(vec2d U) {
	return sqrtf(Vec2d_LenSq(U));
}

static inline float Vec2d_DistSq2f(vec2d U, float Vx, float Vy) {
	return Vec2d_LenSq(Vec2d_Sub2f(U,Vx,Vy));
}

static inline float Vec2d_DistSq(vec2d U, vec2d V) {
	return Vec2d_LenSq(Vec2d_Sub(U,V));
}

static inline float Vec2d_Dot2f(vec2d U, float Vx, float Vy) {
	return (U.X * Vx) + (U.Y * Vy);
}

static inline float Vec2d_Dot1f(vec2d U, float VxVy) {
	return Vec2d_Dot2f(U,VxVy, VxVy);
}
static inline float Vec2d_Dot(vec2d U, vec2d V) {
	return Vec2d_Dot2f(U,V.X,V.Y);
}

static inline vec2d Vec2d_RotateEx(vec2d U, float Sin, float Cos) {
	return (vec2d) { 
		(U.X * Cos) - (U.Y * Sin),
		(U.X * Sin) + (U.Y * Cos)
	};
}

static inline vec2d Vec2d_Rotate(vec2d U, float Rad) {
	float Sin = sinf(Rad);
	float Cos = cosf(Rad);
	return Vec2d_RotateEx(U,Sin,Cos);
}

static inline vec2d Vec2d_RotateDegree(vec2d U, float Angle) {
	const float Rad = Angle * M_PI / 180.0f;
	return Vec2d_Rotate(U,Rad);
}

#endif
