* [X] Move PNG/Texture loading into separate file.
  * [ ] More tightly integrate with the concept of resource loading
* [ ] Single color texture should also somehow be part of some Texture/Resource related file.
      Perhaps a little convention would help for single colors, like giving the color in HTML
      style. (e.g. "#139aCfe"). These could be potentially be used to generate textures as well
      and give them names. Ideally, if a texture hasn't already been registered, it should be
      loaded or generated.
      * [ ] Or just have a boolean on which to use texture and/or baseColor
* [X] "Entity" into its own file
* [X] Each entity should have its own .h file defining behavior
* [X] Move the render quad code into a Debug(.h/.cpp) file or something
* [X] Fix shadow mapping area 
* [X] Apply color to walls and floors
* [X] Each tank needs to be bound to different controls.
* [ ] Update Input query functions
  * [ ] if (applesauce:::Input::keyIsHeldDown(GLFW_KEY_SPACE)
  * [ ] if (applesauce::Input::keyWasJustPressed(GLFW_KEY_A))
* [ ] Encapsulate frame buffer
* [ ] Camera behavior should be defined by Entity logic
* [ ] Figure out how to size shadow "frustum" dynamically
* [ ] Load levels from files
* [ ] Strengthen collision detection so that it is possible to ricochet bullets:  * [ ] Correct normal is passed back.
  * [ ] objects that fall out of the world are removed and don't crash the system
  * [ ] {erhaps contact information is passed to onTouch which may contain a
        reference to other entity.
* [ ] Given the 2D nature of all collision (so far), perhaps it's best to adopt Blender's 
      coordinate system of Z as the vertical axis. Alternatively, we plan for collision that
      involves height as well, though at the moment we aren't doing any vertical shots. That
      may simply hamper gameplay in general.
* [ ] Getting collision right may be a matter of implementing the whole game in 2D, which may not
      be a bad exercise. The question is whether our entitites are still 3D as they need to still
      be rendered in 3D. So, can we essentially have game state be 2D. Alternatively, entities store
      their state and orientations for 2D, but are essentially "on a wall" with Z representing the
      vertical axis. This may be the right compromise point. My concern is how much we need to do to
      translte from the entity information (model matrix) and the rendering.
* [ ] Perhaps a better way of putting it is a separation of concerns between gamestate update
      and rendering. Also, would this facilitate networking?
