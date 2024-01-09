## A guide to using new features

#### SpriteSheetRenderer:

-   This is a rendering class that can be used to display specific elements from a sprite sheet
-   The function DrawSprite takes in the standard texture, modelMatrix, and colour paramteres, but to use the sprite sheet the following is required:
    -   sheet_details is information about the columns and rows of sprites within the sheet. It is stored in a vec2 with x being the numbe columns and y being the number of rows of sprites on the given sheet
    -   sprite_frame is the specific sprite that you wish to select. It is a vec2 with x being the column and y being the row.
-   This requires spritesheets with no padding/margins around the individual sprites. A generic way to use padded sprite sheets is something I am still working on.

#### Key Instruction:
- Left/Right Arrow Keys
    - Player moves Left/Right
- Up Arrow Key
    - Player Jumps
- Space Key
    - Phase Shift
- "B" / "N" Keys
    - "B" skips backwards 1 level, and "N" skips to next level
- "R" Key
    - Restart Game
- "P" Key
    - Toggle between play screen and pause/tutorial screen
