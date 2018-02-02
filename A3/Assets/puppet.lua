-- puppet.lua
-- A simplified puppet without posable joints, but that
-- looks roughly humanoid.

red = gr.material({1.0, 0.0, 0.0}, {0.1, 0.1, 0.1}, 10)
blue = gr.material({0.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
green = gr.material({0.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)
white = gr.material({1.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
cyan = gr.material({0.0, 1.0, 1.0}, {0.1, 0.1, 0.1}, 10)
magenta = gr.material({1.0, 0.0, 1.0}, {0.1, 0.1, 0.1}, 10)
yellow = gr.material({1.0, 1.0, 0.0}, {0.1, 0.1, 0.1}, 10)

function getMesh(meshId, meshName, meshScale, meshMaterial,    
    parentNode, parentScale)
  --[[
  Get a Mesh whose scaling is independent of its parent's scaling;
  not translated from parent
  meshScale: list of size 3: (x, y, z) scaling
  parentScale: list of size 3: (x, y, z) scaling of parent
  ]]
  newMesh = gr.mesh(meshId, meshName)
  parentNode:add_child(newMesh)
  unscale(newMesh, parentScale)
  scale(newMesh, meshScale)
  newMesh:set_material(meshMaterial)
  return newMesh
end

function getMeshJoint(meshId, meshName, meshScale, meshMaterial,    
    jointNode)
  --[[
  Get a Mesh whose parent is a joint; the Mesh will be translated
  such that it's (top-most) endpoint is incident with the joint
  meshScale: list of size 3: (x, y, z) scaling
             (w.r.t. parent's scale)
  jointNode: parent joint node
  
  optional args:
  rotation: list of size 3: (x, y, z) rotation
  ]]
  newMesh = gr.mesh(meshId, meshName)
  jointNode:add_child(newMesh)
  scale(newMesh, meshScale)
    -- position endpoint at the joint
  newMesh:translate(0, -meshScale[2]/2.0, 0)
  newMesh:set_material(meshMaterial)
  return newMesh
end

function arm(name_prefix, rotation, parentNode, parentScale)
  --[[
  Returns an arm rooted at the shoulder joint; removes parent scaling
  name_prefix: string to prefix onto node names
  rotation: list of size 3: (x, y, z) rotation to apply to leg
  parentNode: node of parent
  parentScale: list of size 3: (x, y, z) scale of parent
  ]]
  xRange = {-80, 0, 80}
  if name_prefix == 'right' then
    yRange = {-40, 0, 40}
  else -- left
    yRange = {-40, 0, 40}
  end
  shoulderJoint = gr.joint(name_prefix .. 'ShoulderJoint',
      xRange, yRange)
  rotate(shoulderJoint, rotation)
  parentNode:add_child(shoulderJoint)
  unscale(shoulderJoint, parentScale)

  -- shoulder
  shoulderScale = {0.15, 0.15, 0.15}
  shoulder = gr.mesh('sphere', name_prefix .. 'Shoulder')
  shoulderJoint:add_child(shoulder)
  scale(shoulder, shoulderScale)
  shoulder:set_material(red)

  -- upperarm
  upperarmScale = {0.12, 0.5, 0.12}
  upperarm = getMeshJoint('cube', name_prefix .. 'Upperarm',
      upperarmScale, magenta, shoulderJoint)

  -- elbow joint
  xRange = {0, 0, 90}
  yRange = {-80, 0, 80}
  elbowJoint = gr.joint(name_prefix .. 'ElbowJoint', 
      xRange, yRange)
  upperarm:add_child(elbowJoint)
  unscale(elbowJoint, upperarmScale)
  elbowJoint:translate(0, -0.5, 0)
  
  -- elbow
  elbowScale = {0.1, 0.1, 0.1}
  elbow = gr.mesh('sphere', name_prefix .. 'Elbow')
  elbowJoint:add_child(elbow)
  scale(elbow, elbowScale)
  elbow:set_material(red)

  -- lowerleg
  lowerarmScale = {0.1, 0.5, 0.1}
  lowerarm = getMeshJoint('cube', name_prefix .. 'Lowerarm',
      lowerarmScale, magenta, elbowJoint)

  -- wrist joint
  xRange = {-90, 0, 90}
  if name_prefix == 'right' then
    yRange = {-25, 0, 25}
  else
    yRange = {-25, 0, 25}
  end
  wristJoint = gr.joint(name_prefix .. 'WristJoint',
      xRange, yRange)
  lowerarm:add_child(wristJoint)
  --rotate(ankleJoint, {-90, 0, 0})
  unscale(wristJoint, lowerarmScale)
  wristJoint:translate(0, -0.5, 0)

  -- wrist
  wristScale = {0.08, 0.08, 0.08}
  wrist = gr.mesh('sphere', name_prefix .. 'Wrist')
  wristJoint:add_child(wrist)
  scale(wrist, wristScale)
  wrist:set_material(red)

  -- foot
  handScale = {0.11, 0.25, 0.04}
  hand = getMeshJoint('cube', name_prefix .. 'Hand',
      handScale, magenta, wristJoint)
  
  return shoulderJoint
end

function leg(name_prefix, rotation, parentNode, parentScale)
  --[[
  Returns a leg rooted at the hip joint; removes parent scaling
  name_prefix: string to prefix onto node names
  rotation: list of size 3: (x, y, z) rotation to apply to leg
  parentNode: node of parent
  parentScale: list of size 3: (x, y, z) scale of parent
  ]]
  xRange = {-110, 0, 40}
  if name_prefix == 'right' then
    yRange = {-10, 0, 80}
  else -- left
    yRange = {-80, 0, 10}
  end
  hipJoint = gr.joint(name_prefix .. 'HipJoint',
      xRange, yRange)
  rotate(hipJoint, rotation)
  parentNode:add_child(hipJoint)
  unscale(hipJoint, parentScale)

  -- hip
  hipScale = {0.15, 0.15, 0.15}
  hip = gr.mesh('sphere', name_prefix .. 'Hip')
  hipJoint:add_child(hip)
  scale(hip, hipScale)
  hip:set_material(red)

  -- upperleg
  upperlegScale = {0.12, 0.5, 0.12}
  upperleg = getMeshJoint('cube', name_prefix .. 'Upperleg',
      upperlegScale, magenta, hipJoint)

  -- knee joint
  xRange = {0, 0, 160}
  yRange = {-5, 0, 5}
  kneeJoint = gr.joint(name_prefix .. 'KneeJoint', 
      xRange, yRange)
  upperleg:add_child(kneeJoint)
  unscale(kneeJoint, upperlegScale)
  kneeJoint:translate(0, -0.5, 0)
  
  -- knee
  kneeScale = {0.1, 0.1, 0.1}
  knee = gr.mesh('sphere', name_prefix .. 'Knee')
  kneeJoint:add_child(knee)
  scale(knee, kneeScale)
  knee:set_material(red)

  -- lowerleg
  lowerlegScale = {0.1, 0.5, 0.1}
  lowerleg = getMeshJoint('cube', name_prefix .. 'Lowerleg',
      lowerlegScale, magenta, kneeJoint)

  -- ankle joint
  xRange = {-20, 0, 40}
  if name_prefix == 'right' then
    yRange = {-5, 0, 10}
  else
    yRange = {-10, 0, 5}
  end
  ankleJoint = gr.joint(name_prefix .. 'AnkleJoint',
      xRange, yRange)
  lowerleg:add_child(ankleJoint)
  rotate(ankleJoint, {-90, 0, 0})
  unscale(ankleJoint, lowerlegScale)
  ankleJoint:translate(0, -0.5, 0)

  -- ankle
  ankleScale = {0.08, 0.08, 0.08}
  ankle = gr.mesh('sphere', name_prefix .. 'Ankle')
  ankleJoint:add_child(ankle)
  scale(ankle, ankleScale)
  ankle:set_material(red)

  -- foot
  footScale = {0.11, 0.25, 0.04}
  foot = getMeshJoint('cube', name_prefix .. 'Foot',
      footScale, magenta, ankleJoint)
  
  return hipJoint
end

function inverse(arr)
  --[[ Precond: arr has no zero entries ]]
  newarr = {}
  for i, v in ipairs(arr) do
    newarr[i] = 1.0 / arr[i]
  end
  return newarr
end

function scale(node, scaleVec3)
  --[[
  Scale the node
  scaleVec3: list of size 3: (x, y ,z) scaling
  ]]
  node:scale(table.unpack(scaleVec3))
end

function unscale(node, scaleVec3)
  --[[
  Unscale the node by a dict
  (useful for removing parent node scaling)
  ]]
  node:scale(table.unpack(inverse(scaleVec3)))
end

function rotate(node, rotVec3)
  if rotVec3 then
    node:rotate('x', rotVec3[1])
    node:rotate('y', rotVec3[2])
    node:rotate('z', rotVec3[3])
  end
end

rootnode = gr.node('root')
rootnode:rotate('y', -20.0)
rootnode:scale( 0.25, 0.25, 0.25 )
rootnode:translate(0.0, 0.0, -1.0)

-- torso
torso = gr.mesh('cube', 'torso')
rootnode:add_child(torso)
torsoScale = {0.5, 1.0, 0.5}
scale(torso, torsoScale)
torso:set_material(white)

-- waist joint
waistjoint = gr.joint('waistJoint', {0, 0, 0}, {-30, 0, 30})
torso:add_child(waistjoint)
unscale(waistjoint, torsoScale) -- remove parent scaling
waistjoint:translate(0, -1/2, 0)

-- pelvis
pelvisScale = {torsoScale[1] * 1.1,
    torsoScale[2] * 0.2,
    torsoScale[3] * 1.1}
pelvis = getMeshJoint('cube', 'pelvis', pelvisScale, green,    
    waistjoint)

-- chest
chestScale = {0.6, 0.4, 0.6}
chest = getMesh('cube', 'chest', chestScale, cyan,    
    torso, torsoScale)
chest:translate(0, 1/2, 0) -- move to top of torso

-- left leg
left_leg = leg('left', {0, 0, 0}, pelvis, pelvisScale)
left_leg:translate(-1/3, -1/2, 0) 

-- right leg
right_leg = leg('right', {0, 0, 0}, pelvis, pelvisScale)
right_leg:translate(1/3, -1/2, 0) -- translate w.r.t pelvis scale

-- left arm
left_arm = arm('left', {-90, -90, 0}, chest, chestScale)
left_arm:translate(-1/2, 0, 0) 

-- right arm
right_arm = arm('right', {-90, 90, 0}, chest, chestScale)
right_arm:translate(1/2, 0, 0) 

-- base of neck
neckbase = gr.joint('neckbase', {0, 0, 0}, {-80, 0, 80})
chest:add_child(neckbase)
unscale(neckbase, chestScale) -- remove parent scaling
-- neckbase position is incident with top of torso
neckbase:rotate('x', -180)
neckbase:translate(0, 1/2, 0)

-- neck
neckScale = {0.1, 0.2, 0.1}
neck = getMeshJoint('cube', 'neck', neckScale, yellow,    
    neckbase)

-- base of head
headbase = gr.joint('headbase', {-45, 0, 45}, {-60, 0, 60})
neck:add_child(headbase)
unscale(headbase, neckScale) -- remove parent scaling
headbase:translate(0, -1/2, 0)

-- head
headScale = {0.4, 0.3, 0.4}
head = getMeshJoint('cube', 'head', headScale, blue,    
    headbase)

-- ears
ears = gr.mesh('sphere', 'ears')
head:add_child(ears)
ears:scale(1.2, 0.08, 0.08)
ears:set_material(magenta)

-- left eye
leftEye = gr.mesh('cube', 'leftEye')
head:add_child(leftEye)
leftEye:scale(0.2, 0.1, 0.1)
leftEye:translate(0.2, -0.2, -0.5)
leftEye:set_material(magenta)

-- right eye
rightEye = gr.mesh('cube', 'rightEye')
head:add_child(rightEye)
rightEye:scale(0.2, 0.1, 0.1)
rightEye:translate(-0.2, -0.2, -0.5)
rightEye:set_material(magenta)

return rootnode
