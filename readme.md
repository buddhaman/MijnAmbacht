Minecraft clone #2136216361237712


Todo:

- Make full texture table.
- Implement color on vertices.
- Do lighting when calculating chunk.

- Implement random generator.
- Transparency. Generate waterchunk
- Use cellular automata to update water.
- Regular meshes with floats.
- Use floatmeshes to make trees. Make function that can draw beams like lines.
- Physics engine (verlet).
- Replace static trees with dynamic trees if player is close.
- Ragdoll enemies.
- Collision detection.
- Use physics engine to generate falling blocks after mining. 

done:
- Use perlin noise to generate pretty terain.
- Multiple types of blocks. 
- Multiple chunks.
- Wireframe rendering.              
- Don't render invisible faces.     

Big ideas:
    - Simulate entities. Enemies, bees, birds, sheep, pigs, cow, ants with colonies that collect stuff (blocks). Langtons ant. Cellular automata.
    - Directional lighting on CPU ? (Again are normals necessary?)
    - Lighting (torches etc)
    - Ambient occlusion (Idea: simply count perpendicular faces and add to lighting term. Are normals necessary?).
    - Instanced billboard rendering.
    - Bushes: instanced billboards but static if far. Replace with dynamic if close. 
    - Move trees/bush/grass with shader. Like wind.
    - Dynamic voxels with instancing.

**HUGE IDEAS:**
    - Accurate fluid dynamics to simulate wind/explosions.
    - Online: upload picture of own head.

