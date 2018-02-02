Please see ../README.md first.

Disclaimer: Please excuse the discrepancy between the puppet's appearance in the screenshot and the executable. I could not gain access to the updated version. However, this executable should contain the full functionality described below.

You can switch between two modes: Position and Joints.

Position Mode:
- Dragging with the left mouse button held down changes the x and y translations of the puppet.
- Dragging up/down with the middle mouse button held down zooms out/in.
- Dragging with the right mouse button held down should initiate a "virtual trackball" that controls the orientation of the puppet around its own origin.

Joints Mode:
- Clicking with left mouse button selects/deselects joints.
- Dragging with middle mouse button changes angles of all selected joints.
- Dragging with right mouse button rotates head left and right.

There are various other configurations available in the dialog menu in-game.

Q to Quit.

To compile the executable 'A0':
`premake4 gmake`
`make`

To run:
`./A3 Assets/puppet.lua`

To clean the compiled binaries:
`make clean`
`premake4 clean`

