#include "common.hpp"
#include "components.hpp"

void Transform::scale(vec2 scale)
{
	mat3 S = {{scale.x, 0.f, 0.f}, {0.f, scale.y, 0.f}, {0.f, 0.f, 1.f}};
	mat = mat * S;
}

void Transform::rotate(float radians)
{
	float c = cosf(radians);
	float s = sinf(radians);
	mat3 R = {{c, s, 0.f}, {-s, c, 0.f}, {0.f, 0.f, 1.f}};
	mat = mat * R;
}

void Transform::translate(vec2 offset)
{
	mat3 T = {{1.f, 0.f, 0.f}, {0.f, 1.f, 0.f}, {offset.x, offset.y, 1.f}};
	mat = mat * T;
}

// Function changed to allow debug and release mode. Changes thanks to CPSC 427 student (Kaiden Richardson) piazza post:
// https://piazza.com/class/ljirfl6s1vx145/post/105
bool gl_has_errors()
{
#ifdef DEBUG_OPENGL
	// Existing gl_has_errors implementation

	GLenum error = glGetError();

	if (error == GL_NO_ERROR)
		return false;

	while (error != GL_NO_ERROR)
	{
		const char *error_str = "";
		switch (error)
		{
		case GL_INVALID_OPERATION:
			error_str = "INVALID_OPERATION";
			break;
		case GL_INVALID_ENUM:
			error_str = "INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			error_str = "INVALID_VALUE";
			break;
		case GL_OUT_OF_MEMORY:
			error_str = "OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			error_str = "INVALID_FRAMEBUFFER_OPERATION";
			break;
		}

		fprintf(stderr, "OpenGL: %s", error_str);
		error = glGetError();
		assert(false);
	}

	return true;
#else
	return false;
#endif
}

float cross_product_2D(vec2 a, vec2 b)
{
	return a.x * b.y - b.x * a.y;
}

bool sameSign(float a, float b)
{
	if (a <= 0 && b <= 0)
	{
		return true;
	}
	else if (a >= 0 && b >= 0)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Direction VectorDirection(Motion &player, Motion &entity)
{

	vec2 upRight = entity.position - vec2(-(entity.scale.x / 2), entity.scale.y / 2);
	vec2 upLeft = entity.position - vec2(entity.scale.x / 2, entity.scale.y / 2);
	vec2 downRight = entity.position + vec2(entity.scale.x / 2, entity.scale.y / 2);
	vec2 downLeft = entity.position + vec2(-(entity.scale.x / 2), entity.scale.y / 2);

	upRight = normalize(upRight - entity.position);
	upLeft = normalize(upLeft - entity.position);
	downRight = normalize(downRight - entity.position);
	downLeft = normalize(downLeft - entity.position);

	vec2 playerToEntity = normalize(player.prev_position - entity.position);

	// SImplification of the direction math comes from a post on: https://stackoverflow.com/questions/13640931/how-to-determine-if-a-vector-is-between-two-other-vectors
	if ((cross_product_2D(upLeft, playerToEntity) * cross_product_2D(upLeft, upRight)) >= 0 && (cross_product_2D(upRight, playerToEntity) * cross_product_2D(upRight, upLeft)) >= 0)
	{

		return UP;
	}
	else if ((cross_product_2D(upLeft, playerToEntity) * cross_product_2D(upLeft, downLeft)) >= 0 && (cross_product_2D(downLeft, playerToEntity) * cross_product_2D(downLeft, upLeft)) >= 0)
	{

		return LEFT;
	}

	else if ((cross_product_2D(downRight, playerToEntity) * cross_product_2D(downRight, downLeft)) >= 0 && (cross_product_2D(downLeft, playerToEntity) * cross_product_2D(downLeft, downRight)) >= 0)
	{

		return DOWN;
	}
	else
	{
		return RIGHT;
	}
}

bool point_AABB_intersect(vec2 p, vec2 position, vec2 size) // point - AABB intersect test
{
	// Top left corner
	vec2 A = vec2(position.x - (size.x / 2.0), position.y - (size.y / 2.0));
	// Bottom right corner
	vec2 B = vec2(position.x + (size.x / 2.0), position.y + (size.y / 2.0));

	// collision x-axis?
	bool collisionX = p.x <= B.x &&
					  A.x <= p.x;
	// collision y-axis?
	bool collisionY = p.y <= B.y &&
					  A.y <= p.y;
	// collision only if on both axes
	return collisionX && collisionY;
}
