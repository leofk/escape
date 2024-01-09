// internal
#include "physics_system.hpp"
#include "world_init.hpp"

#if __has_include("world_system.hpp")

#include "world_system.hpp"

#endif

extern bool movement_key[4];
extern float jumpTimer;
extern vec2 mouse_position;
extern int current_layer;
extern bool facingRight;
// extern bool dropPlatform;
extern bool isInJump;

const float COYOTE_TIME = 135.f;
float pl_coyote_time = COYOTE_TIME;

// A struct to hold the details of a swpet AABB collision
// From the video and code at: https://www.youtube.com/watch?v=3dIiTo7mlnU
typedef struct hit
{
    f32 time;
    vec2 position;
    vec2 normal;
    bool is_hit;
} Hit;

static void update_sweep_result(Hit *result, Motion &player, Motion &other, float step_seconds);

// Returns the local bounding coordinates scaled by the current size of the entity
vec2 get_bounding_box(const Motion &motion)
{
    // abs is to avoid negative scale due to the facing direction.
    return {abs(motion.scale.x), abs(motion.scale.y)};
}

// Correct AABB collision detection

bool CheckCollision(Motion &one, Motion &two) // AABB - AABB collision
{
    // Top left corner
    vec2 A_one = vec2(one.position.x - (one.scale.x / 2.0), one.position.y - (one.scale.y / 2.0));
    vec2 A_two = vec2(two.position.x - (two.scale.x / 2.0), two.position.y - (two.scale.y / 2.0));

    // Bottom right corner
    vec2 B_one = vec2(one.position.x + (one.scale.x / 2.0), one.position.y + (one.scale.y / 2.0));
    vec2 B_two = vec2(two.position.x + (two.scale.x / 2.0), two.position.y + (two.scale.y / 2.0));

    // collision x-axis?
    bool collisionX = A_one.x <= B_two.x &&
                      A_two.x <= B_one.x;
    // collision y-axis?
    bool collisionY = A_one.y <= B_two.y &&
                      A_two.y <= B_one.y;
    // collision only if on both axes
    return collisionX && collisionY;
}

bool isLineIntersecting(const vec2 &line_a0, const vec2 &line_a1, const vec2 &line_b0, const vec2 &line_b1)
{
    float denom = (line_b1.y - line_b0.y) * (line_a1.x - line_a0.x) - (line_b1.x - line_b0.x) * (line_a1.y - line_a0.y);
    float num1 = (line_b1.x - line_b0.x) * (line_a0.y - line_b0.y) - (line_b1.y - line_b0.y) * (line_a0.x - line_b0.x);
    float num2 = (line_a1.x - line_a0.x) * (line_a0.y - line_b0.y) - (line_a1.y - line_a0.y) * (line_a0.x - line_b0.x);

    float t = num1 / denom;
    float r = num2 / denom;

    return (t < 1 && t > 0 && r < 1 && r > 0);
}

bool CheckSpikeCollision(Motion &one, Motion &two)
{ // Simple line - line collision
    // one is player...will likely change in future
    float leftBound = two.position.x - (two.scale.x / 2);

    std::vector<vec2> player_bounds = {
        vec2(one.position.x - (one.scale.x / 2.f), one.position.y + (one.scale.y / 2.f)), // bot-left
        vec2(one.position.x + (one.scale.x / 2.f), one.position.y - (one.scale.y / 2.f)), // top-right
        vec2(one.position.x + (one.scale.x / 2.f), one.position.y + (one.scale.y / 2.f)), // bot-right
        vec2(one.position.x - (one.scale.x / 2.f), one.position.y - (one.scale.y / 2.f)), // top-left
    };

    mat2 rotMatrix = mat2(vec2(cos(two.angle), sin(two.angle)),
                          vec2(-sin(two.angle), cos(two.angle)));

    static const std::vector<vec3> spike_vertices = {
        {0.0, 0.165, 0.33},
        {0.33, 0.485, 0.66},
        {0.66, 0.825, 1.0}};

    vec2 offset = vec2(two.position.x - (two.scale.x / 2), two.position.y - (two.scale.y / 2));
    for (auto &vertice : spike_vertices)
    {
        vec2 spike_lbase = vec2(vertice.x * two.scale.x, 0.f);
        vec2 spike_peak = vec2(vertice.y * two.scale.x, -(two.scale.y));
        vec2 spike_rbase = vec2(vertice.z * two.scale.x, 0.f);

        spike_lbase = rotMatrix * spike_lbase;
        spike_rbase = rotMatrix * spike_rbase;
        spike_peak = rotMatrix * spike_peak;

        spike_lbase += offset;
        spike_peak += offset;
        spike_rbase += offset;

        for (int i = 0; i < 2; i++)
        {
            bool leftIntersect1 = isLineIntersecting(player_bounds[i], player_bounds[2], spike_lbase, spike_peak);
            bool rightIntersect1 = isLineIntersecting(player_bounds[i], player_bounds[2], spike_rbase, spike_peak);

            bool leftIntersect2 = isLineIntersecting(player_bounds[i], player_bounds[3], spike_lbase, spike_peak);
            bool rightIntersect2 = isLineIntersecting(player_bounds[i], player_bounds[3], spike_rbase, spike_peak);
            if (leftIntersect1 || rightIntersect1 || leftIntersect2 || rightIntersect2)
            {
                return true;
            }
        }
    }

    return false;
}

void PhysicsSystem::updatePlayerVelocity(float elapsed_ms)
{
    Entity &player = registry.players.entities[0];
    Motion &motion = registry.motions.get(player);
    Movable &movable = registry.movables.get(player);
    float max_velocity = convertWorldScale(movable.max_velocity);

    // Determine horizontal movement based on key presses
    if (!(movement_key[2] ^ movement_key[3]))
    {
        if (!motion.isOnGround && motion.velocity.x != 0)
        {
            motion.velocity.x = ((motion.velocity.x > 0) ? max_velocity : -max_velocity) * 0.65f;
        }
        else
        {
            motion.velocity.x = 0.f;
        }
    }
    else if (movement_key[2] && !movable.left_blocked)
    {
        motion.velocity.x = -max_velocity;
        movable.right_blocked = false;
    }
    else if (movement_key[3] && !movable.right_blocked)
    {
        motion.velocity.x = max_velocity;
        movable.left_blocked = false;
    }

    // printf("%f\n", pl_coyote_time);
    pl_coyote_time -= elapsed_ms;
    if (pl_coyote_time < 0) {
        pl_coyote_time = 0;
    }
    if (movement_key[0] && !isInJump && (motion.isOnGround || pl_coyote_time > 0))
    {
        float jumpHeight = convertWorldScale(movable.jumpHeight);
        float jumpDist = convertWorldScale(movable.jumpDist);
        motion.velocity.y = -(2 * jumpHeight * max_velocity) / jumpDist;
        motion.isOnGround = false;
        isInJump = true;
    }
}

void PhysicsSystem::step(float elapsed_ms)
{
    float step_seconds = elapsed_ms / 1000.f;

    Entity &player = registry.players.entities[0];
    if (registry.deathTimers.has(player))
        return;

    updatePlayerVelocity(elapsed_ms);

    // Iterate over all persistent movable entities
    const auto movable_persistent = registry.movables.entities;
    for (Entity e : movable_persistent)
    {
        Motion &motion = registry.motions.get(e);
        Movable &movable = registry.movables.get(e);
        stepEntity(step_seconds, motion, movable, e);
    }

    // Iterate over all movable entities in the current layer
    const auto movable_in_layer = registry.layers[current_layer]->current_movables.entities;
    // Iterate over all movable entities
    for (Entity e : movable_in_layer)
    {
        Motion &motion = registry.motions.get(e);
        Movable &movable = registry.movables.get(e);
        stepEntity(step_seconds, motion, movable, e);
    }

    stepCompanions();
}

void PhysicsSystem::stepEntity(float step_seconds, Motion &motion, Movable &movable, Entity e) const
{
    motion.prev_position = motion.position;

    motion.position.x += motion.velocity.x * step_seconds;

    if (!motion.isOnGround)
    {
        // Note that jumpers will need to set their initial velocity to perform the jump; see player jump logic
        float grav = 2240.f; // Default gravity for non-jumpable entities
        if (movable.jumpDist > 0 && movable.jumpHeight > 0)
        {
            float jumpHeight = convertWorldScale(movable.jumpHeight);
            float jumpDist = convertWorldScale(movable.jumpDist);
            float max_velocity = convertWorldScale(movable.max_velocity);
            grav = (2 * jumpHeight * max_velocity * max_velocity) /
                   (jumpDist * jumpDist);
        }

        // Player-only check: jump key released before reaching peak of jump
        if (registry.players.has(e) && motion.velocity.y < 0.f && !movement_key[0])
        {
            grav *= 1.55f;
        }
        // Faster falling
        if (motion.velocity.y > 0.f)
        {
            grav *= 1.85f;
        }
        // Velocity Verlet applied only on y-axis
        motion.position.y =
            motion.position.y + motion.velocity.y * step_seconds + grav * (step_seconds * step_seconds * 0.5f);
        motion.velocity.y = motion.velocity.y + grav * step_seconds * 0.5f;
    }

    motion.isOnGround = false; // Assume character is not on ground unless collision is detected
    movable.right_blocked = false;
    movable.left_blocked = false;

    Hit result = {0};
    Player &player_comp = registry.players.components[0];

    auto &motion_container = registry.layers[current_layer]->current_motions;
    for (uint i = 0; i < motion_container.components.size(); i++)
    {
        Motion &motion_i = motion_container.components[i];
        Entity entity_i = motion_container.entities[i];
        if (e == entity_i)
            continue;
        // if the entity is a decoration, no collision
        if (!registry.decorations.has(entity_i))
        {
            if (CheckCollision(motion, motion_i))
            {
                if (registry.walls.has(entity_i) ||
                    (registry.lock_tiles.has(entity_i) && player_comp.keys == 0 &&
                     isPresent(entity_i, registry.lock_tiles)))
                {
                    update_sweep_result(&result, motion, motion_i, step_seconds);
                }
                else if (registry.platforms.has(entity_i) &&
                        (motion.prev_position.y + motion.scale.y / 2.f < motion_i.position.y - motion.scale.y / 2.f + 0.005f)) // Tolerance constant
                {
                    update_sweep_result(&result, motion, motion_i, step_seconds);
                    if (movement_key[1]) {
                        result.is_hit = false;
                    }
                }
                else if (registry.spikes.has(entity_i))
                {
                    if (CheckSpikeCollision(motion, motion_i))
                    {
                        registry.collisions.emplace_with_duplicates(e, entity_i);
                    }
                }
                else if (isPresent(entity_i, registry.collectibles) && isPresent(entity_i, registry.keys) &&
                         isPresent(entity_i, registry.lock_tiles))
                {
                    // Non wall entity, create a collisions event for more complex collision resolution
                    registry.collisions.emplace_with_duplicates(e, entity_i);
                }
            }
        }
    }

    if (result.is_hit && length(result.position - motion.position) <= length(motion.velocity))
    {
        motion.position[0] = result.position[0];
        motion.position[1] = result.position[1];

        if (result.normal[1] != 0)
        {
            motion.position[0] += motion.velocity[0] * step_seconds;
            motion.velocity[1] = 0;
        }
        else if (result.normal[0] != 0)
        {
            motion.position[1] += motion.velocity[1] * step_seconds;
            motion.velocity[0] = 0;
            if (result.normal[0] < 0)
            {
                movable.right_blocked = true;
            }
            else
            {
                movable.left_blocked = true;
            }
        }
    }

    // If the first hit is with the AABB side that has a normal facing up (y < 0) then we have landed on something
    if ((result.is_hit && result.normal.y < 0) || motion.isOnGround)
    {
        motion.isOnGround = true;
        isInJump = false;
        pl_coyote_time = COYOTE_TIME; // Is wrong, but for convenience; assumes that player is the only thing moving via physics
    }
}

void PhysicsSystem::stepCompanions()
{
    // Make companions follow player
    Motion &motion = registry.motions.get(registry.players.entities[0]);
    float ndx = 1.f;
    for (Entity e : registry.companions.entities)
    {
        Motion &companion_motion = registry.motions.get(e);
        if (facingRight)
        {
            companion_motion.position.x = motion.position.x - (motion.scale.x / 3.f) - (10.f * ndx);
        }
        else
        { // Player is moving left
            companion_motion.position.x = motion.position.x + (motion.scale.x / 3.f) + (10.f * ndx);
        }
        companion_motion.position.y = motion.prev_position.y - (motion.scale.y / 3.f) - 5.f;

        ndx++;
    }
}

/** Performs an intersect calculation between a ray and an AABB
 *
 * From the video and code at: https://www.youtube.com/watch?v=3dIiTo7mlnU
 *
 * @param pos origin of the ray
 * @param velocity velocity vector of the ray
 * @param A the top left corner of the AABB
 * @param B the bottom right corner of the AABB
 * @param other_pos the center point of the AABB
 *
 * @returns a Hit struct containing the details of a hit between the ray and the AABB (includes indicitaion if a hit occured at all)
 */
Hit ray_intersect_aabb(vec2 pos, vec2 velocity, vec2 A, vec2 B, vec2 other_pos)
{
    Hit hit = {0};
    vec2 min, max;

    min = A;
    max = B;

    f32 last_entry = -INFINITY;
    f32 first_exit = INFINITY;

    for (u8 i = 0; i < 2; ++i)
    {
        if (velocity[i] != 0)
        {
            f32 t1 = (min[i] - pos[i]) / velocity[i];
            f32 t2 = (max[i] - pos[i]) / velocity[i];

            last_entry = fmaxf(last_entry, fminf(t1, t2));
            first_exit = fminf(first_exit, fmaxf(t1, t2));
        }
        else if (pos[i] <= min[i] || pos[i] >= max[i])
        {
            return hit;
        }
    }

    if (last_entry != -INFINITY && first_exit > last_entry && first_exit > 0 && last_entry < 1)
    {
        hit.position[0] = pos[0] + velocity[0] * last_entry;
        hit.position[1] = pos[1] + velocity[1] * last_entry;

        hit.is_hit = true;
        hit.time = last_entry;

        f32 dx = hit.position[0] - other_pos[0];
        f32 dy = hit.position[1] - other_pos[1];
        f32 px = ((A.x - B.x) / 2) - fabsf(dx);
        f32 py = ((A.y - B.y) / 2) - fabsf(dy);

        if (px < py)
        {
            hit.normal[0] = (dx > 0) - (dx < 0);
        }
        else
        {
            hit.normal[1] = (dy > 0) - (dy < 0);
        }
    }

    return hit;
}

/** Performs a sweep intersection test between two AABB entities
 *
 * Velocity is calculated using the player motion struct with position - prev_position. This could be adjusted to be passed
 * an actual velocity vector so that the function can be used more generally but for now it works since this swept AABB
 * intersection test is only used for moving entities colliding with static elements
 *
 * From the video and code at: https://www.youtube.com/watch?v=3dIiTo7mlnU
 *
 * @param result a Hit struct to hold the result of the full sweep pass
 * @param player player motion struct
 * @param other The other AABB entity to check for collision
 *
 * The result struct is only updated if an earlier hit is detected or if a hit happens
 * at the same time but with a higher velocity.
 *
 */
static void update_sweep_result(Hit *result, Motion &player, Motion &other, float step_seconds)
{
    vec2 velocity = player.velocity * step_seconds * 2.f;

    vec2 A = vec2(other.position.x - other.scale.x / 2 - player.scale.x / 2,
                  other.position.y - other.scale.y / 2 - player.scale.y / 2);
    vec2 B = vec2(other.position.x + other.scale.x / 2 + player.scale.x / 2,
                  other.position.y + other.scale.y / 2 + player.scale.y / 2);

    Hit hit = ray_intersect_aabb(player.position, velocity, A, B, other.position);

    if (hit.is_hit && !std::isnan(hit.time))
    {

        if (hit.time < result->time)
        {
            *result = hit;
        }
        else if (hit.time == result->time)
        {
            // Solve highest velocity axis first.
            if (fabsf(velocity[0]) > fabsf(velocity[1]) && hit.normal[0] != 0)
            {
                *result = hit;
            }
            else if (fabsf(velocity[1]) > fabsf(velocity[0]) && hit.normal[1] != 0)
            {
                *result = hit;
            }
        }
    }
}
