# Copyright Attributions
## Music/Sound Effects:
### Background music:
background_music.mp3/.wav
Title: Vintage Electro Pop Loop \
Creator: frankum \
Date: March 16, 2017 \
Link: https://freesound.org/people/frankum/sounds/384468/ \
The material has not been modified. It is used under Creative Commons License 4.0 

### Consume Sound:
consume.wav
Title: Power Up, Bright, A.wav \
Creator: Inspector J           \
Date: November 20, 2017        \
Link: https://freesound.org/people/InspectorJ/sounds/411460/ \
The material has not been modified. It is used under Creative Commons License 4.0 

### Death Sound 1:
death1.wav
Title: Game Die \
Creator: Jofae \
Date: October 25, 2016 \
Link: https://freesound.org/people/Jofae/sounds/364929/ \
The material has not been modified. It is used under Creative Commons License 1.0 


### Death Sound 2:
death2.wav
Title: Failure/wrong action.wav \
Creator: GabrielAraujo \
Date: July 13, 2014 \
Link: https://freesound.org/people/GabrielAraujo/sounds/242503/ \
The material has not been modified. It is used under Creative Commons License 1.0


### Death Sound 3:
deat3.wav
Title: Retro video game sfx - Fail \
Creator: Ashe Kirk/Owlish Media \
Date: October 14, 2017 \
Link: https://freesound.org/people/OwlStorm/sounds/404743/ \
The material has not been modified. It is used under Creative Commons License 1.0

### Success/Level Completion Sound:
sucecss.wav
Title: Success Jingle \
Creator: JustInvoke \
Date: November 4, 2018 \
Link: https://freesound.org/people/JustInvoke/sounds/446111/ \
The material has not been modified. It is used under Creative Commons License 4.0

### Phase Shift Failure Sound:
swap_denied.wav\
Title: 8-bit Deny/Error Sound\
Creator: philRacoIndie\
Date: December 30, 2020\
Link: https://freesound.org/people/philRacoIndie/sounds/551543/\
The material has been modified to remove silence at the end of the recording. It is used under Creative Commons License 1.0

### Phase Shift Success Sound:
swap_success.wav \
Title: Bubble Pop \
Creator: elmasmalo1 \
Date: January 18, 2017 \
Link: https://freesound.org/people/elmasmalo1/sounds/376968/ \
The material has not been modified. It is used under Creative Commons License 3.0

## CraftPix Entities:
CraftPix does not license under creative commons but does provide the following license concerning use of their products:

Subject to your compliance with the terms and conditions of these Terms, CraftPix grants you a limited, non-exclusive, non-transferable license to all resources purchased from CraftPix store, without the right to sublicense contents.

1.1 CAPABILITIES \
1.1.1 You can use, copy, adapt, modify, prepare derivative works based upon all purchased assets; \
1.1.2 You may use game assets as many times as you like; \
1.1.3 Distribution of source files is NOT permitted; \
1.1.4 You can sell and distribute games with our assets; \
1.1.5 You can use each product in unlimited number of free and commercial projects; \
1.1.6 Without Royalty Fee. By the end of your subscription, you can continue to use all the downloaded game assets. 

## Miscellaneous assets
### Portal Sprites
Title: 2D Pixel Art Portal Sprites
Creator: Elthen's Pixel Art Shop
Link: https://elthen.itch.io/2d-pixel-art-portal-sprites
The material has been copied and placed into a larger tilesheet. It is used under a Attribution-NonCommercial 4.0 International (CC BY-NC 4.0) license with the given supplement permissions:
* You can use the assets in commercial projects, as long as you don't redistribute (i.e upload the assets to other sites) or resell(this includes minting as nfts) the assets.
* You don't have to give credit to me, but it is always appreciated.
* Under no circumstances can you use any of my work, sprites, assets in any form of blockchain-related technology, which includes NFTs, P2E Game, cryptocurrency, or future inventions in the space. It doesn't matter if you make games out of them, sell them by themselves. No matter how much they are modified, you are not allowed to use my sprites in any kind of NFT.

### Smoke Sprites
Creator: @nyk_nck (https://nyknck.itch.io/) \
Link: https://nyknck.itch.io/fx062 \
The material is not licensed under creative commons, but the creator has provided the following terms:
* You are allowed to use all of my assets in commercial projects. You should give me credit with the name @nyk_nck. Redistribution of any asset is not allowed.

### Backgrounds
Backgrounds for tutorial help screens, menus and ending screens were generated using Scenario. 
Terms and Conditions regarding use of their AI can be found here: https://www.scenario.com/terms-and-conditions

### Fonts
Creator: Xin Jin Meng \
Link: https://xinjinmeng.itch.io/xinjinmengs-8-bit-font-pack-feb-2022 \
The material is not licensed under creative commons, but the creator has provided the following terms:
* You are free to use these fonts for your own project as long you as you give credit to me, Xin Jin Meng.

***
# Escape Phase - Team 15

## Milestone 4

In milestone 4 we worked hard on creating a cohseive and fun game experience, start to finish. We took time to identify the little rough patches that were getting in the way of our game coming together exactly how we had hoped it would. We also went out of our way to try and get more player feedback and do additional testing to see where we might have stretched the realities of our game a bit too far. We didn't implement too many major new elements, but instead we added and adjusted things that helped polish our game and offer a more exciting and fun gameplay experience.

### Stability

Entry point:

-   `render_system.cpp`

We needed to go back and fix an issue that was uncovered in milestone 3, involving memory not being freed properly. The render tools and particle system tool were unfortunately left in and not properly cleared, despite being dynamically allocated objects. They are now freed in the render_system destructor and there are no more memory leaks.

### Playability

Entry point:

-   `/maps`

We have included 16 playable levels, with varrying degrees of difficulty, with some maps requiring more precise navigation and others presenting puzzles to solve. In order to play through the game (particularly the first time, though it is challenging on subsequent playthroughs) it takes significantly more than 10 minutes. Adding the coin collection targets, which encourage players to explore more, rather than just focus on completion, it surpasses the time requirement.

### User Experience

Entry point:

-   `/maps`
-   `/docu/Game_Feedback_Response.md`

Tutorial: We have included 4 different tutorial levels, which are accessible from the start screen. These levels cover the basic movement and user interactions, enemies/hazards, collectibles, and game mechanics that are required to complete the game. Each tutorial level introduces the topic, before providing the player the chance to practice on a simpler level that is targetted to require the new skill. By the end of the tutorial, players are aware of all aspects of the game.

User Feedback Adoption: We got feedback from crossplay sessions, both required and through discussion with players throughout the sessions, as well as from friends and our experience extensively playing the game. Those results and the changes implemented are captured in Game_Feedback_Response.md.

### Creative Components

#### [21] Camera Controls

Entry point:

-   `render_system.cpp:move_to_center`

The camera is zoomed in to track the player but is also pinned to the edges of the level to make sure there isn't that strange situation where it displays large chunks of blank background. This change has been integrated with the spotlight and lighting effects so that there are no unusual visual effects due to the additional camera movement.

#### [19] Reloadability

Entry point:

-   `world_system.cpp:~1600`
-   `data/latest_save.json`
-   `data/unlocked_levels.json`

The game can be saved and loaded on deman (with a short cooldown timer to prevent save/load abuse to beat levels). The save and load preserve coin count, visible/removed elements, and current level. The game is saved and loaded with the 5 and 7 keys respectively. The other option is to use the load button on the start screen to jump directly in to your previously saved game.
There is an additional, automatic function that saved unlocked levels without user input, so that they persist between plays, so that a user does not have to complete the game in one sitting every time.  
Additionally, there is some game state that is saved without user interaction, to improve the gameplay experience. The unlocked levels persist without the need to save, and our 3 high score score-boards (coins, time, score) save and display the data across game restarts.

#### [9] Complex Perscribed Motion

Entry point:

-   `Utility/ParticleGenerator.cpp:83`

After the death animation, where the exploded outwards particles follow a path that reflects simulated gravity, the partciles are zipped back to the start location to reform as the player. This zipping is done using hermite polynomials to create a smooth curve for all particles. The position is interpolated for each particle individually, using each particle's start position and the player spawn position, along with each particle's velocity at the end of the gravity effect and a goal velocity vector calculated from the particle position and goal position (scaled to not create too high of a target velocity). The zip takes place over the course of 1.5 seconds and the hermite equation has been adjusted using constants to create a more natural looking smooth curve.

#### [2] Parallax Backgrounds

Entry point:

-   `render_system.cpp:draw_cave_background`
-   `shaders/sprite.vs.glsl`

Using a uniform in our sprite shader and the GL_REPEAT texture parameter, we have created a realstic parallax effect based on player and level movement. Due to the dimensions of our levels, the most effective way to do this was just to scroll in the x direction. The effect takes in to consideration the map pinning from the camera position/zoom so that the backgrounds only move when the level itself is moving along with the player movement. There are 6, or more, layers for each background and the offset is used to create parallax movement for at least 5 of them.

#### [24] Basic Integrated Assets / [26] Numerous Sophisticated Integrated Assets

Entry point:

-   `data/textures`

We have introduced many custom textures, created using AI tools and with hand drawing, and altered many other textured elements to introduce them in to our game. The menu, game completion, and tutorial images were all made using a combination of AI generation with many artistic changes to things like colouring, adding text, and integrating game elements. The spritesheets were all made using GIMP to resize and assemble in a standardized way and also to adjust colours and animations so that they fit more in to the game's art style. The normal maps were were made using Photoshop, GIMP, and some AI tools, to separate specific elements from existing backgrounds, and then generate the maps.

Overall, we have hundreds of assets from multiple sprite packs that allow us to design levels in three different gameplay worlds, which have been assembled in to spritesheets for ease of use. There are also many animated components such as collectibles, enemies, and the exit portal. There are also animations for user interactions, like when a key or coin is picked up or a lock is unlocked.

#### [25] Game Balance

Entry point:

-   `/maps`
-   `docu/game_balancing.md`

We have performed quite a bit testing, working with to create a balanced distribution of level difficulties, to ensure that players are challenged but not overwhelmed, even within the context of our randomized level order. This work was based on feedback from internal and external players. An overview of the balancing work done can be found in `/docu/game_balancing.md`.

#### Newly integrated assets (citing sources for assets)

-   `data/textures`
-   scoreboards: https://free-game-assets.itch.io/free-space-shooter-game-user-interface

## Milestone 3

In milestone 3, we continued working and expanding on game features. Namely, we added the following:

-   Key and lock game mechanic
-   Pass-through platforms
-   A particle system that spawns particle effects on player death
-   Basic load/save functionality
-   Starting and menu screens
-   2.5D lighting effects
-   Randomization (finally) On start, all the levels are randomized, though you still have access to them directly through the level select screen.

During this milestone, we also realized that enemies and powerups aren't pivotal to our game's experience as they often got in the way of the exploratory puzzles that we were creating.
As a replacement for enemies, in order to ensure the difficulty was high enough, we added several more puzzle mechanics, such as the key-lock and the pass-through platforms.
Please see the updated proposal PDF for more details.

### Memory management

Entry point:

-   `main.cpp`
-   `tiny_ecs_registry`

We did quite extensive memory tests with the game being left on for long periods of time, and profiled alongside these tests. While we didn't end up in a place with memory issues, we discovered that there were some opportunities for improvement. We ensured that all additional components (since we had some not contained in ComponentContainers, would be removed when clear_components was called). Also, with the use of our resource manager and sprite rendering classes, we made sure that those dynamically allocated objects, and all OpenGL structures they contained, were properly cleared at teardown. We had already implemented comprehensive level deallocation and re-allocation in the second Milestone, and made sure to introduce a player reset that did not require extensive memory manipulation.

### Handle all user input

Entry point:

-   `src/world_system.cpp:on_key`

The on-key event handler checks the game state and allows only valid keypresses,
depending on the game state. This was important to ensure that there was no user manipulation of the level or game throughout the different animations.
The game reacts predictably to different movement inputs, including phase shifting during movement, multiple key presses, and "mashing."
The mouse inputs are handled by a similar callback setup to capture all menu inputs. Particular care was taken to ensure that variable screen resolutions did not impede user input.

### Real-time gameplay

Entry point: N/A

Alongside our memory management work, we all looked for bottlenecks using IDE profiling and implemented the following:

-   Introduced the Debug/Release mode that was posted on Piazza. Making `gl_has_errors()` only active during Debug configurations improved our performance and made it so that even large screen displays were smooth.
-   Cleared many of the textures that were no longer being used (to lower initialization delay).

### Prior missed milestone features & bug fixes

So far we have not had any missed milestone features or bugs (fingers crossed) so we have just worked on the bugs and minor things we found ourselves along the way.

### Consistent game resolution

Entry point:

-   `src/render_system.cpp:drawToScreen`

This is unchanged from the previous milestone. The game renders in 16:9 resolution, scaled to the full size of the screen, regardless of the device screen size or resolution.

### No crashes, glitches, unpredictable behaviour

Entry point: N/A

No crashes or glitches have been observed within our own testing.

## Creative Components:

### [5] Particle systems

Entry point:

-   `src/Utility/ParticleGenerator.cpp`
-   `src/Utility/ParticleGenerator.hpp`

We added a particle system that performs instanced rendering for ~100 particles that explode outwards when the player dies. The particles begin with randomized velocity and direction (all somewhat upwards), and travel in an arc as a result of gravity being applied on each system step. After a set amount of time, the particles transition to zip back to the player's start position and trigger a respawn once they all arrive there.
The instanced rendering includes intentional, multi-buffer swapping to prevent potential slowdown (which can occur if functions like glBufferSubData are used) from repeated GPU memory access of the same buffer. Buffers are freed for reuse before being swapped out.

### [6] 2.5(3D) lighting

Entry point:

-   point_lighting.fs.glsl
-   second_pass.fs.glsl

The player has been made to be a light-caster that illuminates several elements present on the background texture, when it passes in front of them. Both point and ambient lighting effects were introduced with the Phong reflection model (diffuse, ambient, and specular reflection) and newly generated normal maps were used to produce accurate illumination effects. Combining this with our earlier spotlight effect makes the game much more visually interesting.

### Newly integrated assets (citing sources for assets)

-   `data/textures`
-   buttons: https://free-game-assets.itch.io/free-space-shooter-game-user-interface
-   text: https://xinjinmeng.itch.io/xinjinmengs-8-bit-font-pack-feb-2022
-   unused assets:
    -   https://crusenho.itch.io/complete-gui-essential-pack
    -   https://wenrexa.itch.io/cursors5
-   `data/text`
-   unused text: https://creativk.itch.io/starglow-sprite-text

---

## Milestone 2

In milestone 2, our main achievement was implementing the core game mechanic (phase shifting), along with adapting the map loader to
dynamically load our levels into the game. We've added multiple levels to the game and have also
completed all the required deliverables, along with two creative ones.

We have slightly deviated from our original proposal timeline; randomized powerups and enemy movement will be implemented
in Milestone 3. Since those features weren't strictly required for this milestone,
we opted to prioritize core game mechanics to ensure we had a playable and extensible platform for us to build on. Now
that the major hurdle of multi-phase levels is complete, we will begin tuning and adding to the game to create a more
diverse game-play experience

Please see the bottom of the proposal PDF for the updated timeline.

### Game Logic Response to User Input

Entry point:

-   response: `src/world_system.cpp`
-   physics changes: `src/physics_system.cpp:229`

The core game logic (phase shifting) has been implemented and currently supports shifting between two phases.
This can also be extended beyond the current two layer/phase limit right now. Platforming logic was implemented
in the previous milestone and we have continued to build on that with additional physics tuning and a brand new
way of handling and resolving collisions between AABB elements. We have added keyboard controls to open
the help menu and cycle through levels.

### Sprite Sheet Animation

Entry point: `Utility/SpriteSheetRenderer.hpp`

A new utility class was added to provide an API to simplify draw calls for elements on any sprite sheet. This classes is
based on the original SpriteRenderer but includes the calculation for on the fly sprite draw calls. It also
required new shader logic.

### New integrated assets

Entry point:

-   `maps/`
-   `data/textures`

We have added several JSON files which contain loadable data for our levels. They are read in and lodaed using the map loader that was extended for this milestone.
We've also added a collection of new world tiles and sprite characters, all sampled from sprite sheet textures.

### Mesh-based collision detection

Entry point:

-   `src/physics_system.cpp:70`

Spikes are rendered using a mesh and will use line-line intersection to determine if the player has collided with it.
To create a more performant solution, the more advanced check only happens when a collision when an AABB enters the AABB surround the tile spikes are rendered on to.

### Basic user tutorial/help

Entry point: `render_system.cpp:305`

A tutorial/help screen with player controls is shown when the game is launched and when the `p` key is pressed. A subsequent press switches back from the help screen to the game. The game does not step while in the help screen.

### Minimal lag

Entry point: N/A

No lag has been experienced in our testing of the game, even when testing significant resolution differences, the scaling up has been quite successful and has not added lag to the game.

### Consistent game resolution

Entry point: `render_system.cpp`

The game loads with a consistent aspect ratio and resolution across different screen sizes.
This is achieved by first rendering to a pre-set internal resolution then projecting the
internal render to the true device window size. If the aspect ratio does not match, pillarboxing is used
to ensure that the internal aspect ratio is preserved.

### No crashes, glitches, unpredictable behaviour

Entry point: N/A

The game has not crashed while play-testing. Glitches and unpredictable behaviour have been handled alongside the development process. As of now, the game runs very well.

### [20] External integration

Entry point:

-   `src/map_loader.cpp`
-   `ext/json`

We are using nlohmann's JSON loader for C++ to load in our level maps in a managable and easily accessible format. These json objects are used to generate the levels on demand.

### [23]Audio feedback

Entry point: `src/world_system.cpp:151`

There are 7 audio effects (3 that are randomly selected between on player death, 1 for completing the level, 1 for consuming collectibles, 1 to indicate failed phase shift, and 1 on successful phase), and 1 background track.

## Milestone 1

Provide a writeup explaining how your milestone aligns with the plan:
For milestone 1, implemented a single, comprehensive level, a part of our future cave phase. Our goal was to complete some rudementary structure but we ended up exceeding that and being able to integrate more assets and complex structure than originally expected.
We did complete all required deliverables as well as the additional creative componenets. We have completed our two goal creative components in this milestone, and will continue on according to our planned timeline.

### Implemented feature folder

Entry point: `/src`
The main class structure is found in the /src folder, additional utility classes (included and adapted from the learn-OpenGL tutorial) can be found in the Utility folder.

### Textured Geometry

Entry point: `/data/textures`, `/data/meshes`,` /data/tiles`

We have set included a background image, and custom assets for our player character as well as all the level tiles, decorations, goals in the map. Rendering order is consistent with the background rendered before map tiles.
The entities free for commercial use and were found on https://www.kenney.nl/assets/pixel-platformer.

### Basic 2D transformations

Entry point: `/src/common.cpp`

Scale, Rotation, and Translate Transformations are applied to the rendered entities. with smooth motion across render cycles.

### Key-frame/state interpolation

Entry point: `src/world_system.cpp`

interpolation logic in step function; smooth movement from point A to point B in Cartesian or angle space. Patrolling enemies are calculated using state interpolation based on linear paths.

### Keyboard/mouse control

Entry point: `/src/world_system.cpp:369`

Arrow keys control player movement and the R key resets the game if desired. Up arrow triggers a jump and Right/Left key make player move to right and left.

### Random/coded action

Entry point: `src/world_system.cpp:254`

Two enemies with coded, patrol patterns. The motion is interpolated along coded, linear patrol routes.

### Well-defined game-space boundaries

Entry point: `/src/world_system.cpp:258`

Walls and map tiles provide well-defined game-space, with no way to fall out of the world. Map is loaded in with a map loader, designed by us.

### Simple collision detection & resolution

Entry point: `/src/world_system.cpp:266`

Player does not pass through defined walls, but can walk through grass (decoration element). Obvious penetrations are avoided and collectables disappear on collection.

### Minimal lag

Entry point: **N/A**

No lag has been experienced in our testing of the game. Steps to ensure this: A collider component so that our collision detection only checks entities with the collider component and not every entity that moves.

### No crashes, glitches, unpredictable behaviour

Entry point: **N/A**

The current version of the game has not crashed while playing.

### [1] Simple rendering effects

Entry point: `/src/games/render_system.cpp:267`

`RenderSystem::drawToScreen()` Second pass draw call that samples the initial draw call as a texture to create a "spotlight" effect around player position. Effects are currently done in the water glsl shaders. The sprite shader uses uniform to ensure that the sprite looks in the direction that it is moving.

### [8] Basic physics

Entry point: `/src/physics_system.cpp:106`

The y direction of jumps utilizes verlet integration to calculate motion.

### [23]Audio feedback

Entry point: `/src/world_system.cpp:151`

There are 5 audio effects (3 sounds that can be played on death (randomized), 1 for completing the level, 1 for consuming collectibles), and 1 background music.

### Team organization

Entry point: **N/A**

We created our own Discord channel where all of our communication takes place, and shared techniques and game components in the channel.
We meet on every Friday for game design in the booked room, makes sure all group members are on track and makes sure that other group members can help group members that are struggling.
We use Github where all of our files are stored and where group members can collaborate.
We have also created a source control process in which we create feature branches that get merged into master through PR’s. PR’s are reviewed by group members before being merged.
