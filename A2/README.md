Please see ../README.md first.

Click on the interaction modes to switch between them (or use the keyboard shortcuts).

Rotate View (keyboard shortcut O):
- Dragging with the left mouse button rotates the view vector about the eye's x (horizontal) axis.
- Dragging with the middle mouse button rotates the view about the eye's y (vertical) axis.
- Dragging with the right mouse button rotates the view about the eye's z axis.

Translate View (keyboard shortcut N):
- Dragging with the left mouse button translates the eye position along the eye's x (horizontal) axis.
- Dragging with the middle mouse button translates the eye position along the eye's y axis.
- Dragging with the right mouse button translates the eye position along the eye's z axis.

Perspective (keyboard shortcut P):
- Dragging with the left mouse button changes the field of view (FOV) of the projection over a range from 5 to 160 degrees.
- Dragging with the middle mouse button translates projection's near plane along the view direction.
- Dragging with the right mouse button translates projection's far plane along the view direction.

Rotate Model (keyboard shortcut R):
- Dragging with the left mouse button rotates the cube about its local x axis.
- Dragging with the middle mouse button rotates the cube about its local y axis.
- Dragging with the right mouse button rotates the cube about its local z axis.

Translate Model (keyboard shortcut T):
- Dragging with the left mouse button translates the cube about its local x axis.
- Dragging with the middle mouse button translates the cube about its local y axis.
- Dragging with the right mouse button translates the cube about its local z axis.

Scale Model (keyboard shortcut S):
- Dragging with the left mouse button scales the cube in its local x direction.
- Dragging with the middle mouse button scales the cube in its local y direction.
- Dragging with the right mouse button scales the cube in its local z direction.
(Note that scaling the cube non-uniformly might mean that it no longer looks like a cube when drawn, but like a more general box.)

Viewport (keyboard shortcut V): 
Use the mouse to draw an axis-aligned rectangle on screen, which defines the viewport in normalized device coordinates into which the scene will be drawn.

Q to Quit.

To compile the executable 'A0':
`premake4 gmake`
`make`

To run:
`./A2`

To clean the compiled binaries:
`make clean`
`premake4 clean`

