## Game Feedback And Responses

### Controls

-   Movement:

    -   In M1 we had the player controls mapped to the arrow keys and received several comments that that felt uncomfortable. This was due to the small size of arrow keys on laptops, as well as the fact that most games these days utilize WASD for their movement.
        -   To adapt to this, we changed our movement controls to be WASD but with the jump being tied to the space bar
    -   In M2, people were more positive about the movement keys, but we had around an even split between players that were happy and unhappy about the jump being kept separate from the movement (mapped to space bar)
        -   We decided to condense our controls to be simply WASD with W initiating a jump. We are looking forward to additional feedback and perhaps changing the game to include more than one possible input scheme
        -   An additional thing we found was that some laptops would not allow jumping up and to the left, that seemed to be a callback overlading issue and so simplifying the input handling seemed to help clear that issue up.

-   Specialized Controls:
    -   Layer Shifting Mechanic:
        -   We have gone through a couple different iterations of keyboard mapping. Originally it was tied to space, but that had to change when we made space our jump key.
        -   We then changed it to be O and P with O switching to the first layer and P switching to the second layer. This change was based on our idea that specific keys mapping to layers would be allow us to more intuitively expand to additional layers (if we were to go to +3 we assumed we would map them to the number keys 1,2,3, etc.). Since we have only used two layers, we quite predictably received feedback that it was very challenging to remember which layer was on which key.
        -   We have since gone back to a layer toggle, with space bar swapping between the two layers. This has received mostly positive feedback, with players feeling that it was the most intuitive way to allow either two hand playing (space on the right hand) or single hand (space on the left thumb).

### Gameplay

-   Level difficulty:

    -   In M1 we had only a single level that was made much more challenging because there was a catching on the walls that allowed a player to grab on before continuing with the momentum on release. The feedback was that that was an unintuitive way to apply physics, so we switched the collision resolution to include some momentum carrying on sliding along walls after collision.
    -   In M2 we received feedback that our levels were a bit over-reliant on fast reaction times and specific inputs, which took away that excitement of having the multi-layer system to explore. To respond to that, we created some additional levels that were more based on exploration and puzzle solving.
    -   In M3, people felt that our new levels were great but there was feedback that we had some levels that were still maybe a little bit too precise for jump timings. This was likely also because we introduced our randomness element, where players traverse the levels randomly, potentially starting out on a hard one. In response to this we are adding in some more straightforward levels, and are working on a way to tailor our randomness to prevent frustration (a player will not be able to get all of the hard levels first, there will always be some easier ones mixed in for learning and skill development)
    -   Overall, we had lots of feedback about game balance and gamepley, both internal and external. In order to ensure that the feedback was all taken in to consideration, we did significant work in M4 on game balancing. Since we also submitted the game balance creative component, the details of how we took and used this feedback can be found in the `/docu/game_balancing.md` document.

-   Visuals:
    -   For M1 and M2 we had a simple, more cartoon-ish texture style. This worked but when we introduced different characters for the players, and backgrounds, we got minor feedback (and we also ourselves felt) that the art style no longer felt as cohesive as it did previously.
    -   For M3 and M4 we moved to more standardized tile packs, ensuring that we preserved the theming and art style across levels. We also introduced additional level locations to allow us to have more diverse visuals. Overall, it has produced a more cohesive looking game
    -   We recieved feedback that some of the entities that we used had hitboxes that were not aligned with their appearance (tree branches that did not fill a full tile but the hitbox did). We decided to move away from the irregular/smaller entities for level tiles, and only use squares that took up the full hitbox. This meant that the user would have a consistent experience with navigating the levels. We have also since reintroduced the smaller tiles but they are pass-through platforms that can be navigated in a different manner and so there are no unusual collision issues with them.

### User experience

-   Look and Feel:

    -   There were some comments that the tilesets that we were using didn't match very well, which was understandable because we had used tile packs with different art styles. We went to a single source and looked for similar, 8-bit art tilepacks and ended up finding a collection that were by the same designer(s).
    -   We had used a simplistic collectible, which was originally to be a demo mesh for the mesh loader, and swapped that out for a sprite that was both animated and from the same art style as the tilepacks.
    -   The lock and key mechanic was not very clear, the blocks that were to be unlocked were stylistically not clear enough to actually make it clear that the player needed to do some extra puzzling. We added in a lock marker that showed which tiles could be unlocked, so that players would know a bit more about the design of the level, simply by exploring.
    -   When introducing the tutorial, for M4, we went back and forth about if we should force it on the player when they press start or if it should be a selectable option from the main menu. One of the main issues this caused (ie. feedback we received) was that it was frustrating for a skilled player to be forced to play the tutorial every time they wanted to start a new game. In the end, we decided to provide a selectable tutorial option in the menu, which then takes the player through the instructional screens and levels. Since our tutorial is based purely on the core gameplay skills, and we also provide many levels that can be considered easy, we felt that this was a good way to package up these teaching levels. 
    -   From cross play, we received feedback that our text relaying the current player points was a bit large and that it felt like it distracted from the gameplay. We shrunk it down and moved it further in to the top corner to be out of the way. We used this feedback as we introduced additional text display, making sure that it was always tucked further away and didn't distract from the level.

### Internal Feedback

-   Within our team, we had several playtest sessions and there was unsurprisingly a lot of feedback whenever we worked on the game. Most of the key changes are covered within the previous points but there were a few specific things that came out of our team discussions.
    -   Key layout was a hot topic in our discussions, since we could not settle on a right or left hand dominant layout. In the end we stuck with the more current WASD layout but we acknowledged that it would be ideal to facilitate multiple key layouts for different handedness as well as player preference.
    -   Tilesets: Art style was another discussion we had, and we even explored creating custom tilesets for players/enemies but we ended up preferring a few existing sets since they helped us to provide consistency and meant that by simply selecting the same art style, we could quickly introduce new assets.
