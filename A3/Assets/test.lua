-- a3mark.lua
-- A very simple scene creating a trivial hierarchical puppet.
-- We'll use this scene as part of testing your assignment.
-- See a3mark.png for an example of what this file produces.

rootnode = gr.node('root')

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)

s0 = gr.mesh('sphere','s0')
rootnode:add_child(s0)
s0:set_material(white)
s0:scale(0.3, 0.3, 0.3)

joint = gr.joint('joint', {0,0,0}, {0,0,0})
s0:add_child(joint)
joint:scale(1/0.3, 1/0.3, 1/0.3) -- unscale
joint:translate(0, 6.0, 0)

jointball = gr.mesh('sphere','jointball')
joint:add_child(jointball)
jointball:scale(0.3, 0.3, 0.3)
jointball:set_material(red)

stick = gr.mesh('cube', 'stick')
joint:add_child(stick)
stick:set_material(green)
stick:scale(4.3, 0.1, 0.1)
stick:translate(2.5, 0, 0)

s1 = gr.mesh('sphere','s1')
rootnode:add_child(s1)
s1:scale(0.1, 2.0, 0.1)
s1:set_material(red)

s2 = gr.mesh('sphere','s2')
rootnode:add_child(s2)
s2:scale(0.1, 2.0, 0.1)
s2:rotate('z', -90.0)
s2:translate(2.0, -2.0, 0.0)
s2:set_material(blue)

s3 = gr.mesh('sphere','s3')
rootnode:add_child(s3)
s3:scale(0.1, 0.1, 2.0)
s3:translate(0.0, -2.0, 2.0)
s3:set_material(green)

rootnode:translate(-2.75, 0.0, -10.0)

return rootnode
